param([string]$OutputFolder='Documentation/ArtUpgrade', [string]$LogPrefix='ArtCapture', [string[]]$Only=@())
$ErrorActionPreference = 'Stop'
$demoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$demoBinary = Join-Path $demoRoot 'Build/Windows/FoldlightDemo/Binaries/Win64/FoldlightDemo.exe'
$captureOutput = Join-Path $demoRoot $OutputFolder
New-Item -ItemType Directory -Path $captureOutput -Force | Out-Null
$captures = @(
    @{Name='Courtyard3D'; Room=0; View=0; Extra=@()},
    @{Name='Courtyard2D'; Room=0; View=0; Extra=@('-Capture2D')},
    @{Name='Mechanism2D'; Room=1; View=0; Extra=@('-Capture2D')},
    @{Name='Garden3D'; Room=2; View=0; Extra=@()},
    @{Name='Furnace3D'; Room=3; View=0; Extra=@()},
    @{Name='TowerSouth'; Room=4; View=0; Extra=@('-Capture2D')},
    @{Name='TowerEast'; Room=4; View=1; Extra=@('-Capture2D')},
    @{Name='TowerNorth'; Room=4; View=2; Extra=@('-Capture2D')},
    @{Name='TowerWest'; Room=4; View=3; Extra=@('-Capture2D')},
    @{Name='MageWalk'; Room=0; View=0; Extra=@('-Capture2D','-CaptureAction=Walk')},
    @{Name='MageRun'; Room=0; View=0; Extra=@('-Capture2D','-CaptureAction=Run')},
    @{Name='MageJump'; Room=0; View=0; Extra=@('-Capture2D','-CaptureAction=Jump')},
    @{Name='MageCast'; Room=2; View=0; Extra=@('-Capture2D','-CaptureAction=Cast')}
)
foreach ($capture in $captures) {
    if ($Only.Count -gt 0 -and $capture.Name -notin $Only) { continue }
    $captureLog = Join-Path $demoRoot ('Saved/Logs/'+$LogPrefix+$capture.Name+'.log')
    $captureArgs = @('-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-unattended','-nosplash','-nosound','-FoldlightCapture',('-CaptureRoom='+$capture.Room),('-CaptureView='+$capture.View),('-abslog='+$captureLog)) + $capture.Extra
    $captureProcess = Start-Process -FilePath $demoBinary -ArgumentList $captureArgs -WindowStyle Hidden -PassThru
    if (!$captureProcess.WaitForExit(60000)) {
        Stop-Process -Id $captureProcess.Id
        throw ('Capture timed out: '+$capture.Name)
    }
    $captureText = Get-Content -LiteralPath $captureLog -Raw
    if ($captureText -match 'missing bUsedWithInstancedStaticMeshes|Default Material will be used|Fatal error:|Failed to load.*Foldlight/(Art|External)') {
        throw ('Art resource or rendering failure: '+$captureLog)
    }
    $dimension = if ($capture.Extra -contains '-Capture2D') {'2D'} else {'3D'}
    $source = Join-Path $demoRoot ("Build/Windows/FoldlightDemo/Saved/Screenshots/Foldlight_room$($capture.Room)_view$($capture.View)_$dimension.png")
    foreach ($argument in $capture.Extra) {
        if ($argument -like '-CaptureAction=*') {$source=$source.Replace('.png',('_'+$argument.Substring(15)+'.png'))}
    }
    if (!(Test-Path -LiteralPath $source)) {throw ('Capture was not produced: '+$source)}
    if ((Get-Item -LiteralPath $source).LastWriteTime -lt $captureProcess.StartTime) {throw 'Stale capture'}
    Copy-Item -LiteralPath $source -Destination (Join-Path $captureOutput ($capture.Name+'.png')) -Force
    Write-Output ('Captured '+$capture.Name)
}
