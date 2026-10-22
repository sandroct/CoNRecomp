# fuse_final.ps1 - fusao final: restaura+mergeia o ninja_log e o deixa mais novo que o manifesto
# (o ninja descarta o .ninja_log quando o build.ninja e mais novo) e re-dispara o build.
$ErrorActionPreference = 'Stop'
$o = 'C:\recomp\CoNRecomp\build\port'
$python = 'C:\Users\Alessandro\AppData\Local\Programs\Python\Python312\python.exe'
$his = 'G:\CoNRecomp\build\port\.ninja_log'

Copy-Item "$o\.ninja_log.ours_bak" "$o\.ninja_log" -Force
& $python 'C:\recomp\CoNRecomp\tools\merge_ninja_log.py' $his
(Get-Item "$o\.ninja_log").LastWriteTime = (Get-Date)
$bn = (Get-Item "$o\build.ninja").LastWriteTime
$lg = (Get-Item "$o\.ninja_log").LastWriteTime
Write-Host ("log: {0} linhas | manifesto {1} | log {2} | log mais novo: {3}" -f (Get-Content "$o\.ninja_log").Count, $bn.ToString('HH:mm:ss.fff'), $lg.ToString('HH:mm:ss.fff'), ($lg -gt $bn))

Remove-Item 'C:\recomp\CoNRecomp\extracted\build_final.log' -ErrorAction SilentlyContinue
Start-Process -FilePath 'cmd.exe' -ArgumentList '/c', 'C:\recomp\CoNRecomp\extracted\run_final.bat' -WindowStyle Hidden
Write-Host "build final disparado - observar os primeiros minutos"
