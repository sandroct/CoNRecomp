# iso_read.ps1 - read an ISO9660 (PS2 DVD) image: list files or extract one path.
# Generic tool; no game-specific knowledge. Sector size is fixed at 2048 (DVD/PS2 logical block).
#
# Usage:
#   powershell -File tools\win\iso_read.ps1 -Iso "<path.iso>"                       # recursive listing (LBA, size, path)
#   powershell -File tools\win\iso_read.ps1 -Iso "<path.iso>" -ExtractPath "SYSTEM.CNF" -Out "<dir>"
#
# Notes: reads only; handles multi-sector directories; ignores Joliet/UDF (PS2 discs are readable from
# ISO9660). Directory records names are matched case-insensitively; ";1" version suffixes are trimmed.
param(
    [Parameter(Mandatory = $true)][string]$Iso,
    [string]$ExtractPath = '',
    [string]$Out = ''
)
$ErrorActionPreference = 'Stop'
$SECTOR = 2048

$fs = [System.IO.File]::Open((Resolve-Path $Iso), [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
try {
    function Read-Sectors([long]$lba, [int]$count) {
        $buffer = New-Object byte[] ($count * $SECTOR)
        $null = $fs.Seek($lba * $SECTOR, [System.IO.SeekOrigin]::Begin)
        $total = 0
        while ($total -lt $buffer.Length) {
            $read = $fs.Read($buffer, $total, $buffer.Length - $total)
            if ($read -le 0) { throw "EOF at LBA $lba (byte $($lba * $SECTOR))" }
            $total += $read
        }
        return $buffer
    }

    function Read-UInt32BE([byte[]]$d, [int]$o) {
        return ([uint32]$d[$o] -shl 24) -bor ([uint32]$d[$o+1] -shl 16) -bor ([uint32]$d[$o+2] -shl 8) -bor [uint32]$d[$o+3]
    }
    function Read-UInt32LE([byte[]]$d, [int]$o) {
        return ([uint32]$d[$o]) -bor ([uint32]$d[$o+1] -shl 8) -bor ([uint32]$d[$o+2] -shl 16) -bor ([uint32]$d[$o+3] -shl 24)
    }

    function Parse-Record([byte[]]$d, [int]$o) {
        # returns $null when $o is past data / record length 0 (caller should skip to next sector)
        $len = [int]$d[$o]
        if ($len -eq 0) { return $null }
        $nameLen = [int]$d[$o + 32]
        if ($nameLen -eq 0) { return $null }
        $name = [System.Text.Encoding]::ASCII.GetString($d, $o + 33, $nameLen)
        $name = $name -replace ';1$|^\.+/?$', ''
        $isDir = ($d[$o + 25] -band 2) -ne 0
        # both-endian LBA/size: low half on the left (LE), verify against right (BE); use LE
        $lba = Read-UInt32LE $d ($o + 2)
        $size = Read-UInt32LE $d ($o + 10)
        $beLba = Read-UInt32BE $d ($o + 6)
        if ($lba -ne $beLba) { throw "bad both-endian LBA in record at $o" }
        return @{
            name = $name; lba = $lba; size = $size; isDir = $isDir; next = $o + $len
        }
    }

    function Read-Dir([uint32]$lba, [uint32]$size) {
        # iterate every record in the directory extent; returns an ArrayList of parsed records
        $records = New-Object System.Collections.ArrayList
        $bytesTotal = [uint32]0
        while ($bytesTotal -lt $size) {
            $chunk = Read-Sectors $lba 1
            $lba += 1
            $bytesTotal += $SECTOR
            $pos = 0
            while ($pos -lt $SECTOR) {
                $rec = Parse-Record $chunk $pos
                if ($rec -eq $null) { break }
                if ($rec.next -ge $SECTOR) { break }
                if ($rec.name -ne '') {
                    $null = $records.Add($rec)
                }
                $pos = $rec.next
            }
        }
        return ,$records
    }

    # Primary Volume Descriptor at LBA 16
    $pvd = Read-Sectors 16 1
    if (($pvd[1] -ne 0x43) -or ($pvd[2] -ne 0x44) -or ($pvd[3] -ne 0x30) -or ($pvd[4] -ne 0x30) -or ($pvd[5] -ne 0x31)) {
        throw 'no ISO9660 CD001 volume descriptor at LBA 16'
    }
    # root directory record embedded in the PVD at offset 156
    $root = Parse-Record $pvd 156
    if (-not $root.isDir) { throw 'PVD root record is not a directory' }

    function Resolve-Node([string]$path) {
        # resolve a '/'-separated path from the root; returns the record hash or $null
        $parts = ($path -replace '\\', '/').Split('/') | Where-Object { $_ -ne '' }
        $node = $root
        foreach ($part in $parts) {
            if (-not $node.isDir) { return $null }
            $found = $null
            foreach ($rec in (Read-Dir $node.lba $node.size)) {
                if ($rec.name -ieq $part) { $found = $rec; break }
            }
            if ($found -eq $null) { return $null }
            $node = $found
        }
        return $node
    }

    if ($ExtractPath -ne '') {
        if ($Out -eq '') { throw '-Out is required with -ExtractPath' }
        $node = Resolve-Node $ExtractPath
        if ($node -eq $null) { throw "not found: $ExtractPath" }
        if ($node.isDir) { throw "-ExtractPath must be a file; ${ExtractPath} is a directory" }
        New-Item -ItemType Directory -Force -Path $Out | Out-Null
        $dest = Join-Path $Out ($node.name)
        $outFs = [System.IO.File]::Create($dest)
        try {
            $left = $node.size
            [long]$lba = $node.lba
            while ($left -gt 0) {
                $n = [Math]::Min($left, $SECTOR)
                $chunk = Read-Sectors $lba 1
                if ($n -eq $SECTOR) {
                    $outFs.Write($chunk, 0, $SECTOR)
                } else {
                    $outFs.Write($chunk[0..($n - 1)], 0, $n)
                }
                $lba += 1
                $left -= $n
            }
        } finally { $outFs.Dispose() }
        Write-Host ("extracted {0} -> {1} ({2} bytes)" -f $node.name, $dest, $node.size)
        return
    }

    # default: iterative listing with a queue (deep trees would overflow the PS recursion stack)
    $queue = New-Object System.Collections.ArrayList
    $null = $queue.Add(@{ node = $root; prefix = '' })
    $visited = @{}
    while ($queue.Count -gt 0) {
        $item = $queue[0]
        $null = $queue.RemoveAt(0)
        $key = "$($item.node.lba):$($item.node.size)"
        if ($visited.ContainsKey($key)) { continue }
        $visited[$key] = $true
        $recs = Read-Dir $item.node.lba $item.node.size
        foreach ($rec in $recs) {
            if ($rec.isDir) {
                "{0,-16} {1,12} {2}" -f $rec.lba, $rec.size, "$($item.prefix)$($rec.name)/"
                $null = $queue.Add(@{ node = $rec; prefix = "$($item.prefix)$($rec.name)/" })
            } else {
                "{0,-16} {1,12} {2}" -f $rec.lba, $rec.size, "$($item.prefix)$($rec.name)"
            }
        }
    }
}
finally { $fs.Dispose() }
