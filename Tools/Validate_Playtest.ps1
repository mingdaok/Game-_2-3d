param([ValidateSet('Smoke','Traversal')][string[]]$Modes=@('Smoke','Traversal'))
$ErrorActionPreference = 'Stop'
$demoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$demoBinary = Join-Path $demoRoot 'Build/Windows/FoldlightDemo/Binaries/Win64/FoldlightDemo.exe'
if (!(Test-Path -LiteralPath $demoBinary)) { throw 'Package the Windows playtest before running this validation.' }
foreach ($mode in $Modes) {
    $demoLog = Join-Path $demoRoot "Saved/Logs/Packaged$mode.log"
    $demoArgs = @('-nullrhi','-nosound','-unattended',"-Foldlight$mode", "-abslog=`"$demoLog`"")
    $demoProcess = Start-Process -FilePath $demoBinary -ArgumentList $demoArgs -PassThru -WindowStyle Hidden
    if (!$demoProcess.WaitForExit(240000)) {
        Stop-Process -Id $demoProcess.Id
        throw "$mode validation timed out; see $demoLog"
    }
    $demoText = Get-Content -LiteralPath $demoLog -Raw
    if ($demoText -match 'FOLDLIGHT_(TEST|TRAVERSAL) FAIL' -or $demoText -notmatch 'FOLDLIGHT_(TEST|TRAVERSAL) COMPLETE') {
        throw "$mode validation failed; see $demoLog"
    }
    Write-Output "$mode validation passed: $demoLog"
}
