# watchdog.ps2 - vigia o build distribuido; corrige erros de heap e re-dispara se travar (CoNRecomp)
#
# A cada execucao (agendado a cada 10 min):
#   1. fim? (con.exe OU SUBSET_A_DONE) -> sai
#   2. pausa manual? (extracted\WATCHDOG_OFF) -> sai
#   3. erros de heap no log (C1002/C1060/C3859)? -> auto-fix: poe o arquivo em CON_NOOPT_SOURCES,
#      reconfigura (so a regra daquele arquivo muda) e re-dispara. Maximo 3 fixes por noite.
#   4. compilando agora? -> sai
#   5. parado ha >15 min? -> re-dispara o subset A (pausa segura)
#   6. se o mesmo arquivo falhar 2x, ou houver 3+ re-disparos sem progresso -> escreve ALERTA e para
#      (precisa de intervencao humana).
#
# Log: extracted\watchdog.log

$root = 'C:\recomp\CoNRecomp'
$logFile = "$root\extracted\watchdog.log"
$subsetLog = "$root\extracted\subsetA.log"
$buildLog = "$root\extracted\build_con.log"
$bat = "$root\extracted\run_subsetA.bat"
$cmake = "$root\src\CMakeLists.txt"
$state = "$root\extracted\watchdog_state.json"

function Say([string]$msg) {
    Add-Content -Path $logFile -Value ("{0}  {1}" -f (Get-Date -Format 'dd/MM HH:mm'), $msg) -Encoding UTF8
}

function Read-State {
    if (Test-Path $state) { return Get-Content $state -Raw | ConvertFrom-Json }
    return [pscustomobject]@{ fixes = 0; restarts = 0; lastFix = ''; lastCount = 0 }
}
function Save-State($s) { $s | ConvertTo-Json | Set-Content $state }

$s = Read-State

# 1. fim?
if (Test-Path "$root\build\port\bin\con.exe") { exit 0 }
if (Test-Path $subsetLog) {
    if (Select-String -Path $subsetLog -Pattern 'SUBSET_A_DONE' -Quiet) { exit 0 }
}
# 2. pausa manual?
if (Test-Path "$root\extracted\WATCHDOG_OFF") { exit 0 }

# 3. auto-fix de erros de heap (C1002/C1060/C3859) -------------------------------
$heapLine = $null
foreach ($log in @($subsetLog, $buildLog)) {
    if (Test-Path $log) {
        $hit = Select-String -Path $log -Pattern 'error C(1002|1060|3859)' | Select-Object -Last 1
        if ($hit) {
            $ln = $hit.LineNumber
            $fail = Select-String -Path $log -Pattern 'FAILED:' | Where-Object { $_.LineNumber -lt $ln } | Select-Object -Last 1
            if ($fail -and ($fail.Line -match '__\\([A-Za-z0-9_.]+)\.cpp\.obj')) {
                $heapLine = $Matches[1]
            }
        }
    }
}
if ($heapLine) {
    if ($heapLine -eq $s.lastFix) {
        Say "ALERTA: $heapLine falhou de novo depois do fix - precisa de intervencao humana"
        exit 0
    }
    if ($s.fixes -ge 3) {
        Say "ALERTA: 3 fixes de heap em uma noite - provavel erro de codigo, precisa de intervencao"
        exit 0
    }
    $txt = Get-Content $cmake -Raw
    if ($txt -notmatch [regex]::Escape($heapLine)) {
        $new = $txt -replace '(set\(CON_NOOPT_SOURCES ")([^"]*)(")', ("`$1`$2;$heapLine`$3")
        Set-Content $cmake -Value $new -NoNewline
        Say "auto-fix: $heapLine adicionado a CON_NOOPT_SOURCES (heap do MSVC); reconfigurando"
        & cmd /c "`"C:\recomp\CoNRecomp\tools\win\vsenv.cmd`" cmake -S C:\recomp\CoNRecomp -B C:\recomp\CoNRecomp\build\port" *>> $logFile
        $s.fixes++; $s.lastFix = $heapLine
        Save-State $s
    }
}

# 4. saudavel?
if (Get-Process -Name cl -ErrorAction SilentlyContinue) { exit 0 }

# 5. parado faz tempo? ------------------------------------------------------------
$stale = $true
if (Test-Path $subsetLog) {
    $age = (Get-Date) - (Get-Item $subsetLog).LastWriteTime
    $stale = $age.TotalMinutes -gt 15
}
if (-not $stale) { exit 0 }

# sem progresso entre re-disparos? conta pelo total de objetos
$now = 0
$objDir = "$root\build\port\src\CMakeFiles\con_game_code.dir\__\generated"
if (Test-Path $objDir) { $now = @(Get-ChildItem $objDir -Filter *.obj -ErrorAction SilentlyContinue).Count }
if ($s.lastCount -eq $now -and $s.restarts -ge 2) {
    Say "ALERTA: 3 re-disparos sem progresso novo ($now objetos) - preciso de intervencao"
    exit 0
}

Say "sem compilacao ha >15min ($now objetos): re-disparando o subset A"
$ninja = Get-Process -Name ninja -ErrorAction SilentlyContinue
if ($ninja) { $ninja | Stop-Process -Force }
$n = 0
while ((Get-Process -Name cl -ErrorAction SilentlyContinue) -and $n -lt 12) { Start-Sleep -Seconds 10; $n++ }
Get-Process -Name cl -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue

$p = Start-Process -FilePath 'cmd.exe' -ArgumentList '/c', $bat -WindowStyle Hidden -PassThru
$p.PriorityClass = 'High'
$s.restarts++; $s.lastCount = $now
Save-State $s
Say "subset A re-disparado (PID $($p.Id)) - retoma de onde parou"
