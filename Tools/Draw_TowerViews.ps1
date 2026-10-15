Add-Type -AssemblyName System.Drawing
$out = Join-Path $PSScriptRoot '../Documentation/TowerThreeViews.png'
$bmp = New-Object System.Drawing.Bitmap 2100,1660
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = 'AntiAlias'
$g.Clear([System.Drawing.ColorTranslator]::FromHtml('#101d29'))
function Brush($c) { New-Object System.Drawing.SolidBrush ([System.Drawing.ColorTranslator]::FromHtml($c)) }
function Label($s,$x,$y,$size=20,$color='#dce7ed') {
 $f=New-Object System.Drawing.Font 'Microsoft YaHei',$size
 $b=Brush $color; $g.DrawString($s,$f,$b,[single]$x,[single]$y); $f.Dispose(); $b.Dispose()
}
function Line($x1,$y1,$x2,$y2,$color='#304555',$width=1) {
 $p=New-Object System.Drawing.Pen ([System.Drawing.ColorTranslator]::FromHtml($color)), $width
 $g.DrawLine($p,[single]$x1,[single]$y1,[single]$x2,[single]$y2); $p.Dispose()
}
# ID, local X/Y, top Z, width/depth, thickness; dimensions in UE centimetres.
$platforms=@(
 @('0',-1050,0,0,800,1100,180), @('1',-790,-450,70,320,340,250),
 @('2',0,500,140,400,400,60), @('3',450,-450,210,450,450,60),
 @('4',-1550,-30,280,350,380,60), @('5',-1050,390,350,450,450,60),
 @('6',-1500,900,420,380,350,60), @('7',-1950,-900,490,450,450,60),
 @('8',0,-1350,560,350,380,60), @('9',500,-1800,630,450,450,60))
$scale=.255
function View($mode,$ox,$oy) {
 foreach($v in $platforms) {
  $id=$v[0]; $x=$v[1]; $y=$v[2]; $z=$v[3]; $w=$v[4]; $d=$v[5]; $h=$v[6]
  if($mode -eq 'front') { $a=$x; $b=$z; $rw=$w; $rh=$h }
  elseif($mode -eq 'side') { $a=-$y; $b=$z; $rw=$d; $rh=$h }
  else { $a=$x; $b=$y+$d/2; $rw=$w; $rh=$d }
  $px=$ox+($a-$rw/2)*$scale; $py=$oy-$b*$scale
  $color=if($id -in @('3','5','7','9')){'#a67e41'}elseif($id -eq '1'){'#368a89'}else{'#47667e'}
  $br=Brush $color; $g.FillRectangle($br,[single]$px,[single]$py,[single]($rw*$scale),[single]($rh*$scale)); $br.Dispose()
  Label $id ($px+5) ($py-27) 15 '#ffffff'
 }
 # Moved pillar target: outlined, retaining the initial solid footprint.
 if($mode -ne 'side') {
  $px=$ox+(-450-160)*$scale
  $py=if($mode -eq 'front'){$oy-70*$scale}else{$oy-(-450+170)*$scale}
  $hh=if($mode -eq 'front'){250}else{340}
  $pen=New-Object System.Drawing.Pen ([System.Drawing.Color]::Turquoise),2
  $pen.DashStyle='Dash'; $g.DrawRectangle($pen,[single]$px,[single]$py,[single](320*$scale),[single]($hh*$scale)); $pen.Dispose()
 }
}
Label '四向折光塔 / 最后一关' 65 35 36
Label '关卡结构三视图 · 当前代码坐标 · 未折叠状态' 68 105 21 '#9db4c5'
Label '正视图 / 南视 · X—Z' 70 190 25
Label '侧视图 / 东视 · −Y—Z' 1100 190 25
foreach($z in @(0,140,280,420,560,700)) {
 $yy=500-$z*$scale
 Line 75 $yy 940 $yy; Line 1100 $yy 2010 $yy
 Label ([string]($z/100)+' m') 75 ($yy-24) 12 '#829dac'
}
View 'front' 700 500
View 'side' 1450 500
Label '俯视图 / X—Y' 70 630 25
View 'top' 710 1010
Label '+Y ↑      +X →' 80 700 17 '#9db4c5'
Label '平台编号与高度' 1110 680 26
$names=@('0  起点平台       0.00 m','1  移动石柱       0.70 m','2  南向阶梯       1.40 m','3  南向祭台       2.10 m','4  西向阶梯       2.80 m','5  西向祭台       3.50 m','6  北向阶梯       4.20 m','7  北向祭台       4.90 m','8  东向阶梯       5.60 m','9  终点祭台 / 核心  6.30 m')
for($i=0;$i -lt $names.Count;$i++) { Label $names[$i] 1115 (740+$i*43) 20 }
Label '登塔顺序：南 → 西 → 北 → 东' 1110 1210 22 '#e4bd7e'
Label '金色：祭台     青色：可移动石柱' 1110 1260 18
Label '虚线：石柱移动后的目标位置' 1110 1300 18
Line 80 1410 335 1410 '#dce7ed' 3
Line 80 1400 80 1420 '#dce7ed' 2
Line 335 1400 335 1420 '#dce7ed' 2
Label '10 m · 三视图使用相同缩尺' 80 1430 16
Label '依据 FDWorld.cpp 的碰撞平台绘制；省略装饰、角色与特效，非实机美术截图。' 670 1600 17 '#9db4c5'
$bmp.Save($out,[System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output ([System.IO.Path]::GetFullPath($out))

