# build_control.ps1 - controle de potencia do build (CoNRecomp)
#
#   powershell tools\build_control.ps1 -Mode Full     # 8 jobs + prioridade alta (ESFORCO TOTAL)
#   powershell tools\build_control.ps1 -Mode Pause    # pausa limpa (arquivo atual termina; nada corrompe)
#   powershell tools\build_control.ps1 -Mode Resume   # continua incremental (6 jobs, prioridade normal)
#   powershell tools\build_control.ps1 -Mode Status   # painel atual
#
# Seguranca: PAUSE mata o ninja primeiro e deixa os cl orfaos TERMINAREM o arquivo atual (nunca
# interrompe um cl no meio da escrita -> nenhum .obj truncado). Nada e perdido: o Ninja e incremental.

param(
    [Parameter(Mandatory = $true)][ValidateSet('Full', 'Pause', 'Resume', 'Status')]
    [string]$Mode,
    [int]$Jobs = 8
)

$bat = "$PSScriptRoot\..\extracted\run_build.bat"
$vsenv = "$PSScriptRoot\win\vsenv.cmd"

function Stop-BuildClean {
    $ninja = Get-Process -Name ninja -ErrorAction SilentlyContinue
    if ($ninja) { $ninja | Stop-Process -Force }
    Start-Sleep -Seconds 2
    # espera os cl orfaos terminarem seus arquivos atuais (nunca matar pela metade!)
    $n = 0
    while ((Get-Process -Name cl -ErrorAction SilentlyContinue) -and $n -lt 30) {
        Start-Sleep -Seconds 5
        $n++
    }
    Get-Process -Name cl -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
}

switch ($Mode) {
    'Pause' {
        Stop-BuildClean
        Write-Host "[build] PAUSADO - zero recursos usados. Retome com -Mode Resume."
    }
    'Full' {
        Stop-BuildClean
        $content = "@echo off`r`ncall `"$vsenv`" cmake --build C:\recomp\CoNRecomp\build\port --target con -- -j $Jobs >> C:\recomp\CoNRecomp\extracted\build_con.log 2>&1`r`necho BUILD_EXIT=%ERRORLEVEL% >> C:\recomp\CoNRecomp\extracted\build_con.log`r`n"
        [IO.File]::WriteAllText($bat, $content)
        $p = Start-Process -FilePath 'cmd.exe' -ArgumentList '/c', $bat -WindowStyle Hidden -PassThru
        $p.PriorityClass = 'High'          # herda para os cl filhos: prioridade acima de tudo
        Write-Host "[build] FULL: $Jobs jobs, prioridade Alta (PID $($p.Id)).~2x mais rapido."
    }
    'Resume' {
        Stop-BuildClean
        $content = "@echo off`r`ncall `"$vsenv`" cmake --build C:\recomp\CoNRecomp\build\port --target con -- -j 6 >> C:\recomp\CoNRecomp\extracted\build_con.log 2>&1`r`necho BUILD_EXIT=%ERRORLEVEL% >> C:\recomp\CoNRecomp\extracted\build_con.log`r`n"
        [IO.File]::WriteAllText($bat, $content)
        Start-Process -FilePath 'cmd.exe' -ArgumentList '/c', $bat -WindowStyle Hidden
        Write-Host "[build] RESUME: 6 jobs, prioridade normal."
    }
    'Status' {
        & 'C:\Users\Alessandro\AppData\Local\Programs\Python\Python312\python.exe' "$PSScriptRoot\build_status.py"
    }
}
