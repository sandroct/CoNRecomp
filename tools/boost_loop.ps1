# boost_loop.ps1 - mantem o build em prioridade Alta (modo ESFORCO TOTAL)
# Roda escondido; sai quando o build termina (sem ninja/cl por 3 checagens seguidas).
# Iniciar:  powershell -File tools\boost_loop.ps1   (ou via Start-Process -WindowStyle Hidden)
$misses = 0
while ($true) {
    $procs = Get-Process -Name ninja, cl -ErrorAction SilentlyContinue
    if ($procs) {
        foreach ($p in $procs) {
            try { if ($p.PriorityClass -ne 'High') { $p.PriorityClass = 'High' } } catch {}
        }
        $misses = 0
    } else {
        $misses++
        if ($misses -ge 3) { break }   # build acabou (ou pausado): encerra o booster
    }
    Start-Sleep -Seconds 20
}
