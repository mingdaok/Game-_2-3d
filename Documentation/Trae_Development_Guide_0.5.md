# 折光遗迹 UE5 从零开发至 0.5 试玩版实施文档

文档日期：2026-10-10。目标版本：0.5.0。适用对象：Trae 智能体及协作开发者。目标平台：Windows 64 位，Unreal Engine 5.7，C++ 项目。

这是一份可以交给开发智能体执行的重建说明。正文规定功能、算法、关卡数据、资源管线、实施顺序和验收标准；附录提供从当前工程提取的完整核心源文件、资源生成脚本和配置。无需依靠此前聊天记录推断需求。

**交付目标是可以独立启动、实际通关的中文 Windows 游戏，不是设计演示、只包含蓝图截图的工程，或只有相机切换的概念验证。** 当前基线的五关和机制已经实现；最后一次完整成品验证发生在 2026-09-20，结果为 101 项检查通过、自动物理通关五关、坠落 0 次。本文编写时读取了工程与历史记录，没有重新执行这些游戏测试。Trae 重建后必须产生自己的新验收记录，不能复用旧结论。

## 1. 给 Trae 的启动指令

将下面这段连同本文件一起交给 Trae：

> 请根据本文，在新的独立目录中从零建立 Unreal Engine 5.7 C++ 游戏《折光遗迹》，实现到本文定义的 0.5.0 试玩状态。先读取正文、工程文件清单和阶段计划，再按阶段读取附录对应文件。落实项目创建、代码、资源生成和导入、中文界面、四方向维度转换、五关解谜、魔法、骨骼动画、打包和真实运行验收。请实际修改文件和运行工具，不要只输出方案。保留现有项目。默认使用本文明确列出的 CC0 素材与可重建的原创素材，不购买付费内容。每阶段完成后记录实现和验证结果，再进入下一阶段。缺少本机路径时先探测安装位置；缺少非必要旧特效时按第 11 节采用明确的降级方案，并在交付中说明。不要伪造测试、截图或打包成功。所有游戏内玩家可见文本用简体中文。最终交付可运行工程、Windows 试玩包、操作说明、许可证、测试日志和实机截图。

### 1.1 执行约束

- 推荐新目录 `E:/UE5/Project/FoldlightTrae`；路径只是示例，先检查是否已存在，不能覆盖用户已有工程。
- 参考工程位于 `E:/UE5/Project/NEWMyProject/Workspaces/FoldlightDemo`。它可供比对；不能把改坏它当作“从零重建”。
- 所有功能先达到本文基线，再讨论新玩法。不要自行扩展为联网、开放世界、战斗 RPG 或无限关卡。
- 实施中自行解决常规编译、导入、路径和材质问题；遇到必须由用户提供的付费资源或账号时，先继续不依赖它的任务。
- 尽量分阶段读取附录，避免一次塞入所有源码造成上下文拥挤。记录已完成阶段，恢复任务时接着做。
- 附录源代码是可用的实施基线，不要求智能体凭描述重新猜写全部代码；允许在新工程中恢复这些源文件，再逐项运行验证。

### 1.2 完成定义

必须同时满足：标题页能进入游戏；五关能通关；四方向折叠和展开正确；中文不乱码；第三关起可施法；法师骨骼动画实际播放；环境与贴图随包发布；退出后可继续已到达的房间；Windows 包脱离编辑器仍能运行。自动测试、实际路线通关与画面检查各自独立完成。

## 2. 产品范围与玩家体验

玩家扮演携带法杖的低多边形法师，在遗迹中利用“观察方向改变空间关系”解谜。三维状态中，远近错开的平台彼此分离；折叠到二维后，当前观察方向上的纵深被投影到同一个移动平面，原本分开的平台形成可行走的侧视路线。玩家在脚下平台展开后，回到该平台真实的三维深度。

魔法在两个维度中具有不同形状：三维是宽而低的火印，二维是窄而高的火墙。玩家必须选择合适的维度，烧毁藤蔓、同时加热熔炉，并在第五关逐一激活方向印记。

| 项目 | 0.5 基线 |
| --- | --- |
| 类型 | 单人、第三人称三维探索与二维平台解谜 |
| 关卡 | 五个依次解锁的房间；同一张地图内运行时生成 |
| 二维方向 | 南、东、北、西四个固定世界方向 |
| 美术 | 风格化遗迹，原创几何与 KayKit 地牢资产结合 |
| 默认角色 | KayKit Mage，帽子、披风、法杖及骨骼动作 |
| UI | 全中文 Canvas HUD，标题、目标、提示、暂停、通关统计 |
| 音效 | 三段原创合成音效：折叠、施法、修复 |
| 存档 | 保存当前房间和最终通关状态 |
| 输入 | 键鼠 |
| 发布形式 | Windows Development 试玩包 |

本阶段不包含：冰或光魔法、战斗、敌人、生命值、背包、联网、任意角度折叠、移动端适配、配音、布料模拟、脚底 IK、完整商业发行流程。不要把下载角色包中存在的其他动画当作已经接入的玩法。

## 3. 操作与状态规则

| 按键 | 行为 | 限制 |
| --- | --- | --- |
| Enter | 标题开始或继续；暂停继续；结局重新开始 | 根据当前页面处理 |
| N | 开始新旅程 | 标题或结局页 |
| WASD | 三维移动 | 相对于控制器相机水平朝向 |
| A / D | 二维屏幕左 / 右移动 | 不直接写死世界 X 轴 |
| 鼠标 | 三维转向和俯仰 | 二维及折叠预览中禁用 |
| Space | 跳跃 | 输入缓冲与离台容错；预览时禁用 |
| 按住 V | 折叠预览 | 先停下角色；查看目标维度与安全提示 |
| 松开 V | 请求切换维度 | 安全检查通过后才改变碰撞和位置 |
| Q / F | 上一个 / 下一个观察方向 | 三维和预览可用；二维必须先展开 |
| G | 放置火印 / 火墙 | 第三关起；需要站稳；有冷却 |
| E | 移动石柱或激活出口 | 仅三维，需在交互距离内 |
| R | 重置当前房间 | 恢复机关、藤蔓、方向印记 |
| H | 逐级提示 | 每关三档，最高档明确给出解法 |
| Esc | 暂停 / 继续 | 标题和结局不按游戏内暂停处理 |
| F10 | 退出 | 不用关闭编辑器代替游戏退出 |

### 3.1 状态模型

世界保存 `bStarted / bPaused / bFinished`；角色保存 `b2D / bPreview / bTransition`。输入许可条件为：世界存在、已开始、未暂停、未结束、未处于转换锁定期。移动、跳跃、施法还要分别排除预览等不适用状态。

`bTransition` 是约 0.3 秒的操作锁定，不代表已经实现完整电影化镜头过渡。相机臂长用插值变化，投影模式与碰撞切换仍是明确的状态提交。

暂停时停止角色移动组件、冻结骨骼动画，并停止世界游戏计时、机关移动和燃烧倒计时。不要只显示“暂停”文字却继续执行谜题状态。

## 4. 技术路线与工程建立

### 4.1 基线环境

当前实现使用 UE 5.7、Visual Studio 2022 C++ 工具链和 Windows SDK。原机引擎路径是 `E:/UE5/UE_5.7`；Trae 应先探测本机安装位置，不要假定所有机器相同。外部 Python 用于音效、OBJ 和 GLB 预处理；其中 `prepare_kaykit.py` 需要 NumPy，其余几何和音效生成主要使用标准库。导入脚本由 UE 内置 Python 执行。

不要求安装 Blender。模型生成、GLB 配件绑定和资源导入都已有脚本。若改用 Blender，应说明额外依赖并保证最终结果和尺度一致。

### 4.2 项目和模块命名

项目文件使用 `FoldlightDemo.uproject`，两个 Target 分别为 `FoldlightDemo` 与 `FoldlightDemoEditor`。运行时模块保留名字 `NEWMyProject`，导出宏为 `NEWMYPROJECT_API`。这是当前基线的真实命名，不是笔误。

从零重建时，保留这些内部名字最省迁移工作。如果决定改成新模块名，必须同时修改 Build.cs、模块注册、Target、全部 API 宏、反射类路径及 DefaultEngine.ini。不能仅重命名项目文件。

启用插件：`PythonScriptPlugin`、`EditorScriptingUtilities`（仅 Editor Target），以及 `Niagara`。glTF 使用 UE 的 Interchange 导入管线；若安装版本默认未启用相应导入能力，先启用随引擎提供的 Interchange/glTF 支持，再验证 `AssetImportTask`。

依赖模块：公开依赖 `Core, CoreUObject, Engine, InputCore, EnhancedInput, Niagara`，私有依赖 `SlateCore, AnimGraphRuntime`。虽然依赖列表保留 EnhancedInput，当前输入实现采用 `UInputComponent` 传统轴与按键绑定。

### 4.3 空地图和运行时生成

创建 `/Game/Foldlight/Maps/Trial`，只需要基础地图和 `PlayerStart(-1100,0,95)`。默认 GameMode 是 `/Script/NEWMyProject.FDGameMode`。

`AFDGameMode` 指定 `AFDCharacter` 和 `AFDHUD`，在 `StartPlay` 确认世界中存在唯一 `AFDWorld`。`AFDWorld::BuildWorld()` 生成五关玩法对象、背景、灯光和魔法显示。不要另外手工摆出一套重复机关。

**本项目的核心不是蓝图图表。** 关卡、投影、魔法、HUD、动画图主要由 C++ 实现，UE Python 创建二进制资产。编辑器打开空地图时未立即看到所有运行时布景是正常的；Play 或 `-game` 才会生成。若后续需要策划可编辑性，可在达到基线后迁移到 DataAsset 或蓝图参数化。

## 5. 文件结构与责任划分

```text
FoldlightDemo.uproject
Launch_Playtest.bat
Source/
  FoldlightDemo.Target.cs
  FoldlightDemoEditor.Target.cs
  NEWMyProject/
    NEWMyProject.Build.cs
    NEWMyProject.h / NEWMyProject.cpp
    FoldlightDemo.h
    FDProjection.h
    FDCharacter.cpp
    FDObject.cpp
    FDWorld.cpp
    FDHUD.cpp
    FDVisuals.h
    FDArt.cpp
    FDMageAnimInstance.h / FDMageAnimInstance.cpp
    FDValidation.cpp
    FDFourViewTests.cpp
Config/
  DefaultEngine.ini / DefaultGame.ini / DefaultInput.ini
Tools/
  make_audio.py / make_models.py
  build_content.py / import_art.py / import_chinese_font.py
  prepare_kaykit.py / import_kaykit.py
  Build_Editor.bat / Package_Windows.bat
  Validate_Playtest.ps1 / Capture_Art.ps1
  GeneratedAudio/ / GeneratedModels/ / ExternalAssets/
Content/Foldlight/
  Maps/ / Fonts/ / Audio/ / Art/ / External/KayKit/
Build/Windows/
Documentation/
```

| 文件 / 类型 | 职责 |
| --- | --- |
| FoldlightDemo.h | 玩法类、枚举和状态字段声明 |
| AFDCharacter | 输入、移动、相机、维度请求、角色外观 |
| AFDObject | 源碰撞、投影碰撞、模型拼装、石柱、藤蔓、接收器与出口 |
| AFDWorld | 世界生成、合法落点、法术命中、房间进度、存档、提示 |
| FDProjection.h | 四方向共同使用的坐标基和投影计算 |
| FDVisuals / FDArt | 网格构件、材质、声音、实例化布景 |
| FDMageAnimInstance | 原生骨骼动画混合图和运行时代理 |
| AFDHUD | 中文 Canvas 界面 |
| FDValidation / FDFourViewTests | 机制断言、真实物理自动通关、截图模式 |

注意：`AFDGameMode` 的实现位于 `FDWorld.cpp` 末尾；`FDVisuals::Piece/Material/Sound` 实现在 `FDObject.cpp`。不应凭类名假设还缺少 `FDGameMode.cpp` 或 `FDVisuals.cpp`。

原工程还保留 `WaxCharacter`、`DimensionManagerComponent`、`ElementInteractable`、`Magic_FireRing` 等旧实现。它们不是新试玩的主链路，本文不要求从零复制这些旧类，也不要把旧逻辑和 FD 类同时挂上导致重复输入或双重维度控制。

## 6. 四方向二维和三维转换

### 6.1 坐标约定

所有距离单位为厘米，时间为秒。Z 向上。房间索引从 0 开始，房间原点为 `(Index * 5000, 0, 0)`。下文关卡表都是相对房间原点的数据。

| View | 名称 | 深度法线 N / 相机所在侧 | 屏幕向右 R | 相机朝向 | 压缩轴 |
| ---: | --- | --- | --- | --- | --- |
| 0 | 南视 | (0,1,0) | (1,0,0) | (0,-1,0) | Y |
| 1 | 东视 | (1,0,0) | (0,-1,0) | (-1,0,0) | X |
| 2 | 北视 | (0,-1,0) | (-1,0,0) | (0,1,0) | Y |
| 3 | 西视 | (-1,0,0) | (0,1,0) | (1,0,0) | X |

名称采用本游戏内部约定。南北和东西分别是镜像投影；单纯几何只有两种压缩轴，第五关通过方向印记进一步区分四个相机方向。

```text
Wrap(v) = (v % 4 + 4) % 4
R = (N.y, -N.x, 0)
ProjectedWidth(Size, v) = dot(Size, abs(R))
OnPlane(P, v, d) = P + N * (d - dot(P,N))
CameraYaw(v) = rotation(-N).yaw
```

输入、平台宽度、碰撞、摄像机、安全检查、施法预览和命中必须共用这些函数。禁止在某个子系统偷偷写死 X 或 Y，否则东西视角和非正方形平台必然出错。

### 6.2 源实体和投影代理

每个 `AFDObject` 有两个 BoxComponent：`Solid` 是真实三维碰撞，作为根组件；`Projection` 是当前平面的代理碰撞。美术挂在单独的 `VisualRoot` 下，不靠美术网格承担角色碰撞。

折叠时保持原物体和美术的真实位置，通过正交相机让它们在画面中重合；仅将代理碰撞投影到玩家平面。这样不会破坏源坐标，展开也不需要猜测“之前在哪里”。

```text
PlaneDepth = dot(PlayerLocation, N)
ProjectionCenter = OnPlane(SourceCenter, View, PlaneDepth)
ProjectionExtent = abs(R) * ProjectedWidth(Size,View)/2
                 + abs(N) * 70
                 + (0,0,Size.z/2)
```

这里代理厚度为 140 cm，70 是半厚度。`Size` 是完整尺寸，`Extent` 是半尺寸，不能混用。

三维时禁用 Projection，Solid 阻挡 Pawn。二维时 Solid 对 Pawn 忽略，Projection 在物体 `Blocks()` 为真时启用。烧毁藤蔓必须同时取消真实与投影碰撞，不能只隐藏网格。

### 6.3 预览与正式提交

按下 V：停止角色移动，设置预览状态。三维预览采用所选正交视角；Q/F 可更换预览方向。此时仍未提交投影碰撞，也未传送到平台源深度。

松开 V：退出预览临时相机状态，然后执行 `TrySwitch()`。先算目标、检查合法性，失败则保留原位置和维度并提示原因；成功才执行 `ApplyDimension()`。

输入预览并不是对场景额外渲染一个副本，而是使用正交观察检查将要折叠的轮廓。这一点应体现在实现和提示中。

### 6.4 三维到二维

要求角色 `IsMovingOnGround()`；当前房间不能有移动中的石柱。检查角色投影后是否与阻挡物重叠。

通过后：保存 PlaneDepth；投影当前房间碰撞；设置移动平面法线 N 和原点；开启平面约束；令角色朝向 R 或 -R；相机改为正交，宽 1400；停止当前速度；转换锁定 0.3 秒。`bUsed2D` 置真，折叠次数增加。

相机弹簧臂使用 `SetAbsolute(false,true,false)` 固定世界旋转，关闭 PawnControlRotation 和相机碰撞。不能仅设置相机角度却让角色旋转继续影响父组件。

### 6.5 二维到三维

通过 `CharacterMovement.CurrentFloor.HitResult.GetActor()` 找到实际脚下的 `AFDObject`。它必须属于当前房间且可行走。不能选择“最近的网格”“最深的平台”或全局最高平台替代真正支撑源。

```text
Target = OnPlane(PlayerPosition, View, dot(SupportCenter,N))
Target.z = Support.Top + CapsuleHalfHeight + 2
```

目标在平台横向边界内，基线保留 8 cm 边缘裕量；再检查真实三维目标是否被其他阻挡物占据。安全检查跳过当前支撑平台，以免把正常站立判成穿透。

通过后：取消平面约束，恢复源碰撞，关闭代理；把角色放到 Target；恢复透视相机与控制器旋转；将运动状态设为 Falling，让 CharacterMovement 重新解析地面。

### 6.6 安全检测和失败提示

当前用胶囊外接的保守包围范围检查 AABB。对每个有效阻挡物，用 `abs(Delta.X/Y/Z)` 与物体半尺寸加胶囊半径/半高比较，接触容差为 2 cm。目标二维时先把障碍中心和尺寸转换成代理表示。

至少覆盖：空中禁止切换、石柱运动中禁止切换、折叠后穿障碍禁止、无有效脚下源禁止展开、边缘危险位置禁止展开、三维落点已占用禁止展开。失败不能改变已有维度、位置或法术寿命。

## 7. 角色控制与相机参数

| 参数 | 基线值 |
| --- | ---: |
| 胶囊半径 / 半高 | 30 / 86 |
| 最大移动速度 | 420 cm/s |
| JumpZVelocity | 540 cm/s |
| GravityScale | 1.5 |
| AirControl | 0.75 |
| 步行减速度 | 2000 |
| MaxStepHeight | 32 |
| 三维朝向转速 | Yaw 800 |
| 二维朝向转速 | Yaw 10000 |
| 跳跃输入缓冲 | 0.14 秒 |
| 离台容错 | 0.10 秒 |
| 透视相机臂长 / FOV | 720 / 65° |
| 二维目标臂长 / OrthoWidth | 1600 / 1400 |
| 相机 TargetOffset | (0,0,45) |
| 三维默认俯角 | -27° |
| 俯仰范围 | -65° 至 10° |
| 臂长插值速度 | 12 |
| 手动曝光补偿 | 9.5 |

跳跃通过短时缓冲配合 `LaunchCharacter((0,0,540), false, true)` 实施。角色视觉动画不能改变胶囊体尺寸，也不能用动画根运动替代这里的物理移动。

二维只接受屏幕水平移动；三维使用控制器 yaw 的前向和右向。相机移动与实际角色方向要分别检查，特别是北视和西视的左右镜像。

## 8. 玩法对象和机关生命周期

| Kind | 可行走 | 阻挡 | 用途 |
| --- | --- | --- | --- |
| Platform | 是 | 是 | 平台与浮空石板 |
| Pillar | 是 | 是 | 可移动轨道石柱 |
| Vine | 否 | 未烧毁时是 | 可燃封印 |
| Receiver | 否 | 否 | 双炉加热器、方向印记 |
| Exit | 否 | 否 | 过关交互标记和拱门 |
| Decoration | 否 | 否 | 纯显示对象 |

每个对象保存稳定 `ObjectId`、所属 Room、完整 Size、InitialLocation、SlideTarget 和状态。ObjectId 是谜题条件和测试使用的主键，不要在美术调整中随意改名。

石柱仅在三维距离小于 285 cm 时被 E 激活，按照 360 cm/s 匀速移动到 SlideTarget。到达误差小于 1 cm 时吸附到目标并标记 bMoved。移动期间任何维度转换都拒绝。

藤蔓首次受到合法火焰命中后开始 1.5 秒燃烧，期间改变材质；结束后隐藏视觉、清理附着粒子、禁用两套碰撞。反复 Ignite 不重置燃烧时间。

重置时恢复位置、原始材质和初始碰撞，清除移动、燃烧、发光、激活状态及临时粒子。只恢复颜色而不恢复碰撞是不合格实现。

## 9. 火魔法

### 9.1 释放与锚点

第三关（索引 2）解锁。角色必须站稳，当前冷却为零；每次成功施法持续 4 秒，冷却 0.45 秒。场中只保留一个法术，新法术替换旧法术。

从角色前方 230 cm 计算锚点；二维朝向必须是 ±R，三维使用角色前向。高度先取脚底，在法术位置下方搜索当前房间可行走平台，选不高于脚底 +45 cm 的最高顶面，并把锚点放在顶面 +3 cm。没有候选时保留脚底高度。

分别保存：

- `FireSourceAnchor`：完整、持久的真实三维锚点。
- `FireAnchor`：当前显示和命中使用的锚点。
- `FireRemaining`：剩余时间，不因维度变化重置。

二维首次施法时，使用脚下支撑源的真实深度建立 FireSourceAnchor。以后二维显示使用 `OnPlane(FireSourceAnchor, View, PlaneDepth)`，三维直接恢复源锚点。不要把投影结果反复覆盖源锚点，否则多次切换会漂移。

### 9.2 命中形状

| 状态 | 形状与尺寸 | 判定 |
| --- | --- | --- |
| 三维 | 半径 200 cm、高 70 cm 的低位圆柱范围 | XY 圆对目标 AABB 最近距离，加 Z 重叠 |
| 二维 | 宽 120 cm、高 440 cm 的竖直墙 | 屏幕横向与 Z 重叠，忽略原始深度 |

二维中心为 Anchor + (0,0,220)。横向条件为 `abs(dot(TargetCenter-Center,R)) < 60 + ProjectedWidth(TargetExtent,View)`，竖向为 `abs(DeltaZ) < 220 + ExtentZ`。

三维中心为 Anchor + (0,0,35)。令 `dx=max(abs(DeltaX)-ExtentX,0)`、`dy=max(abs(DeltaY)-ExtentY,0)`，检查 `dx²+dy² <= 200²` 且 `abs(DeltaZ)<35+ExtentZ`。

只检测当前房间的 Vine 和 Receiver，其他房间不能受到火墙跨深度误伤。

### 9.3 两类接收器

普通 Receiver 被命中时刷新 `LitRemaining=0.12` 秒。第四关要求两个低位接收器持续同时点亮超过 0.65 秒；中断则累计清零，完成后本关锁存 `bFurnaceSolved=true`。

方向 Receiver 设置 `RequiredView`。只有二维火墙且方向吻合时设置 `bActivated=true`；三维火印、错误方向均无效。方向激活在当前房间内保持，R 或坠落重置会清除。

第四关高处藤蔓 `r3_vine` 必须先满足双炉条件，不能绕过前半谜题直接烧掉。

### 9.4 显示与命中解耦

法术核心显示由 20 个小网格、2 个细线符印和 12 个字形条组成：前 12 个为火焰，后 8 个为余烬。使用半透明 Unlit 材质，Opacity=0.62，颜色约为 `(1,0.24,0.035)*1.8`。

二维火焰沿高墙上升，三维沿地面圆环分布。出生 0.15 秒淡入，结束前 0.3 秒淡出。网格全部无碰撞，视觉摆动不能改变命中范围。

未施法且魔法已解锁时显示范围预览；按 V 的维度预览期间隐藏法术预览。预览应能帮助玩家判断双炉覆盖和悬空藤蔓高度。

## 10. 五关完整布局

### 10.1 通用数据约定

以下坐标均为相对房间原点，单位 cm。普通平台使用 `Top` 描述顶面，而不是中心 Z：

```text
Height = Top + 180
CenterZ = Top - Height/2
Size = (SX,SY,Height)
```

因此普通平台底部固定在 Z=-180。第五关后续浮空石板厚度固定为 60，中心 Z=Top-30。若把这些高层石板做成落地立柱，会挡住其他方向的投影路线。

所有出口 ID 为 `exit<RoomIndex>`，中心为 `(X,Y,Top+8)`，逻辑尺寸 `(180,180,10)`。出口不提供地面碰撞；玩家需站在对应平台上。

### 10.2 第一关 初次折叠（Room 0）

| ID | X | Y | Top | SX | SY |
| --- | ---: | ---: | ---: | ---: | ---: |
| r0_start | -950 | 0 | 0 | 900 | 1100 |
| r0_a | -300 | -500 | 70 | 420 | 350 |
| r0_b | 100 | 500 | 140 | 380 | 350 |
| r0_c | 490 | -450 | 210 | 400 | 350 |
| r0_goal | 900 | 550 | 280 | 450 | 450 |

出口 `(940,550,Top=280)`。解法：南视折叠，沿屏幕右侧阶梯跳到终点；展开到终点平台真实深度，再 E。通关条件为本关成功使用过二维 `bUsed2D`。

### 10.3 第二关 错位之桥（Room 1）

| ID | 类型 | X | Y | Top | SX | SY |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| r1_start | Platform | -950 | 0 | 0 | 900 | 1200 |
| r1_a | Platform | -320 | 450 | 70 | 360 | 350 |
| r1_pillar | Pillar | -730 | -450 | 140 | 320 | 340 |
| r1_c | Platform | 500 | 450 | 210 | 400 | 350 |
| r1_goal | Platform | 900 | 550 | 280 | 450 | 450 |

石柱目标 X=80，Y 和高度不变；出口 `(940,550,280)`。解法：先在三维靠近起点左后方石柱，E 移动，等停稳，再南视折叠跳过阶梯。通关条件为石柱 bMoved 且 bUsed2D。

### 10.4 第三关 余烬花园（Room 2）

| ID | 类型 | 数据 |
| --- | --- | --- |
| r2_start | Platform | X=0，Y=0，Top=0，SX=2650，SY=1100 |
| r2_vine | Vine | 中心 (100,0,190)，尺寸 (90,1060,380) |
| exit2 | Exit | X=1030，Y=0，Top=0 |

推荐在 `(-140,0)` 附近朝 +X 施法；锚点前移后覆盖藤蔓。等待烧毁，穿过封印到出口。通关条件为 r2_vine.bBurned。这里教学三维低位火印。

### 10.5 第四关 双生熔炉（Room 3）

| ID | 类型 | 数据 |
| --- | --- | --- |
| r3_start | Platform | X=-250，Y=0，Top=0，SX=2300，SY=1200 |
| r3_left | Receiver | 中心 (-650,0,38)，尺寸 (60,60,76) |
| r3_right | Receiver | 中心 (-350,0,38)，尺寸 (60,60,76) |
| r3_vine | Vine | 中心 (450,450,410)，尺寸 (80,350,380) |
| r3_step | Platform | X=780，Y=-430，Top=70，SX=280，SY=350 |
| r3_goal | Platform | X=1080，Y=450，Top=140，SX=350，SY=450 |
| exit3 | Exit | X=1110，Y=450，Top=140 |

高处藤蔓上方还有纯视觉横梁，中心 `(450,450,615)`、尺寸 `(140,450,25)`，不能增加阻挡碰撞。

解法：三维站在 `(-730,0)` 附近面朝 +X 施法，火印中心约 -500，同时覆盖两炉；保持加热达标。然后到 `(220,0)` 附近南视折叠，向 +X 施放高火墙，烧掉不同深度上的悬空藤蔓。继续跳到终点，展开并 E。通关条件为双炉已修复且藤蔓已烧毁。

### 10.6 第五关 四向折光塔（Room 4）

| ID | 类型 | X | Y | Top | SX | SY |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| r4_start | 普通平台 | -1050 | 0 | 0 | 800 | 1100 |
| r4_pillar | 石柱 | -790 | -450 | 70 | 320 | 340 |
| r4_b | 60 厚石板 | 0 | 500 | 140 | 400 | 400 |
| r4_south | 60 厚祭台 | 450 | -450 | 210 | 450 | 450 |
| r4_w1 | 60 厚石板 | -1550 | -30 | 280 | 350 | 380 |
| r4_west | 60 厚祭台 | -1050 | 390 | 350 | 450 | 450 |
| r4_n1 | 60 厚石板 | -1500 | 900 | 420 | 380 | 350 |
| r4_north | 60 厚祭台 | -1950 | -900 | 490 | 450 | 450 |
| r4_e1 | 60 厚石板 | 0 | -1350 | 560 | 350 | 380 |
| r4_goal | 60 厚祭台 | 500 | -1800 | 630 | 450 | 450 |

石柱目标 X=-450。出口 `(500,-1800,630)`。

| 阶段 | View | 路线 | 祭台后操作 |
| --- | ---: | --- | --- |
| 南 | 0 | start → pillar → b → south | 向屏幕右侧施法，展开，选西 |
| 西 | 3 | south → w1 → west | 施法，展开，选北 |
| 北 | 2 | west → n1 → north | 施法，展开，选东 |
| 东 | 1 | north → e1 → goal | 施法，展开，在出口 E |

方向索引按 `[南,东,北,西]` 保存，通关路线按 `[南,西,北,东]`，二者不可混淆。

每个方向印记 ID 为 `r4_seal<View>`，RequiredView 对应 0/1/2/3，尺寸 `(70,70,90)`。印记中心由祭台**中心**加 `R*230+(0,0,150)` 得出，而不是顶面加 150。精确中心如下：

| ID | 中心 | 对应祭台 |
| --- | --- | --- |
| r4_seal0 | (680,-450,330) | r4_south |
| r4_seal1 | (500,-2030,750) | r4_goal |
| r4_seal2 | (-2180,-900,610) | r4_north |
| r4_seal3 | (-1050,620,470) | r4_west |

通关要求起点石柱已移动，四个印记全部激活。到最高祭台展开为三维才能按 E 完成。

## 11. 美术、字体和声音资源管线

### 11.1 原创可重建资源

`make_models.py` 生成 15 个模型：SM_Stone、SM_Arch、SM_Ring、SM_Pedestal、SM_Crystal、SM_Flame、SM_Wax、SM_Cape、SM_Root、SM_Leaf、SM_Grass、SM_Banner、SM_Gear、SM_BrokenArch、SM_Sigil。

它们使用约 100 cm 建模包络，运行时通过 `Size/100` 缩放。保留 OBJ 源文件。导入时合并网格、不导入外部材质、不自动生成碰撞、不转换坐标系。游戏碰撞继续使用 Box/Capsule。

`make_audio.py` 生成 22050 Hz、单声道、16 位 PCM 的 S_Fold、S_Fire、S_Restore。`build_content.py` 导入并把 SoundWave 压缩类型设为 PCM，同时建立 Trial 地图和基础材质。不要把没有下载来源的商业音乐混进交付。

### 11.2 KayKit 官方下载

资源来源为作者官方仓库，基线使用 2026-09-20 下载的免费 CC0 包：

- 角色：https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0
- 地牢：https://github.com/KayKit-Game-Assets/KayKit-Dungeon-Remastered-1.0
- 角色 ZIP：https://codeload.github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0/zip/refs/heads/main
- 地牢 ZIP：https://codeload.github.com/KayKit-Game-Assets/KayKit-Dungeon-Remastered-1.0/zip/refs/heads/main

| 归档名称 | 字节数 | SHA-256 |
| --- | ---: | --- |
| KayKitAdventurers.zip | 6141833 | e19176d799036452bc783c00dd1036e158c66c5a43b7285ccbfea3466d6f57bf |
| KayKitDungeon.zip | 6312130 | 8e92aa460f5be4252dff0cf030f92846ab74b875ce4756076ecce75b5d0c9bee |

main 是可变分支。若新下载哈希不同，不可声称与历史包完全一致，也不可直接套用固定节点编号的 GLB 转换。优先使用用户已有的上述归档；否则检查新版 GLB 的节点、骨架和动画名称，调整转换脚本并记录新哈希。本文没有编造一个未验证的固定提交号。

目录必须与脚本对应：

```text
Tools/ExternalAssets/
  KayKitAdventurers.zip
  KayKitDungeon.zip
  KayKitAdventurers/<archive-root>/addons/kaykit_character_pack_adventures/
  KayKitDungeon/<archive-root>/addons/kaykit_dungeon_remastered/
  Prepared/
```

从角色包取 `Characters/gltf/Mage.glb` 与 `Textures/mage_texture.png`。地牢网格来自 `Assets/gltf`，图集来自 `Assets/texture/dungeon_texture.png`。读取归档内作者 LICENSE.txt，并把原文随包复制，不能仅在说明里写一句“网上免费”。

### 11.3 角色 GLB 预处理

原始模型包含多个蒙皮身体部件和骨骼下的静态配件。`prepare_kaykit.py` 保留原文件，输出 `Prepared/Mage.glb`：

1. 解析 JSON 与 binary chunk，保留坐标、蒙皮、UV 和动画数据。
2. 将帽子、披风、法杖的位置烘焙到 Rig 空间。
3. 根据逆转置矩阵转换法线；重新生成需要的蒙皮属性，不保留失效的切线数据。
4. 配件每顶点绑定作者对应骨骼，权重为 1：帽子 head、披风 chest、法杖 handslot.r。
5. 将配件从骨骼子节点改为 Rig 下的 skinned mesh，避免导入后静态附件留在原地。
6. 隐藏书本和魔杖变体，避免多个武器同时出现。
7. 保留 8 段动画；移除 root 平移通道，角色位移由 CharacterMovement 控制。
8. 输出下载哈希和原始许可证副本。

现有脚本针对该归档使用 Rig=53、root=46，配件/骨骼对为 `(14,15)、(16,17)、(8,9)`。这些是**特定资源的索引，不是通用 glTF 标准**。换包时必须重新解析验证。

### 11.4 导入后的路径契约

法师网格：`/Game/Foldlight/External/KayKit/Mage/Mage/SkeletalMeshes/Mage.Mage`。

动画同目录，名称为 `MageIdle`、`MageWalking_A`、`MageRunning_A`、`MageJump_Start`、`MageJump_Idle`、`MageJump_Land`、`MageSpellcast_Shoot`、`MageInteract`。最后一段仅预留，当前没有交互动作状态。

环境路径：`/Game/Foldlight/External/KayKit/Environment/SM_KK_<name>.SM_KK_<name>`，12 个 name 为：

```text
pillar_decorated
wall_arched
wall_broken
torch_mounted
shelf_small_candles
table_small_decorated_A
barrel_large_decorated
barrel_small_stack
candle_triple
rubble_large
wall_archedwindow_open
shelves
```

`import_kaykit.py` 枚举实际结果并输出 `Prepared/ue_assets.json`。如果 UE 导入命名改变，应统一修复资源命名或代码中的加载路径，不能只让编辑器预览正常而忽略打包时硬编码路径。

### 11.5 材质

| 材质 | 要点 |
| --- | --- |
| M_Surface | Tint、Glow、Grain、Roughness、Metallic 参数，石纹世界噪声 |
| M_Spell | Translucent、Unlit、Opacity=0.62、Glow 参数 |
| M_KK_Mage | 作者图集，Roughness=0.85，少量贴图自发光系数 0.20 |
| M_KK_Dungeon | 作者图集，Roughness=0.85，自发光系数 0.10 |

实例化材质必须设置 `used_with_instanced_static_meshes`；角色材质设置 `used_with_skeletal_mesh`。缺失标记会导致成品材质回退。材质节点的编辑属性用 `set_editor_property`，例如 Multiply 的 `const_b`，不要假定 Python 直接属性赋值总有效。

石材 roughness 约 0.82，黄铜约 0.36、metallic 约 0.55，植物和布料强制非金属。魔法亮度不能盖住平台边缘或让火墙看起来比实际碰撞范围宽很多。

### 11.6 中文字体

从本机 UE 内容目录 `Slate/Fonts/DroidSansFallback.ttf` 导入 FontFace，保存为 `/Game/Foldlight/Fonts/DroidSansFallback`，LoadingPolicy=INLINE。运行时生成 CompositeFont，用 `FSlateFontInfo` 在最终字号下绘制，不是把小号字形纹理放大。

把对应 Android Open Source Project 字体许可证放入 Documentation/Fonts，并随 Windows 包复制。附录收录当前许可证文本供核对。字体须真正进入打包资源，不能依赖编辑器电脑的系统字体。

### 11.7 旧 Niagara 火焰资源的边界

当前参考工程保留 `/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Big_Simple`。它用于藤蔓燃烧时的附加粒子；主火印/火墙已改为程序网格，并不依赖它显示。该旧资产不是 KayKit 包的一部分，本文没有把它冒称为 CC0，也没有提供未经核验的下载链接。

从零重建有两条明确路径：

1. 用户同时提供原工程该资产及可使用的许可时，通过 UE Migrate 迁移它及引用依赖，保留现有 LoadObject 路径。
2. 只有本 MD 时，采用独立资源基线：删除 `FDWorld::BuildWorld` 中对该旧 NiagaraSystem 的 SetAsset/LoadObject；删除 `AFDObject::Ignite` 中加载并 Spawn 该旧 NiagaraSystem 的整个可选块；保留藤蔓燃烧材质、1.5 秒计时、销毁/重置及魔法网格显示；移除 DefaultGame.ini 对旧 Shared 目录的 AlwaysCook 项。Niagara 模块和组件可暂时保留以减少源码改动。

第二种方式可以完成全部玩法和测试，但少了旧版藤蔓附加粒子。若要求画面也补齐，在新工程自行制作燃烧 NiagaraSystem 并接入，明确标注为重建的特效，而不是宣称复原了未提供的二进制资产。最终日志不能持续出现找不到该资产的警告。

## 12. 骨骼动画实现

网格相对胶囊位置 `(0,0,-86)`，Yaw=-90°，统一缩放 0.65。关闭网格碰撞，胶囊继续主导移动。设置 `AlwaysTickPoseAndRefreshBones`，支持截图与无渲染验证。法杖、帽子和披风随作者骨骼绑定，不另外做每帧位置猜测。

`UFDMageAnimInstance` 持有强引用的 Clips 数组；`FFDMageProxy` 使用 `FAnimNode_SequencePlayer_Standalone`，通过 `GetCustomRootNode`、`GetCustomNodes` 注册原生图。不要把依赖 AnimBlueprint 常量折叠数据的普通 SequencePlayer 节点直接当成此方案替代品。

```text
Walk ─┐
      ├─ Gait(走/跑) ─┐
Run ──┘              ├─ Moving(待机/移动) ─┐
Idle ────────────────┘                    │
JumpStart ─┐                             ├─ Air ─┐
           ├─ Jump(起跳/空中) ────────────┘       ├─ Landing ─┐
JumpLoop ──┘                                      │          │
Land ─────────────────────────────────────────────┘          ├─ UpperBody → Output
Cast ────────────────────────────────────────────────────────┘
```

| 动作 | 循环 | 驱动 |
| --- | --- | --- |
| Idle | 是 | 低水平速度 |
| Walking_A | 是 | 速度较低；与 Run 共用 Gait 同步组 |
| Running_A | 是 | 速度提升到正常移动速度 |
| Jump_Start | 否 | 刚进入 Falling，2.5 倍播放速率 |
| Jump_Idle | 是 | 空中持续 |
| Jump_Land | 否 | Falling 结束，2 倍播放速率，短时混合 |
| Spellcast_Shoot | 否 | CastSerial 改变后重播，上半身叠加 |

走跑权重为 `clamp((Speed-140)/230,0,1)`；移动权重目标为 `clamp(Speed/100,0,1)`。起跳约 0.12 秒后开始转入空中姿势。落地窗口 0.32 秒；施法窗口 0.9 秒，约 0.08 秒淡入、0.18 秒淡出。

Layered Bone Blend 从 `spine` 分支往上混合，启用 MeshSpaceRotationBlend。这样施法时腿部仍用移动图。所有从角色读取的运动状态在 PreUpdate 中获取，不要在工作线程随意访问 Actor。

保留 `-CandleHero` 参数作为旧蜡烛外观对比；它是程序拼装角色，不是第二套骨骼角色。正式资源检查默认验证 Mage，不应带这个参数运行 101 项基线检查。

## 13. 环境、灯光和四向可读性

五关各有一个 SceneryRoot，重复物体用 InstancedStaticMeshComponent 分批，所有背景关闭游戏碰撞。只有当前房间背景可见；它不参与玩法投影。

背景需要随着当前方向旋转到相机后侧对应位置。按照所有当前玩法对象在 N 上的最远背面，计算安全背景深度，再保留装饰前缘安全间隔。不能只对南视摆一堵背墙，否则换北视时墙会挡在镜头前。

当前世界坐标基的实现为：BackdropDepth 初始取玩家深度；对每个当前房间对象取 `min(BackdropDepth, dot(Center,N)-dot(Extent,abs(N))+500)`；背景原点投影到这个深度，旋转到 R 朝向。最靠前的布景仍约在局部 Y=-950，因此至少保留约 450 cm 的玩法间隔。

| 房间 | 环境识别元素 |
| --- | --- |
| 初次折叠 | 青绿色布幡、拱廊、黄铜星盘、桌与烛台 |
| 错位之桥 | 蓝灰石材、远景齿轮、桶堆、置物架 |
| 余烬花园 | 绿色遗迹、藤根、植物、碎石和破墙 |
| 双生熔炉 | 暖褐色石材、炉栅、暖色火光、储物陈设 |
| 四向折光塔 | 蓝紫布幡、星象线、环形天体仪和四向祭台 |

专业石柱使用等比例模块拼接，不能把剑盾装饰沿 Z 拉伸数倍。平台仍采用原创倒角石块、金边、接缝和边角植物，这些低矮细节不能覆盖跳跃着陆区。

渲染基线为 DX11 / SM5、关闭 Lumen 与硬件光追、关闭虚拟阴影、关闭自动曝光和运动模糊、保留 Bloom。先以中等笔记本显卡可用为目标，不默认把影视级配置当作试玩要求。

## 14. 中文 HUD、提示和存档

HUD 按 720 高逻辑画布缩放。标题页包含开始/继续、新旅程、退出；游戏中显示关卡名与动态目标、维度与方向、五关进度、魔法状态、四向印记、E 情境提示、底部操作说明和 Toast。暂停页和结局页必须能恢复或重新开始。

结局统计本次 SessionTime、Switches、Falls。三次 H 的提示从观察线索逐步推进到明确路线。所有按钮、状态和错误提示用简体中文；内部类名和资源名不需要汉化。

存档类 `UFDSave`：Room=0、Finished=false、Version=1；槽名 `FoldlightTrial_v1`。进入下一关时持久化当前房间，完成第五关保存 Finished。读不到存档或版本不匹配就创建新存档；Room 夹在 0–4。

本版不保存房间中途机关状态。R 或 Z<-600 的坠落恢复本关起点、三维、南视，清除机关、藤蔓和方向印记。坠落增加 Falls。继续游戏从保存房间的初始状态进入。

测试和截图模式禁止写入用户正常存档；`Persist()` 排除 bSmoke/bCapture。不要用自动通关测试污染玩家进度。

已发现的基线文案小问题：第三关第二档提示仍写“蜡烛前方”。从零交付时应改为“角色前方”或“法师前方”；附录保留源文件原文以方便区分代码快照和建议修正。

## 15. 从零执行顺序

### 阶段 A 建立可编译工程

建立空 C++ 项目或按附录恢复 `.uproject`、Source、Config。先完成模块、Target、反射类和插件设置，生成项目文件并编译 Editor Target。地图尚未建立时，不要把默认地图缺失当成编译错误；首次资源创建可启动编辑器默认空场景。

验收：Editor Target 编译成功，类可被反射，未加载旧项目默认 Pawn。

### 阶段 B 建立基础资源和中文界面

运行音效和模型生成脚本，再依次执行 build_content、import_art、import_chinese_font。检查 Trial、PlayerStart、M_Surface、M_Spell、15 个网格、3 段音效和内嵌字体均存在。

如果直接恢复附录全部源码，代码已经包含 KayKit 加载分支，应把阶段 F 的下载、预处理和导入提前到首次游戏启动前完成，然后仍按 C–F 分别验收功能。若逐步自己实现代码，尚未接入的外部布景不要提前启用；角色可用 `-CandleHero` 临时验证。不要让阶段性缺资源日志混入最终验收。

验收：游戏能进入中文标题和第一关；文字清楚；平台有真实碰撞；声音和模型不存在缺失加载。

### 阶段 C 核心投影机制

先用第一关灰盒核对四方向坐标、左右输入、正交相机和源/投影碰撞。再完成安全展开、预览、禁止空中切换、石柱运动锁定和火焰锚点前置接口。

验收：对四个方向分别测试非正方形平台、真实左右移动、展开源深度、被占用落点拒绝。不能只截图证明“四面都能看”。

### 阶段 D 五关与魔法

按第 10 节加入全部布局，实现石柱、藤蔓、双炉、四方向印记、通关、重置、提示和存档。先测试每关独立机制，再接连续流程。

验收：必须通过第二关石柱、第三关燃烧、第四关形状差异和第五关四面登塔。三维火印不得激活方向印记，错误方向也不得激活。

### 阶段 E 环境表现

接入原创材质和模型、实例化背景、平台细节、法术流焰与环线。检查五关和四方向的可见性，尤其最高出口背后布景的遮挡。

验收：主路线、平台边缘、角色和机关可辨认，所有装饰无玩法碰撞，打包后没有默认材质回退。

### 阶段 F 外部美术和骨骼法师

下载/核验归档；运行 prepare_kaykit；用完整 Editor 运行 import_kaykit；检查资源清单，再启用默认 Mage 和原生动画图。核对 0.65 尺度、-90° 朝向、配件绑定、移动与施法混合。

验收：站立不是 T Pose；走、跑、起跳、空中、落地和施法均有实际骨骼变化；法杖不脱手；暂停停止动作；没有额外网格碰撞。

### 阶段 G 打包与新验证

打包 Windows Development，测试成品实际二进制，而不只运行 Editor。运行机制测试、真实物理通关和截图；保留所有日志；手工检查关键操作与退出继续。复制全部许可证。

验收：第 17 节清单通过，输出真实失败/通过记录和可运行入口。开发中不要先写“已完成”再补资源。

## 16. 构建、导入和运行命令

以下为 PowerShell 示例；先把路径改为本机的实际新项目。命令中的工作目录须设为新工程根目录。

```powershell
$EngineRoot = 'E:/UE5/UE_5.7'
$ProjectRoot = 'E:/UE5/Project/FoldlightTrae'
$ProjectFile = Join-Path $ProjectRoot 'FoldlightDemo.uproject'
Set-Location -LiteralPath $ProjectRoot

# 生成原始资源
python Tools/make_audio.py
python Tools/make_models.py

# 编译 Editor，先确认 VS C++ 工具链已安装
& "$EngineRoot/Engine/Build/BatchFiles/Build.bat" FoldlightDemoEditor Win64 Development "-Project=$ProjectFile" -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { throw 'Editor build failed' }
```

第一次下载与展开示例：

```powershell
$AssetRoot = Join-Path $ProjectRoot 'Tools/ExternalAssets'
New-Item -ItemType Directory -Path $AssetRoot -Force | Out-Null
Invoke-WebRequest 'https://codeload.github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0/zip/refs/heads/main' -OutFile (Join-Path $AssetRoot 'KayKitAdventurers.zip')
Invoke-WebRequest 'https://codeload.github.com/KayKit-Game-Assets/KayKit-Dungeon-Remastered-1.0/zip/refs/heads/main' -OutFile (Join-Path $AssetRoot 'KayKitDungeon.zip')
Get-FileHash (Join-Path $AssetRoot 'KayKitAdventurers.zip') -Algorithm SHA256
Get-FileHash (Join-Path $AssetRoot 'KayKitDungeon.zip') -Algorithm SHA256
# 按第 11 节核验哈希后，展开到尚未被其他任务占用的目录
Expand-Archive (Join-Path $AssetRoot 'KayKitAdventurers.zip') (Join-Path $AssetRoot 'KayKitAdventurers')
Expand-Archive (Join-Path $AssetRoot 'KayKitDungeon.zip') (Join-Path $AssetRoot 'KayKitDungeon')
python -m pip install numpy
python Tools/prepare_kaykit.py
```

UE 资源脚本使用完整 Editor 的 `-ExecutePythonScript` 模式；字体和 glTF 资产保存需要已初始化的 Slate，不能统一替换成 commandlet 模式：

```powershell
New-Item -ItemType Directory -Path (Join-Path $ProjectRoot 'Saved/Logs') -Force | Out-Null
foreach ($ScriptName in @('build_content.py','import_art.py','import_chinese_font.py','import_kaykit.py')) {
    $ScriptPath = Join-Path $ProjectRoot ('Tools/' + $ScriptName)
    $LogPath = Join-Path $ProjectRoot ('Saved/Logs/Import_' + $ScriptName + '.log')
    # 引号覆盖包含空格的本机路径；仅等待本次启动的进程。
    $EditorArgs = @(('"' + $ProjectFile + '"'), '-unattended', '-nosplash', '-nullrhi',
        ('-ExecutePythonScript="' + $ScriptPath + '"'), ('-abslog="' + $LogPath + '"'))
    $EditorProcess = Start-Process -FilePath "$EngineRoot/Engine/Binaries/Win64/UnrealEditor.exe" -ArgumentList $EditorArgs -WindowStyle Hidden -PassThru
    $EditorProcess.WaitForExit()
    if ($EditorProcess.ExitCode -ne 0) { throw ('Import process failed: ' + $ScriptName) }
    $ImportLog = Get-Content -LiteralPath $LogPath -Raw
    if ($ImportLog -match 'LogPython: Error:|Python script executed with errors|Fatal error:') {
        throw ('Inspect import log: ' + $LogPath)
    }
}
```

首次全新工程在空地图创建上若出现 NullRHI 与编辑器初始化兼容问题，可去掉 `-nullrhi` 改用 `-RenderOffscreen`，仍保留完整 Editor 模式并检查日志。每个脚本还应确认自己的成功标记：`FOLDLIGHT_CONTENT_SUCCESS`、`FOLDLIGHT_ART_SUCCESS`、`FOLDLIGHT_CHINESE_FONT_SUCCESS`、`KAYKIT_IMPORT_COMPLETE`。

**导入后复查 DefaultInput.ini：** `DefaultPlayerInputClass=/Script/Engine.PlayerInput`，`DefaultInputComponentClass=/Script/Engine.InputComponent`。编辑器曾自动改成 EnhancedInput 类，造成原有按键行为异常。

打包：

```powershell
& "$EngineRoot/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun "-project=$ProjectFile" -target=FoldlightDemo -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive "-archivedirectory=$ProjectRoot/Build" -utf8output -unattended
if ($LASTEXITCODE -ne 0) { throw 'Windows package failed' }
```

完成后运行 `Tools/Package_Windows.bat` 的许可证复制步骤。附录 BAT 的本机引擎路径需要按实际安装位置调整，不能把它们当跨机器自动检测脚本。

实际游戏：`Build/Windows/FoldlightDemo.exe` 为入口，内部运行二进制为 `Build/Windows/FoldlightDemo/Binaries/Win64/FoldlightDemo.exe`。`Launch_Playtest.bat` 优先启动打包入口；没有包才回退编辑器 `-game`。

```powershell
& ./Tools/Validate_Playtest.ps1
& ./Tools/Capture_Art.ps1 -OutputFolder Documentation/ExternalArt -LogPrefix RebuildCapture
```

若系统执行策略阻止本次本地脚本，使用符合本机政策的进程级方式运行，不要替用户永久放宽全机安全配置。引擎构建和缓存通常需要写入 AppData；权限不足时申请具体目录/进程权限，不把缓存错误误诊为 C++ 错误。

## 17. 验收矩阵

### 17.1 工程与资源

- Editor 与 Game Target 均编译成功。
- 打包日志包含成功结束，进程退出码正常。
- 中文字体在独立成品中能显示，不依赖系统安装。
- 15 个原创网格、12 个环境网格、Mage、7 个运行时动画、材质和音效随包存在。
- 原生动画实例已激活，动画和 SkeletalMesh 使用同一 Skeleton，骨骼姿势确实跨帧变化。
- 导入脚本不把“无 Python 异常前的部分日志”误当全部成功；检查每个成功标记和清单。

### 17.2 机制测试

- 四个方向分别核对相机 Forward=-N、D 向 R 移动、深度维持不变。
- 用非正方形平台验证 Width 使用正确尺寸，不能只测正方形。
- 空中、石柱移动中、投影后重叠或目标被占用时拒绝转换。
- 成功展开恢复脚下源平台深度；失败不改变位置和维度。
- 燃烧完成同时移除源/代理碰撞，重置恢复。
- 3D 低火印和 2D 高墙分别命中对应形状；不同房间不误伤。
- 双炉要求持续共同受热；高藤蔓需先解锁。
- 四向印记拒绝三维和错误方向；正确方向点亮并保持。
- 有效火焰跨维度后源锚点不变、寿命不增长。
- 资源和机制基线合计 101 项断言，最终日志需 `FOLDLIGHT_TEST COMPLETE failures=0`。测试模式允许为构造边界条件直接布置角色和临时障碍，不要把它与自然通关混称。

### 17.3 真实路线通关

`-FoldlightTraversal` 使用移动输入、CharacterMovement、跳跃、交互和施法完成五关。正常房间切换可按游戏规则传送；不能在关卡内部直接把玩家瞬移到终点来冒充通关。

结束标记应为 `FOLDLIGHT_TRAVERSAL COMPLETE`，包含五关完成和 falls=0。出现 FAIL、超时或缺少结束标记都视为失败，不只看进程退出码。

### 17.4 实机画面

固定 1280×720，截图必须来自本次成品运行，检查时间戳，不能复用旧文件。标准截图 13 张：Courtyard3D、Courtyard2D、Mechanism2D、Garden3D、Furnace3D、TowerSouth、TowerEast、TowerNorth、TowerWest、MageWalk、MageRun、MageJump、MageCast。

检查：背景不遮住最高出口或反向路线；人物脚底和尺度合理；法杖随手、帽子随头、披风随身体；动作不是 T Pose；魔法边界清楚；中文 UI 没有方框或截断；没有缺失网格、默认材质回退或致命错误。

`-nullrhi` 用于无渲染机制测试，不能证明画面正确。截图用 `-RenderOffscreen` 实际渲染。

### 17.5 手动体验

新增一轮真实键鼠检查：标题 Enter/N、三维移动与鼠标、四方向 Q/F、按住/松开 V、暂停与恢复、重置、三档提示、退出与继续。当前历史基线没有跨设备帧率基准，不得据此承诺所有机器稳定 60 FPS。

## 18. 常见失败及排查

| 现象 | 重点原因与处理 |
| --- | --- |
| 字体/glTF 导入时 Slate 断言 | 用完整 Editor `-ExecutePythonScript`，不要用未初始化 Slate 的 commandlet 保存这些资产 |
| 键盘轴输入异常 | 检查被自动改写的 DefaultInput 默认类与 AxisMappings |
| 北/西视左右相反 | 输入或投影宽度绕过 FDProjection，写死了世界 X |
| 转身后二维相机旋转 | SpringArm 没有使用正确绝对旋转设置，或 PawnControlRotation 未关闭 |
| 展开传到错误的平台 | 没有使用 CurrentFloor 的实际支撑源身份 |
| 藤蔓消失却还有墙 | 只隐藏视觉，Projection 或 Solid 未同步关闭 |
| 石柱移动时切换穿模 | 安全检查缺少 bSliding 锁定 |
| 多次折叠法术漂移/续时 | 覆盖了 FireSourceAnchor，或切换时重新 PlaceFire |
| 第四关可直接烧高藤蔓 | 忽略 bFurnaceSolved 前置条件 |
| 东/西视高层平台堵路 | 用高实心柱替代了第五关 60 cm 浮空石板 |
| Mage 转向差 90° | 导入坐标朝向与 Actor 前向不一致，应用网格相对 Yaw=-90 |
| 帽子/法杖留在原地 | 静态配件没有转为对应骨骼绑定，或新版 GLB 节点索引不同 |
| 程序动画图编译 Cast 类型错误 | 节点成员名 Cast 遮蔽 UE Cast 模板，使用 `::Cast<AFDCharacter>` |
| 材质在打包后变灰 | 缺少 skeletal/instanced usage 标记，或资产未烘焙 |
| 第三方柱子装饰被拉长 | 改用等比例模块拼接，检查重复柱位置避免重叠 |
| 旧火焰路径缺失 | 按第 11.7 节迁移或移除明确的可选引用 |
| 重建时只看到空地图 | 本工程运行时生成世界，先确认 GameMode 和 StartPlay |
| 旧角色再次出现 | 当前 GameMode/Pawn 指向旧蓝图，或 Mage 加载失败触发回退 |

## 19. 交付清单与证据

交付至少包含：完整 Source/Config/Content、`.uproject`、可重建资源脚本、官方资源来源和下载哈希、中文 README、启动 BAT、Windows 包、许可证目录、测试日志、13 张实机截图及已知限制。

历史参考日志位于原独立工作区：`Saved/Logs/KayKitPackage.log`、`KayKitSmokeFinal.log`、`KayKitTraversalFinal.log` 和 `KayKitCapture*.log`。它们只证明 2026-09-20 的基线，不证明 Trae 的新工程。

新交付应给出实际项目路径、实际可执行入口、编译/打包结果、断言总数与失败数、物理通关结果、截图目录以及本次仍未完成的项目。如果采用不带旧 Niagara 粒子的独立重建方式，在这里明确列出这项视觉差异。

## 20. 附录使用方式和源码范围

后续附录每个文件以 `<!-- FILE: 相对路径 -->` 标记开始，紧接一个完整代码块，以 `<!-- END FILE -->` 结束。将代码块内容写入新工程相应路径；不要把 Markdown 围栏和标记写入源文件。

文件清单中的 SHA-256 对应**代码块正文统一为 LF 换行、UTF-8 编码后的内容**，用于检查摘取是否完整。不是 Unreal uasset 的哈希，也不是原文件 CRLF 字节级哈希。

附录保留当前核心逻辑，未打包引擎、生成缓存、IDE 私有设置、原项目旧玩法类或第三方二进制资产。DefaultEngine.ini 已去除与这次 Windows 重建无关的 Android 文件服务器设置；不需要复制设备凭据。字体和两份 KayKit 许可证以原文本收录。

恢复文件后仍需按第 11.7 节决定旧 Niagara 资源处理、按本机修改引擎安装路径，并修正第 14 节那条旧“蜡烛”提示。读取附录不等于可以跳过资源导入、编译和新验收。

下面的清单和正文由当前工作区文件直接嵌入，不以省略号代替代码。


## 附录 A 可恢复文件清单

共 36 个文本文件。清单哈希用于验证附录正文的完整性。

| 文件 | 行数 | SHA-256（LF UTF-8） |
| --- | ---: | --- |
| `FoldlightDemo.uproject` | 12 | `4047258113b4d5e059ce943277a74d45af7b03dba6ba9fbbf8485ad7777d9fa9` |
| `Source/FoldlightDemo.Target.cs` | 11 | `e6661beb2e09facc25d3bf1650a74c25b0039e2e2b96dea65a62da7ed62cdfd3` |
| `Source/FoldlightDemoEditor.Target.cs` | 11 | `dda82474b9a05801aac74dee1181e38089a2b2aa4420d499b0337f8982f3c9a6` |
| `Source/NEWMyProject/NEWMyProject.Build.cs` | 23 | `48e8053fbdaad79ccd50f7680643892e8223148dc82dbaf22b2ff02386c17ef6` |
| `Source/NEWMyProject/NEWMyProject.h` | 5 | `30af2b68e6d8210e4ce4fbd26f6a2b8c3648cf2a1226d64b17c4f88c94fccdf1` |
| `Source/NEWMyProject/NEWMyProject.cpp` | 6 | `68457955a31ecd2c47d6dcf360b775efdceabd21f97f9d4eff91334db86422dd` |
| `Source/NEWMyProject/FoldlightDemo.h` | 225 | `e8134f9ae6d45929d364971d70b3dded43ebf5c9499938de55782ab484ab2902` |
| `Source/NEWMyProject/FDProjection.h` | 20 | `7f317fbf640a881ecb2a7afe4ee58fdf7341a1056d0beca352a2072700f7fd19` |
| `Source/NEWMyProject/FDVisuals.h` | 16 | `16e53144d518dbfbd3fcfb9179b4743347eb9d533e61a26461319a1cc421545f` |
| `Source/NEWMyProject/FDCharacter.cpp` | 349 | `318ed5584c19ffe670edfd0d2381816656c4d1e27cd2eb7f6440600778f2ed90` |
| `Source/NEWMyProject/FDObject.cpp` | 259 | `f22ff8864ef0a1ee71dec87e8861f583d429909e3718517712778885378f92b8` |
| `Source/NEWMyProject/FDWorld.cpp` | 608 | `27cf325224d7987aa584745506ef643d2e2ff170d4831bd51584d884731a6e0f` |
| `Source/NEWMyProject/FDArt.cpp` | 195 | `16494e415020efc6d4dd99d172ecd0f6c0691a2e214b335081b01012891c93d9` |
| `Source/NEWMyProject/FDHUD.cpp` | 131 | `6d9259e4b788d565a77670aa20677c09f0148106cbadaaa4f0ea0013ef5ea7a0` |
| `Source/NEWMyProject/FDMageAnimInstance.h` | 17 | `ab7d48ecf16458f7765b5ff8a8427765068e1b881598fcf4025e266354c8262d` |
| `Source/NEWMyProject/FDMageAnimInstance.cpp` | 84 | `44cea5634eee7e9025bc5ce206e7d2bb478b4bf119a1e63a49185cacaa787434` |
| `Source/NEWMyProject/FDValidation.cpp` | 321 | `95b4824eebaea9c4ec92afde97929d15b9240572d7b83d720fcf42d6fa71d489` |
| `Source/NEWMyProject/FDFourViewTests.cpp` | 102 | `cca464039d4cf27dc5d079758a2e8886c8f0930f7020c73144778a70ebd98171` |
| `Config/DefaultEngine.ini` | 28 | `2c286423cbf09e184a59c856e840741cfcc1372edb80baba6d66c9e93a136453` |
| `Config/DefaultGame.ini` | 19 | `7e25cf931cc75cf6afabb2c2e16c5f336d47413804b86ed1466c60d89f7f9a31` |
| `Config/DefaultInput.ini` | 91 | `cd0952ddbb02b152872884b5a8a6c3b932599698b39c1a0396c21152fc8bccb4` |
| `Tools/make_audio.py` | 23 | `9c2cedd028b577eb95423ccd763492224a413b7f5d393bdb6d7202ccc4b716df` |
| `Tools/make_models.py` | 151 | `17ac86a5c27bf6ef587bbd341c8cb396db82804490dbced9404675507333afec` |
| `Tools/build_content.py` | 65 | `a514270380298c03886cb505e51b28053764b5bdfe9adeab428ca661b3a08aca` |
| `Tools/import_art.py` | 73 | `c1035c749aa11c227e88103f0ac588e9cb24ad324e4816debe4c1cb565f5b138` |
| `Tools/import_chinese_font.py` | 24 | `49aa8d3d118750b6ab9e2887fc31549b93d77bd6300e8fc1dfa9b53da2558006` |
| `Tools/prepare_kaykit.py` | 83 | `23891e50a973d7e903413d02766b65cc1da4bf2036d6711f3021285a653521a1` |
| `Tools/import_kaykit.py` | 78 | `34241cd968d35f3872ffd289337c04d45fc56be9fb57a56f0b08c74a9c9a9d0b` |
| `Tools/Build_Editor.bat` | 3 | `d68d4e649450a23d33ee8c2955dd60ba9a2533a749f89349677fdd54d68bdae9` |
| `Tools/Package_Windows.bat` | 8 | `dad68dee03e30d598d66b518ab0f5a1d0d5912fe4006ded03780f2d3ff28a033` |
| `Tools/Validate_Playtest.ps1` | 19 | `610b43466490b0ad2e21e84c377ee4fafb35782d7a0f7c4568624e051342a3ec` |
| `Tools/Capture_Art.ps1` | 44 | `273d0e5fdb25304a568aeb90fff056d96ee4cbaa1f5399755910f56892f9298b` |
| `Launch_Playtest.bat` | 8 | `f26e7da4e192350eac93c4ac4b3a826fb0fee0c7a73eb3129e413629c8cd75ea` |
| `Documentation/Fonts/AndroidOpenSourceProject_License.txt` | 211 | `4cb18094865c52c38c801d3e33c6a063dbf641b1747f6be24f891511eab78d15` |
| `Tools/ExternalAssets/Prepared/KayKitAdventurers-CC0.txt` | 33 | `50e3fb7858f1351436923f0fd4bb4ea9198db1248bb2325b46a66af94aac517e` |
| `Tools/ExternalAssets/Prepared/KayKitDungeon-CC0.txt` | 33 | `648a302fcc46e7207f67a8e64dea9a9690e6f35ce6ee36ab7ce4c77855bc59e0` |

## 附录 B 完整文件内容

### 文件 FoldlightDemo.uproject

<!-- FILE: FoldlightDemo.uproject -->
```json
{
  "FileVersion": 3,
  "EngineAssociation": "5.7",
  "Category": "Games",
  "Description": "Foldlight - a five-room dimensional puzzle playtest",
  "Modules": [{"Name": "NEWMyProject", "Type": "Runtime", "LoadingPhase": "Default"}],
  "Plugins": [
    {"Name": "PythonScriptPlugin", "Enabled": true, "TargetAllowList": ["Editor"]},
    {"Name": "EditorScriptingUtilities", "Enabled": true, "TargetAllowList": ["Editor"]},
    {"Name": "Niagara", "Enabled": true}
  ]
}
```
<!-- END FILE -->

### 文件 Source/FoldlightDemo.Target.cs

<!-- FILE: Source/FoldlightDemo.Target.cs -->
```csharp
using UnrealBuildTool;
public class FoldlightDemoTarget : TargetRules
{
    public FoldlightDemoTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("NEWMyProject");
    }
}
```
<!-- END FILE -->

### 文件 Source/FoldlightDemoEditor.Target.cs

<!-- FILE: Source/FoldlightDemoEditor.Target.cs -->
```csharp
using UnrealBuildTool;
public class FoldlightDemoEditorTarget : TargetRules
{
    public FoldlightDemoEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("NEWMyProject");
    }
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/NEWMyProject.Build.cs

<!-- FILE: Source/NEWMyProject/NEWMyProject.Build.cs -->
```csharp
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NEWMyProject : ModuleRules
{
	public NEWMyProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara" });

		PrivateDependencyModuleNames.AddRange(new string[] { "SlateCore", "AnimGraphRuntime" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/NEWMyProject.h

<!-- FILE: Source/NEWMyProject/NEWMyProject.h -->
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/NEWMyProject.cpp

<!-- FILE: Source/NEWMyProject/NEWMyProject.cpp -->
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "NEWMyProject.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, NEWMyProject, "NEWMyProject" );
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FoldlightDemo.h

<!-- FILE: Source/NEWMyProject/FoldlightDemo.h -->
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "FoldlightDemo.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UCameraComponent;
class USpringArmComponent;
class UPointLightComponent;
class UNiagaraComponent;
class AFDWorld;

UENUM()
enum class EFDKind : uint8 { Platform, Pillar, Vine, Receiver, Exit, Decoration };

UCLASS()
class NEWMYPROJECT_API UFDSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 Room = 0;
    UPROPERTY() bool Finished = false;
    UPROPERTY() int32 Version = 1;
};

UCLASS()
class NEWMYPROJECT_API AFDObject : public AActor
{
    GENERATED_BODY()
public:
    AFDObject();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Solid;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Projection;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> VisualRoot;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Pieces;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> OriginalMaterials;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Surface;
    UPROPERTY() TObjectPtr<AFDWorld> Director;
    EFDKind Kind = EFDKind::Decoration;
    int32 Room = 0;
    FString ObjectId;
    FVector Size = FVector(100);
    FVector InitialLocation = FVector::ZeroVector;
    FVector SlideTarget = FVector::ZeroVector;
    bool bMoved = false;
    bool bSliding = false;
    bool bBurning = false;
    bool bBurned = false;
    bool bLit = false;
    bool bProjected = false;
    int32 RequiredView = INDEX_NONE;
    bool bActivated = false;
    float BurnTime = 0;
    float LitRemaining = 0;
    void Configure(AFDWorld* World, EFDKind Type, int32 RoomId, const FString& Id, FVector Location, FVector Dimensions, FLinearColor Color);
    void SetProjection(bool Enabled, float PlaneDepth, int32 View = 0);
    void Ignite();
    void ResetObject();
    void Slide();
    bool IsWalkable() const;
    bool Blocks() const;
    FVector GetExtent() const { return Size * .5f; }
    float Top() const { return GetActorLocation().Z + Size.Z * .5f; }
};

UCLASS()
class NEWMYPROJECT_API AFDCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AFDCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Boom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> WaxBody;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Flame;
    UPROPERTY() TObjectPtr<UPointLightComponent> CandleLight;
    UPROPERTY() TObjectPtr<AFDWorld> Director;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BodyPieces;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> WalkingFeet;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> CastingHands;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BlinkingEyes;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Cape;
    float VisualTime = 0;
    float CastPose = 0;
    int32 CastSerial = 0;
    bool bSkeletalHero = false;
    float LandingPulse = 0;
    bool bVisualAirborne = false;
    bool b2D = false;
    bool bPreview = false;
    bool bTransition = false;
    float PlaneDepth = 0;
    int32 FoldView = 0;
    float TransitionTime = 0;
    float JumpBuffer = 0;
    float CoyoteTime = 0;
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void JumpPressed();
    void PreviewStart();
    void PreviewCommit();
    void PreviousView();
    void NextView();
    bool SelectView(int32 View);
    void UpdateViewCamera();
    bool TrySwitch();
    bool CanSwitch(FVector& Destination, FString& Reason) const;
    void ApplyDimension(bool Is2D, FVector Destination);
    void CastFire();
    void Interact();
    void ResetRoom();
    void Hint();
    void PauseToggle();
    void Confirm();
    void NewGame();
    void Quit();
    bool InputAllowed() const;
};

UCLASS()
class NEWMYPROJECT_API AFDWorld : public AActor
{
    GENERATED_BODY()
public:
    AFDWorld();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY() TArray<TObjectPtr<AFDObject>> Objects;
    UPROPERTY() TObjectPtr<AFDCharacter> Player;
    UPROPERTY() TObjectPtr<USceneComponent> FireRoot;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> FirePieces;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> FireSigils;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> FireGlyphs;
    UPROPERTY() TObjectPtr<UNiagaraComponent> FireFX;
    UPROPERTY() TObjectPtr<UPointLightComponent> FireLight;
    UPROPERTY() TObjectPtr<USceneComponent> PreviewRoot;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> PreviewPieces;
    UPROPERTY() TObjectPtr<UFDSave> Save;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> SceneryRoots;
    int32 CurrentRoom = 0;
    int32 HintLevel = 0;
    bool bStarted = false;
    bool bPaused = false;
    bool bFinished = false;
    bool bInitialized = false;
    bool bUsed2D = false;
    bool bFurnaceSolved = false;
    float FurnaceHold = 0;
    float FireRemaining = 0;
    float CastCooldown = 0;
    float ToastRemaining = 0;
    float SessionTime = 0;
    int32 Falls = 0;
    int32 Switches = 0;
    FString Toast;
    FVector FireAnchor = FVector::ZeroVector;
    FVector FireSourceAnchor = FVector::ZeroVector;
    bool bSmoke = false;
    bool bCapture = false;
    bool bFourTest = false;
    int32 SmokeStep = 0;
    int32 SmokeFailures = 0;
    float SmokeClock = 0;
    void BuildWorld();
    AFDObject* Add(EFDKind Type, int32 Room, const FString& Id, FVector Local, FVector Size, FLinearColor Color);
    AFDObject* Find(const FString& Id) const;
    static FVector RoomOrigin(int32 Index) { return FVector(Index * 5000.0f, 0, 0); }
    void Start(bool Fresh);
    void EnterRoom(int32 Index, bool SaveProgress);
    void ResetCurrentRoom(bool FromFall = false);
    void Project(bool Is2D, float PlaneDepth);
    bool CheckSwitch(const AFDCharacter* Character, FVector& Destination, FString& Reason) const;
    bool IsSafe(FVector Location, const AFDCharacter* Character, bool Target2D, const AFDObject* Support = nullptr) const;
    void PlaceFire();
    FVector GetCastAnchor() const;
    void RefreshFireVisual();
    void AnimateFireVisual();
    void UpdateFire(float DeltaSeconds);
    void UseNearby();
    bool RoomSolved() const;
    void Notify(const FString& Message, float Duration = 3);
    void RequestHint();
    void Persist();
    FString RoomTitle() const;
    FString Objective() const;
    FString ContextAction() const;
    void RunSmoke(float DeltaSeconds);
    void RunTraversal(float DeltaSeconds);
    void RunFourViewTests(float DeltaSeconds);
    void UpdateScenery();
    void Check(bool Condition, const FString& Label);
};

UCLASS()
class NEWMYPROJECT_API AFDHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    UPROPERTY(Transient) TObjectPtr<class UFont> ChineseFont;
    void Text(const FString& Value, float X, float Y, float Size = 1, FLinearColor Color = FLinearColor::White, bool Centered = false);
    void Panel(float X, float Y, float W, float H, float Alpha = .8f);
};

UCLASS()
class NEWMYPROJECT_API AFDGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AFDGameMode();
    virtual void StartPlay() override;
};
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDProjection.h

<!-- FILE: Source/NEWMyProject/FDProjection.h -->
```cpp
#pragma once
#include "CoreMinimal.h"

// Cardinal camera positions; screen-right and collapsed depth share this basis.
namespace FDProjection
{
    inline int32 Wrap(int32 View) { return (View%4+4)%4; }
    inline FVector Depth(int32 View)
    {
        static const FVector Axes[]={FVector(0,1,0),FVector(1,0,0),FVector(0,-1,0),FVector(-1,0,0)};
        return Axes[Wrap(View)];
    }
    inline FVector Right(int32 View) { const FVector N=Depth(View); return FVector(N.Y,-N.X,0); }
    inline float Yaw(int32 View) { return (-Depth(View)).Rotation().Yaw; }
    inline float Width(FVector Size,int32 View) { return FVector::DotProduct(Size,Right(View).GetAbs()); }
    inline FVector OnPlane(FVector Point,int32 View,float DepthValue)
    { const FVector N=Depth(View); return Point+N*(DepthValue-FVector::DotProduct(Point,N)); }
    inline const TCHAR* Name(int32 View)
    { static const TCHAR* Names[]={TEXT("南视"),TEXT("东视"),TEXT("北视"),TEXT("西视")}; return Names[Wrap(View)]; }
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDVisuals.h

<!-- FILE: Source/NEWMyProject/FDVisuals.h -->
```cpp
#pragma once
#include "CoreMinimal.h"
class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
namespace FDVisuals
{
    UMaterialInstanceDynamic* Material(UObject* Owner, FLinearColor Color, float Glow = 0);
    void Sound(AActor* Owner, const TCHAR* Name, float Volume = .5f);
    void BuildScenery(AActor* Owner, USceneComponent* Parent, int32 Room);
    UStaticMeshComponent* Piece(AActor* Owner, USceneComponent* Parent, const TCHAR* Shape, FVector Position, FVector Size, FLinearColor Color, float Glow = 0);
    const FLinearColor Stone(.24f, .32f, .33f);
    const FLinearColor Edge(.48f, .67f, .61f);
    const FLinearColor Gold(.95f, .54f, .16f);
    const FLinearColor Ink(.016f, .028f, .038f);
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDCharacter.cpp

<!-- FILE: Source/NEWMyProject/FDCharacter.cpp -->
```cpp
#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "FDVisuals.h"
#include "FDMageAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AFDCharacter::AFDCharacter()
{
    PrimaryActorTick.bCanEverTick=true;
    GetCapsuleComponent()->InitCapsuleSize(30,86);
    bUseControllerRotationYaw=false;
    GetCharacterMovement()->bOrientRotationToMovement=true;
    GetCharacterMovement()->RotationRate=FRotator(0,800,0);
    GetCharacterMovement()->MaxWalkSpeed=420;
    GetCharacterMovement()->JumpZVelocity=540;
    GetCharacterMovement()->GravityScale=1.5f;
    GetCharacterMovement()->AirControl=.75f;
    GetCharacterMovement()->BrakingDecelerationWalking=2000;
    GetCharacterMovement()->MaxStepHeight=32;
    Boom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    Boom->SetupAttachment(RootComponent);
    Boom->TargetArmLength=720;
    Boom->bUsePawnControlRotation=true;
    Boom->bDoCollisionTest=true;
    Boom->TargetOffset=FVector(0,0,45);
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Boom,USpringArmComponent::SocketName);
    Camera->FieldOfView=65;
    Camera->OrthoWidth=1400;
    Camera->PostProcessSettings.bOverride_AutoExposureMethod=true;
    Camera->PostProcessSettings.AutoExposureMethod=EAutoExposureMethod::AEM_Manual;
    Camera->PostProcessSettings.bOverride_AutoExposureBias=true;
    Camera->PostProcessSettings.AutoExposureBias=9.5f;
    Camera->PostProcessSettings.bOverride_MotionBlurAmount=true;
    Camera->PostProcessSettings.MotionBlurAmount=0;
    WaxBody=CreateDefaultSubobject<USceneComponent>(TEXT("WaxBody"));
    WaxBody->SetupAttachment(RootComponent);
    WaxBody->SetRelativeLocation(FVector(0,0,-86));
    CandleLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("CandleLight"));
    CandleLight->SetupAttachment(WaxBody);
    CandleLight->SetRelativeLocation(FVector(0,0,137));
    CandleLight->SetLightColor(FLinearColor(1,.58f,.23f));
    CandleLight->SetIntensity(90);
    CandleLight->SetAttenuationRadius(230);
    CandleLight->SetCastShadows(false);
}

void AFDCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (!FParse::Param(FCommandLine::Get(),TEXT("CandleHero")))
    {
        if (USkeletalMesh* Mage=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Foldlight/External/KayKit/Mage/Mage/SkeletalMeshes/Mage.Mage")))
        {
            bSkeletalHero=true;
            GetMesh()->SetSkeletalMeshAsset(Mage);
            GetMesh()->SetRelativeLocation(FVector(0,0,-86));
            GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
            GetMesh()->SetRelativeScale3D(FVector(.65f));
            GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            GetMesh()->SetAnimInstanceClass(UFDMageAnimInstance::StaticClass());
            CandleLight->SetRelativeLocation(FVector(15,0,120));
            GetCharacterMovement()->SetComponentTickEnabled(false);
            return;
        }
    }
    const FLinearColor Wax(.91f,.80f,.57f), Dark(.035f,.095f,.12f);
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("SM_Wax"),FVector(0,0,79),FVector(50,50,82),Wax));
    Cape=FDVisuals::Piece(this,WaxBody,TEXT("SM_Cape"),FVector(-3,0,42),FVector(76,74,67),Dark);
    BodyPieces.Add(Cape);
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("SM_Ring"),FVector(0,0,70),FVector(41,41,5),FDVisuals::Gold));
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("SM_Crystal"),FVector(23,0,64),FVector(8,12,17),FDVisuals::Gold,.5f));
    // Melted rim and individual wax runs give the candle a recognizable silhouette.
    for (int I=0;I<8;++I)
    {
        const float A=I*PI*.25f, Length=10+(I*13)%22;
        BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(FMath::Cos(A)*23,FMath::Sin(A)*23,115-Length*.4f),FVector(7,7,Length),Wax));
    }
    for (float Y : {-11.f,11.f})
    {
        UStaticMeshComponent* Eye=FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(23,Y,95),FVector(6,7,12),Dark);
        BodyPieces.Add(Eye); BlinkingEyes.Add(Eye);
    }
    for (float Y : {-23.f,23.f})
    {
        UStaticMeshComponent* Foot=FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(8,Y,12),FVector(32,22,23),Dark);
        UStaticMeshComponent* Hand=FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(0,Y*1.4f,59),FVector(18,18,25),Wax);
        BodyPieces.Add(Foot); WalkingFeet.Add(Foot);
        BodyPieces.Add(Hand); CastingHands.Add(Hand);
    }
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("Cylinder"),FVector(0,0,124),FVector(6,6,16),Dark));
    Flame=FDVisuals::Piece(this,WaxBody,TEXT("SM_Flame"),FVector(0,0,142),FVector(21,21,40),FDVisuals::Gold,5);
    BodyPieces.Add(Flame);
    GetCharacterMovement()->SetComponentTickEnabled(false);
}

void AFDCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis(TEXT("FD_Forward"),this,&AFDCharacter::MoveForward);
    Input->BindAxis(TEXT("FD_Right"),this,&AFDCharacter::MoveRight);
    Input->BindAxis(TEXT("FD_Turn"),this,&AFDCharacter::Turn);
    Input->BindAxis(TEXT("FD_Look"),this,&AFDCharacter::LookUp);
    Input->BindKey(EKeys::SpaceBar,IE_Pressed,this,&AFDCharacter::JumpPressed);
    Input->BindKey(EKeys::SpaceBar,IE_Released,this,&ACharacter::StopJumping);
    Input->BindKey(EKeys::V,IE_Pressed,this,&AFDCharacter::PreviewStart);
    Input->BindKey(EKeys::V,IE_Released,this,&AFDCharacter::PreviewCommit);
    Input->BindKey(EKeys::Q,IE_Pressed,this,&AFDCharacter::PreviousView);
    Input->BindKey(EKeys::F,IE_Pressed,this,&AFDCharacter::NextView);
    Input->BindKey(EKeys::G,IE_Pressed,this,&AFDCharacter::CastFire);
    Input->BindKey(EKeys::E,IE_Pressed,this,&AFDCharacter::Interact);
    Input->BindKey(EKeys::R,IE_Pressed,this,&AFDCharacter::ResetRoom);
    Input->BindKey(EKeys::H,IE_Pressed,this,&AFDCharacter::Hint);
    Input->BindKey(EKeys::Escape,IE_Pressed,this,&AFDCharacter::PauseToggle);
    Input->BindKey(EKeys::Enter,IE_Pressed,this,&AFDCharacter::Confirm);
    Input->BindKey(EKeys::N,IE_Pressed,this,&AFDCharacter::NewGame);
    Input->BindKey(EKeys::F10,IE_Pressed,this,&AFDCharacter::Quit);
}

bool AFDCharacter::InputAllowed() const { return Director && Director->bStarted && !Director->bPaused && !Director->bFinished && !bTransition; }
void AFDCharacter::MoveForward(float Value)
{
    if (!InputAllowed() || b2D || bPreview || !Controller) return;
    AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),Value);
}
void AFDCharacter::MoveRight(float Value)
{
    if (!InputAllowed() || bPreview || !Controller) return;
    FVector Right=b2D ? FDProjection::Right(FoldView) : FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y);
    AddMovementInput(Right,Value);
}
void AFDCharacter::Turn(float Value) { if (InputAllowed() && !b2D && !bPreview) AddControllerYawInput(Value); }
void AFDCharacter::LookUp(float Value) { if (InputAllowed() && !b2D && !bPreview) AddControllerPitchInput(Value); }
void AFDCharacter::JumpPressed() { if (InputAllowed() && !bPreview) JumpBuffer=.14f; }
void AFDCharacter::PreviewStart()
{
    if (!InputAllowed()) return;
    bPreview=true;
    GetCharacterMovement()->StopMovementImmediately();
    if (!b2D)
    {
        Boom->bUsePawnControlRotation=false;
        Boom->bDoCollisionTest=false;
        Boom->SetAbsolute(false,true,false);
        Boom->SetWorldRotation(FRotator(0,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        Camera->SetOrthoWidth(1400);
    }
}
void AFDCharacter::PreviewCommit()
{
    if (!bPreview) return;
    bPreview=false;
    if (!b2D)
    {
        Boom->SetAbsolute(false,false,false);
        Boom->bUsePawnControlRotation=true;
        Boom->bDoCollisionTest=true;
        Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
    }
    TrySwitch();
}
bool AFDCharacter::CanSwitch(FVector& Destination, FString& Reason) const
{
    if (!Director) { Reason=TEXT("房间正在加载，请稍候。"); return false; }
    return Director->CheckSwitch(this,Destination,Reason);
}
void AFDCharacter::PreviousView() { SelectView(FoldView-1); }
void AFDCharacter::NextView() { SelectView(FoldView+1); }
bool AFDCharacter::SelectView(int32 View)
{
    if (!InputAllowed()) return false;
    if (b2D) { Director->Notify(TEXT("先按 V 展开为三维，再用 Q / F 选择新的折叠方向。")); return false; }
    FoldView=FDProjection::Wrap(View);
    UpdateViewCamera();
    Director->UpdateScenery();
    Director->Notify(FString::Printf(TEXT("已选择%s。按住 V 预览；预览中也可用 Q / F 换方向。"),FDProjection::Name(FoldView)));
    return true;
}
void AFDCharacter::UpdateViewCamera()
{
    if (b2D || bPreview)
    {
        Boom->bUsePawnControlRotation=false; Boom->bDoCollisionTest=false;
        Boom->SetAbsolute(false,true,false);
        Boom->SetWorldRotation(FRotator(0,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        Camera->SetOrthoWidth(1400);
    }
    else if (Controller) Controller->SetControlRotation(FRotator(-27,FDProjection::Yaw(FoldView),0));
}
bool AFDCharacter::TrySwitch()
{
    if (!InputAllowed()) return false;
    FVector Destination; FString Reason;
    if (!CanSwitch(Destination,Reason)) { Director->Notify(Reason); return false; }
    ApplyDimension(!b2D,Destination);
    FDVisuals::Sound(this,TEXT("S_Fold"),.4f);
    Director->Switches++;
    if (b2D) Director->bUsed2D=true;
    return true;
}
void AFDCharacter::ApplyDimension(bool Is2D, FVector Destination)
{
    b2D=Is2D; bPreview=false;
    GetCharacterMovement()->StopMovementImmediately();
    if (Is2D)
    {
        PlaneDepth=FVector::DotProduct(Destination,FDProjection::Depth(FoldView));
        Director->Project(true,PlaneDepth);
        GetCharacterMovement()->SetPlaneConstraintNormal(FDProjection::Depth(FoldView));
        GetCharacterMovement()->SetPlaneConstraintOrigin(Destination);
        GetCharacterMovement()->SetPlaneConstraintEnabled(true);
        GetCharacterMovement()->RotationRate=FRotator(0,10000,0);
        const FVector Right=FDProjection::Right(FoldView);
        const float Facing=FVector::DotProduct(GetActorForwardVector(),Right)<0 ? -1.f : 1.f;
        SetActorRotation((Right*Facing).Rotation());
        Boom->bUsePawnControlRotation=false;
        Boom->bDoCollisionTest=false;
        Boom->SetAbsolute(false,true,false);
        Boom->SetWorldRotation(FRotator(0,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        Camera->SetOrthoWidth(1400);
    }
    else
    {
        GetCharacterMovement()->SetPlaneConstraintEnabled(false);
        Director->Project(false,0);
        GetCharacterMovement()->RotationRate=FRotator(0,800,0);
        Boom->SetAbsolute(false,false,false);
        Boom->bUsePawnControlRotation=true;
        Boom->bDoCollisionTest=true;
        if (Controller) Controller->SetControlRotation(FRotator(-27,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
    }
    SetActorLocation(Destination,false,nullptr,ETeleportType::TeleportPhysics);
    GetCharacterMovement()->SetMovementMode(MOVE_Falling);
    bTransition=true; TransitionTime=.3f;
    Director->RefreshFireVisual();
    Director->UpdateScenery();
}

void AFDCharacter::CastFire()
{
    if (InputAllowed() && !bPreview)
    {
        Director->PlaceFire();
    }
}
void AFDCharacter::Interact() { if (InputAllowed() && !bPreview) Director->UseNearby(); }
void AFDCharacter::ResetRoom() { if (InputAllowed()) Director->ResetCurrentRoom(); }
void AFDCharacter::Hint() { if (InputAllowed()) Director->RequestHint(); }
void AFDCharacter::PauseToggle()
{
    if (!Director || !Director->bStarted || Director->bFinished) return;
    Director->bPaused=!Director->bPaused;
    GetCharacterMovement()->SetComponentTickEnabled(!Director->bPaused);
    if (bPreview && !b2D)
    {
        Boom->SetAbsolute(false,false,false);
        Boom->bUsePawnControlRotation=true;
        Boom->bDoCollisionTest=true;
        Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
    }
    bPreview=false;
}
void AFDCharacter::Confirm()
{
    if (!Director) return;
    if (!Director->bStarted) Director->Start(false);
    else if (Director->bPaused) PauseToggle();
    else if (Director->bFinished) Director->Start(true);
}
void AFDCharacter::NewGame() { if (Director && (!Director->bStarted || Director->bFinished)) Director->Start(true); }
void AFDCharacter::Quit() { UKismetSystemLibrary::QuitGame(this,Cast<APlayerController>(Controller),EQuitPreference::Quit,false); }

void AFDCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Director) Director=Cast<AFDWorld>(UGameplayStatics::GetActorOfClass(this,AFDWorld::StaticClass()));
    if (bSkeletalHero) GetMesh()->bPauseAnims=Director && Director->bPaused;
    if (!Director || Director->bPaused) return;
    if (Controller && Cast<APlayerController>(Controller))
        Cast<APlayerController>(Controller)->PlayerCameraManager->ViewPitchMin=-65;
    if (Controller && Cast<APlayerController>(Controller))
        Cast<APlayerController>(Controller)->PlayerCameraManager->ViewPitchMax=10;
    if (bTransition)
    {
        TransitionTime-=DeltaSeconds;
        if (TransitionTime<=0) bTransition=false;
    }
    Boom->TargetArmLength=FMath::FInterpTo(Boom->TargetArmLength,(b2D || bPreview) ? 1600.f : 720.f,DeltaSeconds,12);
    if (InputAllowed())
    {
        JumpBuffer=FMath::Max(0.f,JumpBuffer-DeltaSeconds);
        CoyoteTime=GetCharacterMovement()->IsMovingOnGround() ? .1f : FMath::Max(0.f,CoyoteTime-DeltaSeconds);
        if (JumpBuffer>0 && (GetCharacterMovement()->IsMovingOnGround() || CoyoteTime>0))
        {
            LaunchCharacter(FVector(0,0,540),false,true);
            JumpBuffer=CoyoteTime=0;
        }
    }
    const float Speed=GetVelocity().Size2D();
    VisualTime+=DeltaSeconds;
    const float Time=VisualTime, Walk=FMath::Clamp(Speed/420.f,0.f,1.f);
    const bool Airborne=GetCharacterMovement()->IsFalling();
    if (bVisualAirborne && !Airborne) LandingPulse=1;
    bVisualAirborne=Airborne;
    LandingPulse=FMath::Max(0.f,LandingPulse-DeltaSeconds*5);
    CastPose=FMath::Max(0.f,CastPose-DeltaSeconds);
    if (bSkeletalHero)
    {
        CandleLight->SetIntensity(65+CastPose*65);
        return;
    }
    const float Cast=FMath::Sin(FMath::Clamp(CastPose/.55f,0.f,1.f)*PI);
    WaxBody->SetRelativeLocation(FVector(0,0,-86+FMath::Sin(Time*12)*Walk*2+FMath::Sin(Time*2)*.45f));
    WaxBody->SetRelativeRotation(FRotator(-Walk*3-Cast*4,0,FMath::Sin(Time*6)*Walk*2));
    WaxBody->SetRelativeScale3D(FVector(1+LandingPulse*.045f,1+LandingPulse*.045f,1-LandingPulse*.07f));
    for (int I=0;I<WalkingFeet.Num();++I)
    {
        const float Side=I ? 1.f : -1.f, Stride=FMath::Sin(Time*12+I*PI);
        WalkingFeet[I]->SetRelativeLocation(FVector(8+Stride*Walk*12,Side*23,12+FMath::Max(0.f,Stride)*Walk*8+(Airborne ? 5.f : 0.f)));
        CastingHands[I]->SetRelativeLocation(FVector(-Stride*Walk*7+Cast*28,Side*(32-Cast*9),59+Cast*18));
    }
    const float BlinkPhase=FMath::Fmod(Time,4.3f);
    const float EyeOpen=BlinkPhase<.16f ? FMath::Clamp(FMath::Abs(BlinkPhase-.08f)/.08f,.08f,1.f) : 1.f;
    for (UStaticMeshComponent* Eye : BlinkingEyes) Eye->SetRelativeScale3D(FVector(.06f,.07f,.12f*EyeOpen));
    if (Cape) Cape->SetRelativeRotation(FRotator(Walk*5+FMath::Sin(Time*3)*1.5f,0,FMath::Sin(Time*5)*Walk*3));
    if (Flame)
    {
        Flame->SetRelativeScale3D(FVector(.21f,.21f,.4f+FMath::Sin(Time*7)*.035f+Cast*.08f));
        Flame->SetRelativeRotation(FRotator(FMath::Sin(Time*4)*4-Walk*9,0,FMath::Sin(Time*3)*3));
    }
    CandleLight->SetIntensity(100+FMath::Sin(Time*8)*8+Cast*35);
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDObject.cpp

<!-- FILE: Source/NEWMyProject/FDObject.cpp -->
```cpp
#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "FDVisuals.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void FDVisuals::Sound(AActor* Owner,const TCHAR* Name,float Volume)
{
    USoundBase* Audio=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Foldlight/Audio/%s.%s"),Name,Name));
    if (Audio) UGameplayStatics::PlaySound2D(Owner,Audio,Volume);
}

UMaterialInstanceDynamic* FDVisuals::Material(UObject* Owner, FLinearColor Color, float Glow)
{
    UMaterialInterface* Parent = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Foldlight/Art/M_Surface.M_Surface"));
    if (!Parent) Parent = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInstanceDynamic* Result = UMaterialInstanceDynamic::Create(Parent, Owner);
    if (Result)
    {
        Result->SetVectorParameterValue(TEXT("Tint"), Color);
        Result->SetVectorParameterValue(TEXT("Color"), Color);
        Result->SetVectorParameterValue(TEXT("Glow"), Color * (Glow + .32f));
        const bool Brass=Color.R>Color.B*2 && Color.G>Color.B*1.5f;
        Result->SetScalarParameterValue(TEXT("Metallic"),Brass ? .55f : 0.f);
        Result->SetScalarParameterValue(TEXT("Roughness"),Brass ? .36f : .82f);
    }
    return Result;
}

UStaticMeshComponent* FDVisuals::Piece(AActor* Owner, USceneComponent* Parent, const TCHAR* Shape, FVector Position, FVector Size, FLinearColor Color, float Glow)
{
    UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Owner);
    Owner->AddInstanceComponent(C);
    C->SetupAttachment(Parent);
    C->SetMobility(EComponentMobility::Movable);
    C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    const bool Authored=FString(Shape).StartsWith(TEXT("SM_"));
    const FString MeshPath=Authored ? FString::Printf(TEXT("/Game/Foldlight/Art/Models/%s.%s"),Shape,Shape) : FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape);
    C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*MeshPath));
    UMaterialInstanceDynamic* Mat=Material(Owner, Color, Glow);
    if (FString(Shape)==TEXT("SM_Stone") || FString(Shape)==TEXT("SM_Arch") || FString(Shape)==TEXT("SM_Pedestal")) Mat->SetScalarParameterValue(TEXT("Grain"),1.f);
    if (FString(Shape)==TEXT("SM_Root") || FString(Shape)==TEXT("SM_Leaf") || FString(Shape)==TEXT("SM_Grass"))
    { Mat->SetScalarParameterValue(TEXT("Metallic"),0); Mat->SetScalarParameterValue(TEXT("Roughness"),.95f); }
    C->SetMaterial(0, Mat);
    C->SetRelativeLocation(Position);
    C->SetRelativeScale3D(Size / 100.0f);
    C->RegisterComponent();
    return C;
}

AFDObject::AFDObject()
{
    PrimaryActorTick.bCanEverTick = true;
    Solid = CreateDefaultSubobject<UBoxComponent>(TEXT("SourceCollision"));
    SetRootComponent(Solid);
    Solid->SetCollisionProfileName(TEXT("BlockAll"));
    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Visual"));
    VisualRoot->SetupAttachment(Solid);
    Projection = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectedCollision"));
    Projection->SetupAttachment(Solid);
    Projection->SetCollisionProfileName(TEXT("BlockAll"));
    Projection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Projection->SetHiddenInGame(true);
}

bool AFDObject::IsWalkable() const { return Kind == EFDKind::Platform || Kind == EFDKind::Pillar; }
bool AFDObject::Blocks() const { return IsWalkable() || (Kind == EFDKind::Vine && !bBurned); }

void AFDObject::Configure(AFDWorld* World, EFDKind Type, int32 RoomId, const FString& Id, FVector Location, FVector Dimensions, FLinearColor Color)
{
    Director = World; Kind = Type; Room = RoomId; ObjectId = Id; Size = Dimensions;
    InitialLocation = Location;
    SetActorLocation(Location);
    Solid->SetBoxExtent(Size * .5f);
    Solid->SetCollisionEnabled(Blocks() ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    const auto Piece = [&](const TCHAR* Shape, FVector Pos, FVector S, FLinearColor C, float Glow = 0.f)
    {
        UStaticMeshComponent* M = FDVisuals::Piece(this, VisualRoot, Shape, Pos, S, C, Glow);
        Pieces.Add(M); OriginalMaterials.Add(Cast<UMaterialInstanceDynamic>(M->GetMaterial(0))); return M;
    };
    if (IsWalkable())
    {
        UStaticMeshComponent* Main = Piece(TEXT("SM_Stone"), FVector::ZeroVector, Size, Color);
        Surface = Cast<UMaterialInstanceDynamic>(Main->GetMaterial(0));
        const float Top=Size.Z*.5f;
        const FLinearColor Decks[]={FLinearColor(.36f,.49f,.45f),FLinearColor(.36f,.42f,.46f),FLinearColor(.31f,.44f,.34f),FLinearColor(.43f,.35f,.28f),FLinearColor(.34f,.40f,.49f)};
        Piece(TEXT("SM_Stone"),FVector(0,0,Top-9),FVector(Size.X,Size.Y,18),Decks[FMath::Clamp(Room,0,4)]);
        Piece(TEXT("SM_Stone"),FVector(0,0,-Top+10),FVector(Size.X*.96f,Size.Y*.96f,20),Color*.7f);
        // Four matching fascias keep the playable lip legible from every view.
        for (int Side=0;Side<4;++Side)
        {
            const FVector N=FDProjection::Depth(Side), R=FDProjection::Right(Side);
            const float Span=FDProjection::Width(Size,Side), Depth=FVector::DotProduct(Size,N.GetAbs());
            UStaticMeshComponent* Band=Piece(TEXT("Cube"),N*(Depth*.5f+.4f)+FVector(0,0,Top-23),FVector(Span*.92f,2,3),FDVisuals::Gold,.35f);
            Band->SetRelativeRotation(R.Rotation());
            const int Count=FMath::Clamp(FMath::RoundToInt(Span/160),2,12);
            for (int J=0;J<Count;++J)
            {
                const FVector P=N*(Depth*.5f+.5f)+R*((J+.5f)*Span/Count-Span*.5f)+FVector(0,0,Top-40);
                Piece(TEXT("SM_Stone"),P,FVector(18,18,17),Color*.7f)->SetRelativeRotation(FRotator(0,45,0));
            }
        }
        // Shallow dark joints subdivide wide platforms without changing collision.
        const int Tiles=FMath::Clamp(FMath::CeilToInt(Size.X/220),2,14);
        for (int J=1;J<Tiles;++J)
            Piece(TEXT("Cube"),FVector(-Size.X*.5f+J*Size.X/Tiles,0,Top+.2f),FVector(2,Size.Y*.90f,.4f),Color*.42f);
        for (float Y : {-.3f,0.f,.3f})
            Piece(TEXT("Cube"),FVector(0,Size.Y*Y,Top+.25f),FVector(Size.X*.92f,2,.5f),Color*.42f);
        Piece(TEXT("SM_Ring"),FVector(0,0,Top+.7f),FVector(90,90,1),FDVisuals::Gold,.12f);
        FRandomStream DetailSeed(FCrc::StrCrc32(*Id));
        // Low, collision-free weathering stays at the corners, clear of jumps.
        for (int I=0;I<4;++I)
        {
            const FVector P(Size.X*(I%2 ? .40f : -.40f),Size.Y*(I<2 ? .39f : -.39f),Top+.4f);
            if (Room!=3)
            {
                Piece(TEXT("SM_Grass"),P,FVector(48,48,Room==2 ? 25 : 13),FLinearColor(.13f,.24f,.14f));
                Piece(TEXT("SM_Leaf"),P+FVector(10,0,1),FVector(27,35,36),FLinearColor(.17f,.28f,.15f))->SetRelativeRotation(FRotator(70,DetailSeed.FRandRange(0,360),0));
            }
            for (int J=0;J<2;++J)
                Piece(TEXT("SM_Stone"),P+FVector(DetailSeed.FRandRange(-28,28),DetailSeed.FRandRange(-22,22),1),FVector(DetailSeed.FRandRange(10,24),12,3),Color*.55f)->SetRelativeRotation(FRotator(0,DetailSeed.FRandRange(0,180),0));
            Piece(TEXT("Cube"),P+FVector(-20,15,0),FVector(45,1.2f,.5f),Color*.35f)->SetRelativeRotation(FRotator(0,35+I*27,0));
        }
        if (Kind == EFDKind::Pillar)
        {
            Piece(TEXT("Cube"), FVector(0, Size.Y*.5f+4, 0), FVector(65, 8, 65), FDVisuals::Gold, .3f)->SetRelativeRotation(FRotator(0,0,45));
        }
    }
    else if (Kind == EFDKind::Vine)
    {
        for (int I=0; I<7; ++I)
        {
            float Y = (I-3) * Size.Y / 7;
            UStaticMeshComponent* Branch = Piece(TEXT("SM_Root"), FVector(FMath::Sin(I*2.f)*12,Y,0), FVector(105,105,Size.Z), FLinearColor(.24f,.10f,.055f));
            Branch->SetRelativeRotation(FRotator(0,0,(I%2 ? 13 : -13)));
            for (int K=0;K<3;++K)
                Piece(TEXT("SM_Crystal"),FVector(18,Y+12,Size.Z*(K*.23f-.25f)),FVector(28,45,25),FLinearColor(.39f,.20f,.08f),.15f)->SetRelativeRotation(FRotator(0,0,45+K*35));
        }
        Piece(TEXT("Cube"), FVector(0,0,-Size.Z*.3f), FVector(Size.X,Size.Y,18), FLinearColor(.19f,.075f,.04f));
        Piece(TEXT("Cube"), FVector(0,0,Size.Z*.2f), FVector(Size.X,Size.Y,18), FLinearColor(.19f,.075f,.04f));
    }
    else if (Kind == EFDKind::Receiver)
    {
        Piece(TEXT("SM_Pedestal"), FVector(0,0,-18), FVector(100,100,38), FDVisuals::Stone);
        Piece(TEXT("SM_Ring"),FVector(0,0,7),FVector(75,75,6),FDVisuals::Gold,.2f);
        for (int I=0;I<4;++I)
        {
            const float A=I*PI*.5f;
            Piece(TEXT("SM_Crystal"),FVector(FMath::Cos(A)*33,FMath::Sin(A)*33,20),FVector(14,14,48),FDVisuals::Gold,.1f);
        }
        UStaticMeshComponent* Core = Piece(TEXT("SM_Crystal"), FVector(0,0,30), FVector(43,43,65), FDVisuals::Gold, .15f);
        Surface = Cast<UMaterialInstanceDynamic>(Core->GetMaterial(0));
    }
    else if (Kind == EFDKind::Exit)
    {
        Piece(TEXT("SM_Pedestal"), FVector(0,0,-8), FVector(205,205,26), FDVisuals::Stone);
        for (int I=0;I<12;++I)
        {
            const float A = I * PI / 6;
            Piece(TEXT("Cube"), FVector(FMath::Cos(A)*88,FMath::Sin(A)*88,8), FVector(20,8,6), FDVisuals::Gold, 1.5f)->SetRelativeRotation(FRotator(0,FMath::RadiansToDegrees(A),0));
        }
        for (float X : {-104.f,104.f})
        {
            Piece(TEXT("SM_Pedestal"),FVector(X,0,75),FVector(42,42,150),FDVisuals::Stone);
            Piece(TEXT("SM_Crystal"),FVector(X,0,164),FVector(25,25,50),FDVisuals::Edge,1.2f);
        }
        Piece(TEXT("SM_Ring"),FVector(0,-10,165),FVector(280,280,13),FDVisuals::Gold,.25f)->SetRelativeRotation(FRotator(0,0,90));
        Piece(TEXT("SM_Ring"),FVector(0,-10,165),FVector(241,241,4),FDVisuals::Edge,1.1f)->SetRelativeRotation(FRotator(0,0,90));
        for (int I=0;I<12;++I)
        {
            const float A=I*PI/6;
            Piece(TEXT("SM_Stone"),FVector(FMath::Cos(A)*131,-20,165+FMath::Sin(A)*131),FVector(10,8,18),FDVisuals::Gold,.6f)->SetRelativeRotation(FRotator(0,0,FMath::RadiansToDegrees(A)-90));
        }
        UStaticMeshComponent* Core = Piece(TEXT("SM_Crystal"), FVector(0,-8,264), FVector(24,24,42), FDVisuals::Gold, 3);
        Surface = Cast<UMaterialInstanceDynamic>(Core->GetMaterial(0));
    }
}

void AFDObject::SetProjection(bool Enabled, float PlaneDepth, int32 View)
{
    bProjected = Enabled;
    Solid->SetCollisionResponseToChannel(ECC_Pawn, Enabled ? ECR_Ignore : ECR_Block);
    Solid->SetCollisionEnabled(Blocks() ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    Projection->SetBoxExtent(FDProjection::Right(View).GetAbs()*FDProjection::Width(Size,View)*.5f+FDProjection::Depth(View).GetAbs()*70+FVector(0,0,Size.Z*.5f));
    Projection->SetWorldLocation(FDProjection::OnPlane(GetActorLocation(),View,PlaneDepth));
    Projection->SetCollisionEnabled(Enabled && Blocks() ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void AFDObject::Ignite()
{
    if (Kind != EFDKind::Vine || bBurning || bBurned) return;
    bBurning = true; BurnTime = 1.5f;
    for (UStaticMeshComponent* M : Pieces) M->SetMaterial(0, FDVisuals::Material(this, FDVisuals::Gold, 2));
    if (Director) Director->Notify(TEXT("藤蔓正在燃烧，它的投影也会一同消失。"));
    if (UNiagaraSystem* FX = LoadObject<UNiagaraSystem>(nullptr,TEXT("/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Big_Simple.NS_Fire_Big_Simple")))
    {
        UNiagaraComponent* C = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FX, GetActorLocation()-FVector(0,0,Size.Z*.5f), FRotator::ZeroRotator, FVector(.7f), true);
        if (C) C->SetFloatParameter(TEXT("User.Lifetime"), 1.5f);
        // Spawned effects are owned separately and explicitly removed at burn completion below.
        if (C) C->AttachToComponent(VisualRoot, FAttachmentTransformRules::KeepWorldTransform);
    }
}

void AFDObject::Slide()
{
    if (Kind != EFDKind::Pillar || bSliding || bMoved) return;
    bSliding = true;
    if (Director) Director->Notify(TEXT("石柱正在沿轨道移动，留意侧面的平台轮廓。"));
}

void AFDObject::ResetObject()
{
    bMoved = bSliding = bBurning = bBurned = bLit = bActivated = false;
    BurnTime = LitRemaining = 0;
    SetActorLocation(InitialLocation);
    VisualRoot->SetVisibility(true,true);
    TArray<USceneComponent*> AttachedPieces;
    VisualRoot->GetChildrenComponents(true, AttachedPieces);
    for (USceneComponent* C : AttachedPieces) if (UNiagaraComponent* FX = Cast<UNiagaraComponent>(C)) FX->DestroyComponent();
    if (Kind == EFDKind::Vine)
        for (int32 I=0;I<Pieces.Num();++I) Pieces[I]->SetMaterial(0,OriginalMaterials[I]);
    SetProjection(false,0);
}

void AFDObject::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Director || Director->bPaused || !Director->bStarted || Director->CurrentRoom != Room) return;
    if (bSliding)
    {
        SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(),SlideTarget,DeltaSeconds,360));
        if (GetActorLocation().Equals(SlideTarget,1)) { SetActorLocation(SlideTarget); bSliding=false; bMoved=true; }
    }
    if (bBurning)
    {
        BurnTime -= DeltaSeconds;
        if (BurnTime<=0)
        {
            bBurned=true; bBurning=false;
            VisualRoot->SetVisibility(false,true);
            TArray<USceneComponent*> AttachedPieces; VisualRoot->GetChildrenComponents(true,AttachedPieces);
            for (USceneComponent* C : AttachedPieces) if (UNiagaraComponent* FX=Cast<UNiagaraComponent>(C)) FX->DestroyComponent();
            SetProjection(bProjected,Director->Player ? Director->Player->PlaneDepth : 0,Director->Player ? Director->Player->FoldView : 0);
        }
    }
    if (Kind==EFDKind::Receiver)
    {
        LitRemaining=FMath::Max(0.f,LitRemaining-DeltaSeconds); bLit=bActivated || LitRemaining>0;
        if (Surface) Surface->SetVectorParameterValue(TEXT("Glow"),FDVisuals::Gold*(bLit ? 6.f : .15f));
    }
    if (Kind==EFDKind::Exit && Surface)
        Surface->SetVectorParameterValue(TEXT("Glow"),(Director->RoomSolved() ? FDVisuals::Edge : FDVisuals::Gold)*3.f);
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDWorld.cpp

<!-- FILE: Source/NEWMyProject/FDWorld.cpp -->
```cpp
#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "FDVisuals.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AFDWorld::AFDWorld()
{
    PrimaryActorTick.bCanEverTick=true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    FireRoot=CreateDefaultSubobject<USceneComponent>(TEXT("FireRoot"));
    FireRoot->SetupAttachment(RootComponent);
    FireLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
    FireLight->SetupAttachment(FireRoot);
    FireLight->SetIntensity(600);
    FireLight->SetAttenuationRadius(650);
    FireLight->SetLightColor(FDVisuals::Gold);
    FireLight->SetCastShadows(false);
    FireFX=CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireFX"));
    FireFX->SetupAttachment(FireRoot);
    FireFX->SetAutoActivate(false);
    PreviewRoot=CreateDefaultSubobject<USceneComponent>(TEXT("Preview"));
    PreviewRoot->SetupAttachment(RootComponent);
}

void AFDWorld::BeginPlay()
{
    Super::BeginPlay();
    bSmoke=FParse::Param(FCommandLine::Get(),TEXT("FoldlightSmoke")) || FParse::Param(FCommandLine::Get(),TEXT("FoldlightTraversal"));
    bCapture=FParse::Param(FCommandLine::Get(),TEXT("FoldlightCapture"));
    BuildWorld();
    if (UGameplayStatics::DoesSaveGameExist(TEXT("FoldlightTrial_v1"),0))
        Save=Cast<UFDSave>(UGameplayStatics::LoadGameFromSlot(TEXT("FoldlightTrial_v1"),0));
    if (!Save || Save->Version!=1) Save=Cast<UFDSave>(UGameplayStatics::CreateSaveGameObject(UFDSave::StaticClass()));
    CurrentRoom=FMath::Clamp(Save->Room,0,4);
}

AFDObject* AFDWorld::Add(EFDKind Type, int32 Room, const FString& Id, FVector Local, FVector Size, FLinearColor Color)
{
    AFDObject* O=GetWorld()->SpawnActor<AFDObject>();
    O->Configure(this,Type,Room,Id,RoomOrigin(Room)+Local,Size,Color);
    Objects.Add(O); return O;
}
AFDObject* AFDWorld::Find(const FString& Id) const
{
    for (AFDObject* O : Objects) if (O && O->ObjectId==Id) return O;
    return nullptr;
}

void AFDWorld::BuildWorld()
{
    using namespace FDVisuals;
    const auto Platform=[&](int32 R,const TCHAR* Id,float X,float Y,float Top,float SX,float SY)
    {
        const float Height=Top+180;
        return Add(EFDKind::Platform,R,Id,FVector(X,Y,Top-Height*.5f),FVector(SX,SY,Height),Stone);
    };
    const auto Pillar=[&](int32 R,const TCHAR* Id,float X,float Y,float Top,float TargetX)
    {
        AFDObject* O=Add(EFDKind::Pillar,R,Id,FVector(X,Y,(Top-180)*.5f),FVector(320,340,Top+180),FLinearColor(.18f,.30f,.30f));
        O->SlideTarget=O->InitialLocation+FVector(TargetX-X,0,0);
        Piece(this,RootComponent,TEXT("Cube"),RoomOrigin(R)+FVector((X+TargetX)*.5f,Y,-174),FVector(FMath::Abs(TargetX-X)+320,18,10),Gold,1);
        for (float DY : {-170.f,170.f})
            Piece(this,RootComponent,TEXT("Cube"),RoomOrigin(R)+FVector(TargetX,Y+DY,-166),FVector(320,8,8),Gold,2);
        return O;
    };
    const auto Exit=[&](int32 R,float X,float Y,float Z)
    { Add(EFDKind::Exit,R,FString::Printf(TEXT("exit%d"),R),FVector(X,Y,Z+8),FVector(180,180,10),Gold); };

    Platform(0,TEXT("r0_start"),-950,0,0,900,1100);
    Platform(0,TEXT("r0_a"),-300,-500,70,420,350);
    Platform(0,TEXT("r0_b"),100,500,140,380,350);
    Platform(0,TEXT("r0_c"),490,-450,210,400,350);
    Platform(0,TEXT("r0_goal"),900,550,280,450,450); Exit(0,940,550,280);

    Platform(1,TEXT("r1_start"),-950,0,0,900,1200);
    Platform(1,TEXT("r1_a"),-320,450,70,360,350);
    Pillar(1,TEXT("r1_pillar"),-730,-450,140,80);
    Platform(1,TEXT("r1_c"),500,450,210,400,350);
    Platform(1,TEXT("r1_goal"),900,550,280,450,450); Exit(1,940,550,280);

    Platform(2,TEXT("r2_start"),0,0,0,2650,1100);
    Add(EFDKind::Vine,2,TEXT("r2_vine"),FVector(100,0,190),FVector(90,1060,380),Gold);
    Exit(2,1030,0,0);

    Platform(3,TEXT("r3_start"),-250,0,0,2300,1200);
    Add(EFDKind::Receiver,3,TEXT("r3_left"),FVector(-650,0,38),FVector(60,60,76),Gold);
    Add(EFDKind::Receiver,3,TEXT("r3_right"),FVector(-350,0,38),FVector(60,60,76),Gold);
    Add(EFDKind::Vine,3,TEXT("r3_vine"),FVector(450,450,410),FVector(80,350,380),Gold);
    Platform(3,TEXT("r3_step"),780,-430,70,280,350);
    Platform(3,TEXT("r3_goal"),1080,450,140,350,450); Exit(3,1110,450,140);
    // A suspended root seal communicates the second lock of the furnace room.
    Piece(this,RootComponent,TEXT("Cube"),RoomOrigin(3)+FVector(450,450,615),FVector(140,450,25),Stone);

    // Four segments turn south -> west -> north -> east. Floating slabs avoid
    // making later, higher platforms solid columns across earlier projections.
    const auto Slab=[&](const TCHAR* Id,FVector Center,float Top,FVector2D Footprint)
    { return Add(EFDKind::Platform,4,Id,FVector(Center.X,Center.Y,Top-30),FVector(Footprint.X,Footprint.Y,60),Stone); };
    Platform(4,TEXT("r4_start"),-1050,0,0,800,1100);
    Pillar(4,TEXT("r4_pillar"),-790,-450,70,-450);
    Slab(TEXT("r4_b"),FVector(0,500,0),140,FVector2D(400,400));
    AFDObject* South=Slab(TEXT("r4_south"),FVector(450,-450,0),210,FVector2D(450,450));
    Slab(TEXT("r4_w1"),FVector(-1550,-30,0),280,FVector2D(350,380));
    AFDObject* West=Slab(TEXT("r4_west"),FVector(-1050,390,0),350,FVector2D(450,450));
    Slab(TEXT("r4_n1"),FVector(-1500,900,0),420,FVector2D(380,350));
    AFDObject* North=Slab(TEXT("r4_north"),FVector(-1950,-900,0),490,FVector2D(450,450));
    Slab(TEXT("r4_e1"),FVector(0,-1350,0),560,FVector2D(350,380));
    AFDObject* East=Slab(TEXT("r4_goal"),FVector(500,-1800,0),630,FVector2D(450,450));
    const TArray<AFDObject*> Landings={South,East,North,West};
    for (int32 View=0;View<4;++View)
    {
        AFDObject* Landing=Landings[View];
        const FVector Local=Landing->GetActorLocation()-RoomOrigin(4)+FDProjection::Right(View)*230+FVector(0,0,150);
        AFDObject* Seal=Add(EFDKind::Receiver,4,FString::Printf(TEXT("r4_seal%d"),View),Local,FVector(70,70,90),Gold);
        Seal->RequiredView=View;
        // Four bars form a directional arrow on the landing.
        const FVector R=FDProjection::Right(View), N=FDProjection::Depth(View);
        for (int32 I=0;I<4;++I)
        {
            FVector P=Landing->GetActorLocation()+R*(I*35-55); P.Z=Landing->Top()+2;
            UStaticMeshComponent* Mark=Piece(this,Landing->VisualRoot,TEXT("Cube"),FVector::ZeroVector,FVector(7,100-I*22,3),Gold,.8f);
            Mark->SetWorldLocation(P); Mark->SetWorldRotation(R.Rotation());
        }
    }
    Exit(4,500,-1800,630);

    // Scenic architecture is intentionally collision-free and never participates in projection.
    for (int32 R=0;R<5;++R)
    {
        FVector O=RoomOrigin(R);
        USceneComponent* Scenery=NewObject<USceneComponent>(this);
        AddInstanceComponent(Scenery); Scenery->SetupAttachment(RootComponent);
        Scenery->SetRelativeLocation(O); Scenery->RegisterComponent(); SceneryRoots.Add(Scenery);
        BuildScenery(this,Scenery,R);
        for (float X : {-1050.f,1000.f})
        {
            UPointLightComponent* Light=NewObject<UPointLightComponent>(this);
            AddInstanceComponent(Light); Light->SetupAttachment(RootComponent);
            Light->SetMobility(EComponentMobility::Movable);
            Light->SetRelativeLocation(O+FVector(X,100,550));
            Light->SetIntensity(6500); Light->SetAttenuationRadius(2100);
            Light->SetLightColor(X<0 ? FLinearColor(.44f,.73f,.83f) : FLinearColor(1,.64f,.29f));
            Light->SetCastShadows(false); Light->RegisterComponent();
        }
    }
    ADirectionalLight* Sun=GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector,FRotator(-48,-35,0));
    Sun->GetLightComponent()->SetIntensity(5.f);
    Cast<UDirectionalLightComponent>(Sun->GetLightComponent())->SetForwardShadingPriority(1);
    Sun->GetLightComponent()->SetLightColor(FLinearColor(.72f,.85f,1));
    ADirectionalLight* Fill=GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector,FRotator(-20,-90,0));
    Fill->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Fill->GetLightComponent()->SetIntensity(2.7f);
    Fill->GetLightComponent()->SetLightColor(FLinearColor(.64f,.82f,.91f));
    Fill->GetLightComponent()->SetCastShadows(false);
    ASkyLight* Sky=GetWorld()->SpawnActor<ASkyLight>();
    Sky->GetLightComponent()->SetIntensity(.65f);
    Sky->GetLightComponent()->SetLightColor(FLinearColor(.30f,.48f,.60f));
    Sky->GetLightComponent()->SetCastShadows(false);
    Sky->GetLightComponent()->RecaptureSky();
    AExponentialHeightFog* Fog=GetWorld()->SpawnActor<AExponentialHeightFog>();
    Fog->GetComponent()->SetFogDensity(.014f);
    Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(.035f,.07f,.095f));
    Fog->SetActorLocation(FVector(0,0,-300));
    UMaterialInterface* SpellSurface=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Foldlight/Art/M_Spell.M_Spell"));
    UMaterialInstanceDynamic* SpellMaterial=UMaterialInstanceDynamic::Create(SpellSurface,this);
    SpellMaterial->SetVectorParameterValue(TEXT("Glow"),FLinearColor(1,.24f,.035f)*1.8f);
    for (int I=0;I<20;++I)
    {
        UStaticMeshComponent* Wisp=Piece(this,FireRoot,I<12 ? TEXT("SM_Flame") : TEXT("SM_Crystal"),FVector::ZeroVector,FVector(20),Gold);
        Wisp->SetMaterial(0,SpellMaterial); Wisp->SetCastShadow(false); FirePieces.Add(Wisp);
    }
    for (int I=0;I<2;++I)
    {
        UStaticMeshComponent* Ring=Piece(this,FireRoot,TEXT("SM_Sigil"),FVector::ZeroVector,FVector(100,100,1),FLinearColor(.85f,.32f,.07f),.6f);
        Ring->SetCastShadow(false); FireSigils.Add(Ring);
    }
    for (int I=0;I<12;++I)
    {
        UStaticMeshComponent* Glyph=Piece(this,FireRoot,TEXT("Cube"),FVector::ZeroVector,FVector(12,3,1),Gold,.7f);
        Glyph->SetCastShadow(false); FireGlyphs.Add(Glyph);
    }
    for (int I=0;I<4;++I)
        PreviewPieces.Add(Piece(this,PreviewRoot,TEXT("Cube"),FVector::ZeroVector,FVector(5),Edge,2));
    FireFX->SetAsset(LoadObject<UNiagaraSystem>(nullptr,TEXT("/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Big_Simple.NS_Fire_Big_Simple")));
    FireRoot->SetVisibility(false,true); PreviewRoot->SetVisibility(false,true);
}

void AFDWorld::Start(bool Fresh)
{
    if (!Player || !Save) return;
    bStarted=true; bPaused=false; bFinished=false; Falls=Switches=0; SessionTime=0;
    if (Fresh || Save->Finished) { Save->Room=0; Save->Finished=false; Persist(); }
    EnterRoom(FMath::Clamp(Save->Room,0,4),false);
    Notify(TEXT("Q / F 选择南、东、北、西视角；按住 V 预览，松开切换。H 获取提示。"),8);
}

void AFDWorld::EnterRoom(int32 Index, bool SaveProgress)
{
    if (!Player) return;
    Project(false,0);
    CurrentRoom=FMath::Clamp(Index,0,4); HintLevel=0; bUsed2D=false;
    Player->FoldView=0;
    bFurnaceSolved=false; FurnaceHold=0; FireRemaining=0; CastCooldown=0;
    FireFX->DeactivateImmediate(); FireRoot->SetVisibility(false,true);
    for (AFDObject* O : Objects) if (O->Room==CurrentRoom) O->ResetObject();
    Player->GetCharacterMovement()->SetComponentTickEnabled(true);
    Player->ApplyDimension(false,RoomOrigin(CurrentRoom)+FVector(-1100,0,90));
    Player->SetActorRotation(FRotator(0,0,0));
    Player->JumpBuffer=Player->CoyoteTime=0;
    if (SaveProgress) { Save->Room=CurrentRoom; Save->Finished=false; Persist(); }
}

void AFDWorld::ResetCurrentRoom(bool FromFall)
{
    if (FromFall) Falls++;
    EnterRoom(CurrentRoom,false);
    Notify(FromFall ? TEXT("已返回本关检查点，机关已复位。") : TEXT("本关已重置，试试另一种排列方式。"));
}

void AFDWorld::Project(bool Is2D, float PlaneDepth)
{
    for (AFDObject* O : Objects)
    {
        O->SetProjection(Is2D && O->Room==CurrentRoom,PlaneDepth,Player ? Player->FoldView : 0);
        O->VisualRoot->SetVisibility(O->Room==CurrentRoom && !O->bBurned,true);
    }
    UpdateScenery();
}

void AFDWorld::UpdateScenery()
{
    if (!Player) return;
    for (AFDObject* O : Objects)
        if (O->Room==CurrentRoom && O->Kind==EFDKind::Exit)
            O->VisualRoot->SetWorldRotation(FDProjection::Right(Player->FoldView).Rotation());
    for (int32 I=0;I<SceneryRoots.Num();++I)
    {
        SceneryRoots[I]->SetVisibility(I==CurrentRoom,true);
        if (I!=CurrentRoom) continue;
        const FVector N=FDProjection::Depth(Player->FoldView);
        FVector Origin=RoomOrigin(I);
        float BackdropDepth=FVector::DotProduct(Player->GetActorLocation(),N);
        // The arcade's nearest ornaments are about 950 cm behind its origin.
        // Keep another 450 cm clear behind the farthest source platform in
        // this view, including the tall tower's opposite-side exit.
        for (AFDObject* Object : Objects)
            if (Object->Room==CurrentRoom)
                BackdropDepth=FMath::Min(BackdropDepth,FVector::DotProduct(Object->GetActorLocation(),N)-FVector::DotProduct(Object->GetExtent(),N.GetAbs())+500.f);
        Origin=FDProjection::OnPlane(Origin,Player->FoldView,BackdropDepth);
        SceneryRoots[I]->SetWorldLocation(Origin);
        SceneryRoots[I]->SetWorldRotation(FDProjection::Right(Player->FoldView).Rotation());
    }
}

bool AFDWorld::IsSafe(FVector Location,const AFDCharacter* Character,bool Target2D,const AFDObject* Support) const
{
    const float Radius=Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float Half=Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || !O->Blocks() || O==Support) continue;
        FVector Center=O->GetActorLocation(), Ext=O->GetExtent();
        if (Target2D)
        {
            const int32 View=Character->FoldView;
            Center=FDProjection::OnPlane(Center,View,FVector::DotProduct(Location,FDProjection::Depth(View)));
            Ext=FDProjection::Right(View).GetAbs()*FDProjection::Width(O->Size,View)*.5f+FDProjection::Depth(View).GetAbs()*70+FVector(0,0,Ext.Z);
        }
        // Conservative capsule envelope keeps corners safe; touching a top is allowed.
        if (FMath::Abs(Location.X-Center.X)<Ext.X+Radius-2 &&
            FMath::Abs(Location.Y-Center.Y)<Ext.Y+Radius-2 &&
            FMath::Abs(Location.Z-Center.Z)<Ext.Z+Half-2) return false;
    }
    return true;
}

bool AFDWorld::CheckSwitch(const AFDCharacter* Character,FVector& Destination,FString& Reason) const
{
    Destination=Character->GetActorLocation();
    if (!Character->GetCharacterMovement()->IsMovingOnGround()) { Reason=TEXT("请先在平台上站稳，再切换维度。"); return false; }
    for (AFDObject* O : Objects) if (O->Room==CurrentRoom && O->bSliding) { Reason=TEXT("请等待石柱停止移动。"); return false; }
    if (!Character->b2D)
    {
        if (!IsSafe(Destination,Character,true)) { Reason=TEXT("折叠后会与障碍重叠，请移到空旷处。"); return false; }
        Reason=FString::Printf(TEXT("以%s折叠纵深。A / D 始终对应屏幕左 / 右；Q / F 可换预览方向。"),FDProjection::Name(Character->FoldView)); return true;
    }
    AFDObject* Support=Cast<AFDObject>(Character->GetCharacterMovement()->CurrentFloor.HitResult.GetActor());
    if (!Support || !Support->IsWalkable() || Support->Room!=CurrentRoom)
    { Reason=TEXT("请先落稳在带标记的平台上，再展开为三维。"); return false; }
    Destination=FDProjection::OnPlane(Destination,Character->FoldView,FVector::DotProduct(Support->GetActorLocation(),FDProjection::Depth(Character->FoldView)));
    Destination.Z=Support->Top()+Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+2;
    if (FMath::Abs(FVector::DotProduct(Destination-Support->GetActorLocation(),FDProjection::Right(Character->FoldView)))>FDProjection::Width(Support->Size,Character->FoldView)*.5f-8)
    { Reason=TEXT("请远离平台边缘，确保安全展开。"); return false; }
    if (!IsSafe(Destination,Character,false,Support))
    { Reason=TEXT("三维落点被障碍占据，请换一个平台。"); return false; }
    Reason=TEXT("展开后，将回到脚下平台原本的三维位置。"); return true;
}

FVector AFDWorld::GetCastAnchor() const
{
    if (!Player) return FVector::ZeroVector;
    FVector Direction=Player->GetActorForwardVector();
    if (Player->b2D) { const FVector Right=FDProjection::Right(Player->FoldView); Direction=Right*(FVector::DotProduct(Direction,Right)<0 ? -1 : 1); }
    FVector Anchor=Player->GetActorLocation()+Direction*230;
    const float Feet=Player->GetActorLocation().Z-Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    Anchor.Z=Feet;
    float Best=-100000;
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || !O->IsWalkable()) continue;
        const FVector Delta=Anchor-O->GetActorLocation();
        if (Player->b2D)
        { if (FMath::Abs(FVector::DotProduct(Delta,FDProjection::Right(Player->FoldView)))>FDProjection::Width(O->Size,Player->FoldView)*.5f) continue; }
        else if (FMath::Abs(Delta.X)>O->Size.X*.5f || FMath::Abs(Delta.Y)>O->Size.Y*.5f) continue;
        if (O->Top()<=Feet+45 && O->Top()>Best) Best=O->Top();
    }
    if (Best>-10000) Anchor.Z=Best+3;
    return Anchor;
}

void AFDWorld::PlaceFire()
{
    if (CurrentRoom<2) { Notify(TEXT("火焰印记将在第三关「余烬花园」中解锁。")); return; }
    if (CastCooldown>0) return;
    if (!Player->GetCharacterMovement()->IsMovingOnGround()) { Notify(TEXT("请站稳在平台上，再施放火焰印记。")); return; }
    FireAnchor=GetCastAnchor();
    FireSourceAnchor=FireAnchor;
    if (Player->b2D)
    {
        // Use the support's real depth as the persistent anchor for a later unfold.
        if (AFDObject* Support=Cast<AFDObject>(Player->GetCharacterMovement()->CurrentFloor.HitResult.GetActor()))
            FireSourceAnchor=FDProjection::OnPlane(FireAnchor,Player->FoldView,FVector::DotProduct(Support->GetActorLocation(),FDProjection::Depth(Player->FoldView)));
    }
    FireRemaining=4; CastCooldown=.45f;
    Player->CastSerial++; Player->CastPose=.9f;
    FDVisuals::Sound(this,TEXT("S_Fire"),.5f);
    RefreshFireVisual();
}

void AFDWorld::RefreshFireVisual()
{
    if (!Player) return;
    FireRoot->SetVisibility(FireRemaining>0,true);
    if (FireRemaining<=0) return;
    FireAnchor=Player->b2D ? FDProjection::OnPlane(FireSourceAnchor,Player->FoldView,Player->PlaneDepth) : FireSourceAnchor;
    FireRoot->SetWorldLocation(FireAnchor);
    FireRoot->SetWorldRotation(Player->b2D ? FDProjection::Right(Player->FoldView).Rotation() : FRotator::ZeroRotator);
    FireLight->SetRelativeLocation(FVector(0,0,Player->b2D ? 220 : 80));
    FireFX->DeactivateImmediate();
    AnimateFireVisual();
}

void AFDWorld::AnimateFireVisual()
{
    if (!Player || FireRemaining<=0) return;
    const bool Flat=Player->b2D;
    const float Time=4-FireRemaining;
    const float Fade=FMath::Clamp(FMath::Min(Time/.15f,FireRemaining/.3f),.01f,1.f);
    for (int I=0;I<FirePieces.Num();++I)
    {
        const float A=I*2*PI/12+Time*.35f;
        const float Phase=FMath::Fmod(Time*.48f+I*.137f,1.f);
        FVector P,S;
        if (Flat)
        {
            if (I<12)
            {
                const float Height=170+155*(.5f+.5f*FMath::Sin(Time*5+I*1.7f));
                P=FVector((I-5.5f)*7.5f+FMath::Sin(Time*4+I)*3,(I%3-1)*8,Height*.5f+5);
                S=FVector(14,11,Height);
            }
            else { P=FVector(FMath::Sin(A*3)*46,0,12+Phase*418); S=FVector(3,3,8)*(1-Phase*.7f); }
        }
        else
        {
            P=FVector(FMath::Cos(A)*174,FMath::Sin(A)*174,I<12 ? 20.f : 7+Phase*57);
            S=I<12 ? FVector(13,13,30+FMath::Sin(Time*7+I)*9) : FVector(3,3,6);
        }
        FirePieces[I]->SetRelativeLocation(P);
        FirePieces[I]->SetRelativeScale3D(S*(Fade/100.f));
        FirePieces[I]->SetRelativeRotation(FRotator(0,I*37+Time*30,Flat ? FMath::Sin(Time*3+I)*2 : 0));
    }
    for (int I=0;I<FireSigils.Num();++I)
    {
        FireSigils[I]->SetRelativeLocation(Flat && I==0 ? FVector(0,0,220) : FVector(0,0,2+I));
        FireSigils[I]->SetRelativeScale3D((Flat ? (I==0 ? FVector(120,440,1.2f) : FVector(110,70,1)) : FVector(I==0 ? 400 : 335,I==0 ? 400 : 335,1))/100.f);
        FireSigils[I]->SetRelativeRotation(Flat && I==0 ? FRotator(0,0,90) : FRotator(0,Time*(I ? -9 : 6),0));
        if (UMaterialInstanceDynamic* Mat=Cast<UMaterialInstanceDynamic>(FireSigils[I]->GetMaterial(0)))
            Mat->SetVectorParameterValue(TEXT("Glow"),FDVisuals::Gold*(Fade*.8f));
    }
    for (int I=0;I<FireGlyphs.Num();++I)
    {
        const float A=I*PI/6+Time*.1f;
        FireGlyphs[I]->SetRelativeLocation(Flat ? FVector(I%2 ? 56 : -56,-2,35+(I/2)*73) : FVector(FMath::Cos(A)*184,FMath::Sin(A)*184,3));
        FireGlyphs[I]->SetRelativeScale3D(Flat ? FVector(.04f,.02f,.10f) : FVector(.14f,.025f,.01f));
        FireGlyphs[I]->SetRelativeRotation(Flat ? FRotator::ZeroRotator : FRotator(0,FMath::RadiansToDegrees(A),0));
    }
    FireLight->SetIntensity((Flat ? 210.f : 120.f)*Fade*(.92f+.08f*FMath::Sin(Time*9)));
}

void AFDWorld::UpdateFire(float DeltaSeconds)
{
    if (FireRemaining<=0) return;
    FireRemaining=FMath::Max(0.f,FireRemaining-DeltaSeconds);
    if (FireRemaining<=0) { FireRoot->SetVisibility(false,true); FireFX->DeactivateImmediate(); return; }
    AnimateFireVisual();
    const bool Flat=Player->b2D;
    FVector Center=FireAnchor+FVector(0,0,Flat ? 220 : 35);
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || (O->Kind!=EFDKind::Vine && O->Kind!=EFDKind::Receiver)) continue;
        const FVector Delta=O->GetActorLocation()-Center, Ext=O->GetExtent();
        bool Hit=false;
        if (Flat)
            Hit=FMath::Abs(FVector::DotProduct(Delta,FDProjection::Right(Player->FoldView)))<60+FDProjection::Width(Ext,Player->FoldView) && FMath::Abs(Delta.Z)<220+Ext.Z;
        else
        {
            const float DX=FMath::Max(0.f,FMath::Abs(Delta.X)-Ext.X);
            const float DY=FMath::Max(0.f,FMath::Abs(Delta.Y)-Ext.Y);
            Hit=DX*DX+DY*DY<=200*200 && FMath::Abs(Delta.Z)<35+Ext.Z;
        }
        if (Hit)
        {
            if (O->Kind==EFDKind::Vine)
            {
                // The furnace must be restored before its suspended root seal can burn.
                if (O->ObjectId!=TEXT("r3_vine") || bFurnaceSolved) O->Ignite();
            }
            else if (O->RequiredView==INDEX_NONE) O->LitRemaining=.12f;
            else if (Flat && Player->FoldView==O->RequiredView && !O->bActivated)
            {
                O->bActivated=true;
                Notify(RoomSolved() ? TEXT("四向印记已全部点亮！展开为三维，在拱门前按 E 修复核心。") : FString::Printf(TEXT("%s印记已点亮。站稳后展开为三维，用 Q / F 寻找下一段阶梯。"),FDProjection::Name(O->RequiredView)),6);
                FDVisuals::Sound(this,TEXT("S_Restore"),.35f);
            }
        }
    }
}

bool AFDWorld::RoomSolved() const
{
    if (CurrentRoom==0) return bUsed2D;
    if (CurrentRoom==1) return bUsed2D && Find(TEXT("r1_pillar"))->bMoved;
    if (CurrentRoom==2) return Find(TEXT("r2_vine"))->bBurned;
    if (CurrentRoom==3) return bFurnaceSolved && Find(TEXT("r3_vine"))->bBurned;
    for (int32 I=0;I<4;++I) if (!Find(FString::Printf(TEXT("r4_seal%d"),I))->bActivated) return false;
    return Find(TEXT("r4_pillar"))->bMoved;
}

void AFDWorld::UseNearby()
{
    if (Player->b2D) { Notify(TEXT("请先展开为三维，再移动石柱或激活出口。")); return; }
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || O->Kind!=EFDKind::Pillar || O->bMoved) continue;
        if (FVector::Dist2D(Player->GetActorLocation(),O->GetActorLocation())<285)
        { O->Slide(); return; }
    }
    AFDObject* Exit=Find(FString::Printf(TEXT("exit%d"),CurrentRoom));
    if (Exit && FVector::Dist(Player->GetActorLocation(),Exit->GetActorLocation())<220)
    {
        if (!RoomSolved()) { Notify(Objective(),4); return; }
        FDVisuals::Sound(this,TEXT("S_Restore"),.55f);
        if (CurrentRoom<4)
        { EnterRoom(CurrentRoom+1,true); Notify(TEXT("本关已修复，通关进度已保存。"),4); }
        else
        {
            bFinished=true; Save->Room=4; Save->Finished=true; Persist();
            Player->GetCharacterMovement()->StopMovementImmediately();
            Player->GetCharacterMovement()->SetComponentTickEnabled(false);
        }
        return;
    }
    Notify(TEXT("靠近带标记的石柱或出口拱门，再按 E 交互。"));
}

FString AFDWorld::ContextAction() const
{
    if (!Player || Player->b2D) return FString();
    for (AFDObject* O : Objects)
        if (O->Room==CurrentRoom && O->Kind==EFDKind::Pillar && !O->bMoved && FVector::Dist2D(Player->GetActorLocation(),O->GetActorLocation())<285)
            return TEXT("E  /  移动石柱");
    AFDObject* Exit=Find(FString::Printf(TEXT("exit%d"),CurrentRoom));
    if (Exit && FVector::Dist(Player->GetActorLocation(),Exit->GetActorLocation())<220)
        return RoomSolved() ? TEXT("E  /  修复并前进") : TEXT("出口仍被封印");
    return FString();
}

void AFDWorld::Persist()
{
    if (!bSmoke && !bCapture && !UGameplayStatics::SaveGameToSlot(Save,TEXT("FoldlightTrial_v1"),0))
        Notify(TEXT("进度保存失败，本次游戏仍可继续。"));
}
void AFDWorld::Notify(const FString& Message,float Duration) { Toast=Message; ToastRemaining=Duration; }
FString AFDWorld::RoomTitle() const
{
    static const TCHAR* Names[]={TEXT("初次折叠"),TEXT("错位之桥"),TEXT("余烬花园"),TEXT("双生熔炉"),TEXT("四向折光塔")};
    return Names[FMath::Clamp(CurrentRoom,0,4)];
}
FString AFDWorld::Objective() const
{
    switch(CurrentRoom)
    {
    case 0: return TEXT("折叠错位的平台，在远端拱门前展开为三维。");
    case 1: return Find(TEXT("r1_pillar"))->bMoved ? TEXT("石柱已就位，折叠出一条跨越断层的路。") : TEXT("在三维中找到轨道石柱，将它移到阶梯缺口。");
    case 2: return Find(TEXT("r2_vine"))->bBurned ? TEXT("藤蔓封印已解除，前往出口拱门。") : TEXT("靠近藤蔓，按 G 施放已觉醒的火焰印记。");
    case 3: return !bFurnaceSolved ? TEXT("用一枚三维火印，同时点燃两座低位熔炉。") : (Find(TEXT("r3_vine"))->bBurned ? TEXT("两道封印均已解除，折叠平台前往出口。") : TEXT("将火印折叠为高墙，烧毁悬空的藤蔓。"));
    default:
        if (!Find(TEXT("r4_pillar"))->bMoved) return TEXT("先移动轨道石柱，再以南视折叠，开始换面登塔。");
        for (int32 View : {0,3,2,1})
            if (!Find(FString::Printf(TEXT("r4_seal%d"),View))->bActivated)
                return FString::Printf(TEXT("前往%s祭台，以同向二维火墙点亮印记；按 H 查看路线。"),FDProjection::Name(View));
        return TEXT("四向印记已点亮！在最高祭台展开为三维，按 E 修复核心。");
    }
}
void AFDWorld::RequestHint()
{
    HintLevel=FMath::Min(HintLevel+1,3);
    static const TCHAR* Hints[5][3]={
        {TEXT("金边平台的轮廓，可以在同一个平面上相连。"),TEXT("Q / F 可选择四个观察方向；南北压缩前后，东西压缩左右。"),TEXT("选择南视后按住再松开 V，向右跳跃；到终点展开为三维，在拱门前按 E。")},
        {TEXT("带标记的石柱，连接着一条金色轨道。"),TEXT("保持三维，靠近起始平台左后方的石柱。"),TEXT("靠近石柱按 E，等它停稳后按住再松开 V，沿对齐的阶梯跳跃。")},
        {TEXT("藤蔓已经干枯，留意琥珀色的施法范围预览。"),TEXT("火印会出现在蜡烛前方一小段距离处。"),TEXT("面朝藤蔓，在约两米外按 G；烧毁藤蔓后，前往远端拱门按 E。")},
        {TEXT("两座低位熔炉，需要同时受到火焰加热。"),TEXT("宽阔的三维火印能覆盖两座熔炉，狭窄的二维火墙则不行。"),TEXT("在三维中将火印放到双炉之间；解封后站到高处藤蔓左侧，切换二维再按 G。")},
        {TEXT("登塔路线依次是南视、西视、北视、东视；每到祭台，朝屏幕右侧施放火墙。"),TEXT("在祭台展开三维，Q / F 换方向，再折叠；新的阶梯会出现在屏幕右侧。"),TEXT("先按 E 移动石柱。沿南→西→北→东登塔，各祭台按 G 点亮印记，最后展开并按 E。")}
    };
    Notify(Hints[CurrentRoom][HintLevel-1],9);
}

void AFDWorld::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Player) Player=Cast<AFDCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if (!Player) return;
    Player->Director=this;
    if (!bInitialized)
    {
        bInitialized=true;
        Player->SetActorLocation(RoomOrigin(CurrentRoom)+FVector(-1100,0,90));
        Player->GetController()->SetControlRotation(FRotator(-27,-90,0));
        if (bSmoke || bCapture) Start(true);
    }
    if (bSmoke || bCapture) RunSmoke(DeltaSeconds);
    if (!bStarted || bPaused || bFinished) { PreviewRoot->SetVisibility(false,true); return; }
    SessionTime+=DeltaSeconds;
    CastCooldown=FMath::Max(0.f,CastCooldown-DeltaSeconds);
    ToastRemaining=FMath::Max(0.f,ToastRemaining-DeltaSeconds);
    UpdateFire(DeltaSeconds);
    if (CurrentRoom==3 && !bFurnaceSolved)
    {
        if (Find(TEXT("r3_left"))->LitRemaining>0 && Find(TEXT("r3_right"))->LitRemaining>0) FurnaceHold+=DeltaSeconds;
        else FurnaceHold=0;
        if (FurnaceHold>.65f) { bFurnaceSolved=true; Notify(TEXT("双生熔炉已修复，接下来烧毁高处的藤蔓封印。"),5); }
    }
    if (Player->GetActorLocation().Z<-600) { ResetCurrentRoom(true); return; }
    const bool ShowPreview=CurrentRoom>=2 && FireRemaining<=0 && !Player->bPreview;
    PreviewRoot->SetVisibility(ShowPreview,true);
    if (ShowPreview)
    {
        PreviewRoot->SetWorldLocation(GetCastAnchor());
        const bool Flat=Player->b2D;
        PreviewRoot->SetWorldRotation(Flat ? FDProjection::Right(Player->FoldView).Rotation() : FRotator::ZeroRotator);
        const float Width=Flat ? 120 : 400;
        for (int I=0;I<4;++I)
        {
            FVector P,S;
            if (Flat)
            {
                P=I<2 ? FVector((I ? 1 : -1)*Width*.5f,0,220) : FVector(0,0,I==2 ? 0 : 440);
                S=I<2 ? FVector(3,3,440) : FVector(Width,3,3);
            }
            else
            {
                P=I<2 ? FVector((I ? 1 : -1)*200,0,4) : FVector(0,(I==2 ? -1 : 1)*200,4);
                S=I<2 ? FVector(3,400,3) : FVector(400,3,3);
            }
            PreviewPieces[I]->SetRelativeLocation(P); PreviewPieces[I]->SetRelativeScale3D(S/100);
        }
    }
}

AFDGameMode::AFDGameMode()
{
    DefaultPawnClass=AFDCharacter::StaticClass(); HUDClass=AFDHUD::StaticClass();
}
void AFDGameMode::StartPlay()
{
    Super::StartPlay();
    if (!UGameplayStatics::GetActorOfClass(this,AFDWorld::StaticClass())) GetWorld()->SpawnActor<AFDWorld>();
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDArt.cpp

<!-- FILE: Source/NEWMyProject/FDArt.cpp -->
```cpp
#include "FDVisuals.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

void FDVisuals::BuildScenery(AActor* Owner,USceneComponent* Parent,int32 Room)
{
    // One instanced component per mesh/material combination. Hidden room roots
    // hide their full hierarchy; ornaments never acquire gameplay collision.
    TMap<FString,UInstancedStaticMeshComponent*> Batches;
    const auto Imported=[&](const TCHAR* Name,FVector Position,FVector Scale,FRotator Rotation=FRotator::ZeroRotator)
    {
        const FString Key=FString(TEXT("KayKit_"))+Name;
        UInstancedStaticMeshComponent* Batch=Batches.FindRef(Key);
        if (!Batch)
        {
            const FString Path=FString::Printf(TEXT("/Game/Foldlight/External/KayKit/Environment/SM_KK_%s.SM_KK_%s"),Name,Name);
            UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,*Path);
            if (!Mesh) return;
            Batch=NewObject<UInstancedStaticMeshComponent>(Owner);
            Owner->AddInstanceComponent(Batch); Batch->SetupAttachment(Parent);
            Batch->SetMobility(EComponentMobility::Movable);
            Batch->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Batch->SetStaticMesh(Mesh); Batch->SetCastShadow(false);
            Batch->RegisterComponent(); Batches.Add(Key,Batch);
        }
        Batch->AddInstance(FTransform(Rotation,Position,Scale));
    };
    const auto Part=[&](const TCHAR* Shape,FVector P,FVector S,FLinearColor C,float Glow=0.f,FRotator Rot=FRotator::ZeroRotator)
    {
        const FString Key=FString(Shape)+C.ToString()+FString::SanitizeFloat(Glow);
        UInstancedStaticMeshComponent* Batch=Batches.FindRef(Key);
        if (!Batch)
        {
            Batch=NewObject<UInstancedStaticMeshComponent>(Owner);
            Owner->AddInstanceComponent(Batch);Batch->SetupAttachment(Parent);
            Batch->SetMobility(EComponentMobility::Movable);
            Batch->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            const bool Custom=FString(Shape).StartsWith(TEXT("SM_"));
            const FString MeshPath=Custom ? FString::Printf(TEXT("/Game/Foldlight/Art/Models/%s.%s"),Shape,Shape) : FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape);
            Batch->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*MeshPath));
            UMaterialInstanceDynamic* Mat=Material(Owner,C,Glow);
            if (FString(Shape)==TEXT("SM_Stone") || FString(Shape).Contains(TEXT("Arch"))) Mat->SetScalarParameterValue(TEXT("Grain"),1.f);
            if (FString(Shape)==TEXT("SM_Root") || FString(Shape)==TEXT("SM_Leaf") || FString(Shape)==TEXT("SM_Grass") || FString(Shape)==TEXT("SM_Banner"))
            { Mat->SetScalarParameterValue(TEXT("Metallic"),0); Mat->SetScalarParameterValue(TEXT("Roughness"),.95f); }
            Batch->SetMaterial(0,Mat);Batch->SetCastShadow(false);
            Batch->RegisterComponent();Batches.Add(Key,Batch);
        }
        Batch->AddInstance(FTransform(Rot,P,S/100.f));
    };
    const FLinearColor Palettes[]={FLinearColor(.16f,.28f,.30f),FLinearColor(.20f,.24f,.32f),FLinearColor(.18f,.29f,.23f),FLinearColor(.29f,.20f,.18f),FLinearColor(.20f,.24f,.34f)};
    const FLinearColor Base=Palettes[FMath::Clamp(Room,0,4)];
    const FLinearColor Accent=Room==3 ? FLinearColor(.76f,.28f,.10f) : FLinearColor(.26f,.55f,.55f);
    // Atmospheric rear wall is behind both layers of the open arcade.
    Part(TEXT("Cube"),FVector(0,-2100,200),FVector(10000,90,4200),Base*.40f);
    for (int I=-5;I<=5;++I)
    {
        const float X=I*650.f;
        Part(TEXT("SM_Arch"),FVector(X,-1720,770),FVector(650,85,650),Base*.65f);
        for (float DX : {-290.f,290.f})
            Part(TEXT("SM_Stone"),FVector(X+DX,-1710,40),FVector(56,90,1130),Base*.65f);
    }
    for (int I=-5;I<=5;++I)
    {
        const float X=I*650.f;
        // An arch model occupies half its nominal height; its spring line is
        // center minus one quarter of that height.
        const bool Broken=(Room==1 || Room==2) && (FMath::Abs(I)%3==1);
        Part(Broken ? TEXT("SM_BrokenArch") : TEXT("SM_Arch"),FVector(X,-1330,635),FVector(650,130,650),Base*1.15f);
        for (float DX : {-325.f})
        {
            Imported(TEXT("pillar_decorated"),FVector(X+DX,-1370,-485),FVector(1.2f));
            Imported(TEXT("pillar_decorated"),FVector(X+DX,-1370,-5),FVector(1.2f));
        }
        // Suspended brass lantern and a cool crystal behind each opening.
        Part(TEXT("Cylinder"),FVector(X,-1380,660),FVector(3,3,230),Gold*.6f);
        Part(TEXT("SM_Ring"),FVector(X,-1380,525),FVector(90,90,5),Gold,.25f);
        Part(TEXT("SM_Crystal"),FVector(X,-1380,540),FVector(32,32,82),Accent,1.2f);
        Part(TEXT("SM_Pedestal"),FVector(X,-1430,-240),FVector(110,110,70),Base);
        Part(TEXT("SM_Crystal"),FVector(X,-1430,-145),FVector(65,65,145),Accent,.3f,FRotator(0,0,12));
        if ((Room==0 || Room==4) && I%2==0)
        {
            const FLinearColor Fabric=Room==0 ? FLinearColor(.08f,.24f,.25f) : FLinearColor(.22f,.13f,.31f);
            Part(TEXT("SM_Banner"),FVector(X+190,-1430,250),FVector(140,100,365),Fabric);
            Part(TEXT("Cylinder"),FVector(X+190,-1420,435),FVector(7,7,170),Gold*.7f,0,FRotator(90,0,0));
            Part(TEXT("SM_Ring"),FVector(X+190,-1400,295),FVector(64,64,3),Gold*.8f,.15f,FRotator(0,0,90));
            Part(TEXT("SM_Crystal"),FVector(X+190,-1395,295),FVector(18,4,43),Gold*.8f,.15f);
        }
    }
    // Author-made masonry and small vignettes occupy the rear arcade only.
    // Its root follows the active projection, keeping all four silhouettes clear.
    for (int I=-4;I<=4;++I)
    {
        const float X=I*650.f;
        if (I%2==0)
            Imported(Room==2 ? TEXT("wall_broken") : Room==4 ? TEXT("wall_archedwindow_open") : TEXT("wall_arched"),FVector(X,-1650,-485),FVector(1.6f));
        Imported(TEXT("torch_mounted"),FVector(X+230,-1210,185),FVector(1.5f));
        if (I%2!=0)
        {
            Part(TEXT("SM_Stone"),FVector(X,-1280,-325),FVector(410,250,60),Base*1.2f);
            if (Room==0 || Room==4)
            {
                Imported(TEXT("table_small_decorated_A"),FVector(X-85,-1260,-295),FVector(1.55f));
                Imported(TEXT("candle_triple"),FVector(X+115,-1260,-295),FVector(1.35f));
                Imported(TEXT("shelf_small_candles"),FVector(X,-1450,150),FVector(1.7f));
            }
            else if (Room==1 || Room==3)
            {
                Imported(TEXT("barrel_large_decorated"),FVector(X-75,-1310,-295),FVector(1.1f));
                Imported(TEXT("barrel_small_stack"),FVector(X+100,-1250,-295),FVector(.9f));
                Imported(TEXT("shelves"),FVector(X,-1460,-10),FVector(1.25f));
            }
            else Imported(TEXT("rubble_large"),FVector(X,-1260,-295),FVector(.48f,.65f,.6f));
        }
    }
    // Broken cornice and inset masonry courses; no full dark wall in front of it.
    for (int I=-8;I<=8;++I)
    {
        Part(TEXT("SM_Stone"),FVector(I*405.f,-1350,845),FVector(397,200,54),Base*1.35f);
        Part(TEXT("SM_Stone"),FVector(I*405.f,-1360,-540),FVector(397,210,74),Base);
        Part(TEXT("SM_Stone"),FVector(I*405.f,-1440,-615),FVector(394,280,74),Base*.8f);
    }
    // Monumental, distant astrolabe provides an identifiable room landmark.
    const FVector Center(300,-1940,490);
    if (Room==0 || Room==4)
    {
    Part(TEXT("SM_Ring"),Center,FVector(900,900,15),Gold*.65f,.12f,FRotator(0,0,90));
    Part(TEXT("SM_Ring"),Center+FVector(0,8,0),FVector(750,750,9),Accent,.35f,FRotator(0,0,90));
    for (int I=0;I<16;++I)
    {
        const float A=I*PI/8;
        Part(TEXT("SM_Crystal"),Center+FVector(FMath::Cos(A)*414,20,FMath::Sin(A)*414),FVector(15,14,36),Gold*.8f,.25f,FRotator(0,0,FMath::RadiansToDegrees(A)-90));
    }
    }
    if (Room==1)
    {
        for (int I=-2;I<=2;++I)
        {
            const float Diameter=I%2==0 ? 580.f : 340.f;
            Part(TEXT("SM_Gear"),FVector(I*390,-1880,330+(I%2)*230),FVector(Diameter,Diameter,26),Gold*.45f,.10f,FRotator(0,0,90));
            Part(TEXT("SM_Ring"),FVector(I*390,-1850,330+(I%2)*230),FVector(Diameter*.45f,Diameter*.45f,7),Accent,.45f,FRotator(0,0,90));
        }
    }
    if (Room==3)
    {
        for (int I=-3;I<=3;++I)
        {
            const FVector P(I*800,-1740,110);
            Part(TEXT("SM_Arch"),P+FVector(0,0,180),FVector(440,80,540),Base*1.7f);
            Part(TEXT("SM_Flame"),P+FVector(0,-90,40),FVector(90,80,245),FLinearColor(.75f,.15f,.025f),.9f);
            for (int J=-2;J<=2;++J)
                Part(TEXT("SM_Stone"),P+FVector(J*57,25,-50),FVector(13,23,380),Base*.55f);
            Part(TEXT("SM_Stone"),P+FVector(0,30,-235),FVector(440,140,45),Base);
        }
    }
    // Seeded low-poly rubble and firefly motes, shared in instanced batches.
    FRandomStream Random(410+Room);
    for (int I=0;I<48;++I)
    {
        const float X=Random.FRandRange(-3100,3100), Z=Random.FRandRange(-1000,-660);
        Part(TEXT("SM_Stone"),FVector(X,Random.FRandRange(-1700,-1000),Z),FVector(Random.FRandRange(45,150),90,Random.FRandRange(35,110)),Base*.8f,0,FRotator(Random.FRandRange(-30,30),Random.FRandRange(0,180),Random.FRandRange(-30,30)));
        Part(TEXT("Sphere"),FVector(Random.FRandRange(-3000,3000),Random.FRandRange(-1900,-1050),Random.FRandRange(-300,1300)),FVector(Random.FRandRange(2,5)),Accent,.9f);
    }
    if (Room==2 || Room==3)
    {
        for (int I=-4;I<=4;++I)
        {
            Part(TEXT("SM_Root"),FVector(I*620.f,-1220,220),FVector(390,220,1050),Room==2 ? FLinearColor(.15f,.24f,.14f) : FLinearColor(.23f,.12f,.08f),0,FRotator(0,0,(I%2)*12));
            for (int J=0;J<3;++J)
                Part(TEXT("SM_Crystal"),FVector(I*620.f+45,-1210,100+J*120),FVector(65,28,32),Accent,.25f,FRotator(0,0,J*45));
            if (Room==2)
            {
                for (int J=0;J<7;++J)
                {
                    const float Side=J%2 ? -1.f : 1.f;
                    Part(TEXT("SM_Leaf"),FVector(I*620.f+Side*56,-1190,-100+J*100),FVector(94,160,150),FLinearColor(.13f,.31f,.19f),0,FRotator(0,0,Side*55));
                    Part(TEXT("SM_Grass"),FVector(I*620.f+J*26-80,-1280,-490),FVector(120,120,130),FLinearColor(.12f,.25f,.14f));
                }
            }
        }
    }
    if (Room==4)
    {
        const FVector Stars[]={FVector(-1280,-1850,410),FVector(-960,-1850,720),FVector(-480,-1850,575),FVector(-190,-1850,955),FVector(380,-1850,1080),FVector(890,-1850,720),FVector(1280,-1850,840)};
        for (int I=0;I<7;++I)
        {
            Part(TEXT("SM_Crystal"),Stars[I],FVector(15,10,24),Accent,1.4f);
            if (I<6)
            {
                const FVector D=Stars[I+1]-Stars[I];
                Part(TEXT("Cube"),(Stars[I]+Stars[I+1])*.5f,FVector(D.Size(),2,2),Accent,.3f,D.Rotation());
            }
        }
    }
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDHUD.cpp

<!-- FILE: Source/NEWMyProject/FDHUD.cpp -->
```cpp
#include "FoldlightDemo.h"
#include "FDVisuals.h"
#include "FDProjection.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/FontFace.h"
#include "Kismet/GameplayStatics.h"

void AFDHUD::Text(const FString& Value,float X,float Y,float Size,FLinearColor Color,bool Centered)
{
    if (!ChineseFont)
    {
        UFontFace* Face=LoadObject<UFontFace>(nullptr,TEXT("/Game/Foldlight/Fonts/DroidSansFallback.DroidSansFallback"));
        checkf(Face,TEXT("The packaged Chinese font face is required by Foldlight HUD."));
        ChineseFont=NewObject<UFont>(this);
        ChineseFont->FontCacheType=EFontCacheType::Runtime;
        ChineseFont->LegacyFontSize=10;
        FTypefaceEntry Entry(FName(TEXT("Regular")));
        Entry.Font=FFontData(Face);
        ChineseFont->GetMutableInternalCompositeFont().DefaultTypeface.Fonts.Add(Entry);
    }
    const float S=Canvas->ClipY/720.f;
    // Rasterize at the final point size instead of enlarging a small glyph atlas.
    const FSlateFontInfo FontInfo(ChineseFont,15.f*Size*S,FName(TEXT("Regular")));
    FCanvasTextItem Item(FVector2D(X*S,Y*S),FText::FromString(Value),FontInfo,Color);
    Item.bCentreX=Centered;
    Item.EnableShadow(FLinearColor(0,0,0,.75f),FVector2D(S,2*S));
    Canvas->DrawItem(Item);
}
void AFDHUD::Panel(float X,float Y,float W,float H,float Alpha)
{
    const float S=Canvas->ClipY/720.f;
    DrawRect(FLinearColor(.012f,.023f,.03f,Alpha),X*S,Y*S,W*S,H*S);
}
void AFDHUD::DrawHUD()
{
    Super::DrawHUD();
    AFDWorld* D=Cast<AFDWorld>(UGameplayStatics::GetActorOfClass(this,AFDWorld::StaticClass()));
    if (!D || !D->Player || !Canvas) return;
    const float W=Canvas->ClipX/(Canvas->ClipY/720.f);
    const FLinearColor White(.94f,.91f,.82f), Muted(.54f,.66f,.68f), Gold=FDVisuals::Gold, Teal=FDVisuals::Edge;
    if (!D->bStarted)
    {
        Panel(0,0,W,720,.27f); Panel(0,0,650,720,.83f);
        Text(TEXT("一束微光，照见折叠的世界。"),55,75,.8f,Teal);
        Text(TEXT("折光遗迹"),50,155,3.9f,White);
        Text(TEXT("四向折叠试玩 · 五重试炼"),56,245,1.05f,Gold);
        Text(TEXT("在三维世界中，重排前行的道路。"),56,330,1.05f,White);
        Text(TEXT("旋转四个视角，让远近化为脚下的阶梯。"),56,366,1.05f,White);
        Text(TEXT("将一圈火印，化作一面烈焰之墙。"),56,402,1.05f,White);
        Text(D->Save && D->Save->Room>0 && !D->Save->Finished ? TEXT("回车键   /   继续旅程") : TEXT("回车键   /   开始旅程"),56,510,1.15f,Gold);
        Text(TEXT("N  新的旅程     F10  退出游戏"),56,558,.9f,Muted);
        Text(TEXT("试玩版 0.5   /   使用虚幻引擎 5.7 制作"),56,662,.68f,Muted);
        return;
    }
    if (D->bFinished)
    {
        Panel(0,0,W,720,.85f);
        Text(TEXT("世界重新找回了它的纵深。"),65,125,.95f,Teal);
        Text(TEXT("微光重燃"),60,205,3.1f,White);
        Text(TEXT("五重试炼，一簇微火，照亮前路。"),65,302,1.1f,White);
        Text(FString::Printf(TEXT("用时 %02d:%02d   /   折叠 %d 次   /   坠落 %d 次"),int(D->SessionTime)/60,int(D->SessionTime)%60,D->Switches,D->Falls),65,379,1,Gold);
        Text(TEXT("你已完成第一版试玩的全部关卡。"),65,453,.95f,Muted);
        Text(TEXT("回车键  再玩一次       F10  退出游戏"),65,560,1.1f,Gold);
        return;
    }
    Panel(25,25,615,95,.78f);
    Text(FString::Printf(TEXT("0%d  /  %s"),D->CurrentRoom+1,*D->RoomTitle()),43,39,1.12f,White);
    Text(D->Objective(),43,79,.73f,Muted);
    Panel(W-250,25,225,125,.86f);
    Text(D->Player->b2D ? TEXT("二维 / 已折叠") : TEXT("三维 / 已展开"),W-193,39,1.05f,D->Player->b2D ? Gold : Teal);
    const FString ViewHelp=D->Player->b2D ? FString::Printf(TEXT("%s · 先用 V 展开，再换方向"),FDProjection::Name(D->Player->FoldView)) : FString::Printf(TEXT("%s · Q / F 换方向"),FDProjection::Name(D->Player->FoldView));
    Text(ViewHelp,W-233,110,.62f,Gold);
    for (int I=0;I<5;++I)
    {
        float S=Canvas->ClipY/720.f;
        DrawRect(I<=D->CurrentRoom ? Gold : FLinearColor(.12f,.19f,.20f), (W-191+I*31)*S,86*S,22*S,4*S);
    }
    if (D->CurrentRoom>=2)
    {
        Panel(25,139,215,69,.73f);
        Text(D->Player->b2D ? TEXT("G  /  烈焰之墙") : TEXT("G  /  火焰印记"),43,151,.9f,Gold);
        Text(D->FireRemaining>0 ? FString::Printf(TEXT("剩余 %.1f 秒 / 一枚火印"),D->FireRemaining) : TEXT("已就绪 / 替换现有火印"),43,181,.67f,Muted);
    }
    if (D->CurrentRoom==4)
    {
        Panel(25,222,215,110,.85f);
        Text(TEXT("四向印记 · 南 西 北 东"),43,234,.74f,White);
        for (int32 I=0;I<4;++I)
        {
            const int32 Order[]={0,3,2,1};
            const int32 Face=Order[I];
            Text(FString::Printf(TEXT("%s %s"),FDProjection::Name(Face),D->Find(FString::Printf(TEXT("r4_seal%d"),Face))->bActivated ? TEXT("已亮") : TEXT("未亮")),43+(I%2)*96,264+(I/2)*27,.69f,D->Find(FString::Printf(TEXT("r4_seal%d"),Face))->bActivated ? Gold : Muted);
        }
    }
    const FString Action=D->ContextAction();
    if (!Action.IsEmpty()) { Panel(W*.5f-220,548,440,43,.84f); Text(Action,W*.5f-202,558,.95f,Gold); }
    if (D->ToastRemaining>0 && !D->Player->bPreview)
    {
        Panel(25,603,W-50,45,.86f); Text(D->Toast,43,616,.78f,White);
    }
    Panel(0,671,W,49,.91f);
    Text(TEXT("WASD 移动 / 空格 跳跃 / Q·F 选方向 / 按住 V 预览，松开切换 / G 施法 / E 交互 / R 重置 / H 提示 / Esc 暂停"),25,685,.72f,Muted);
    if (D->Player->bPreview)
    {
        FVector Destination; FString Reason;
        const bool Valid=D->Player->CanSwitch(Destination,Reason);
        Panel(25,592,W-50,66,.95f);
        Text(Valid ? TEXT("松开 V，切换维度") : TEXT("暂时无法切换维度"),43,602,.95f,Valid ? Teal : Gold);
        Text(Reason,43,633,.72f,White);
        if (Valid && D->Player->b2D)
        {
            FVector2D P;
            if (PlayerOwner->ProjectWorldLocationToScreen(Destination,P))
            {
                DrawLine(P.X-18,P.Y,P.X+18,P.Y,Gold,2);
                DrawLine(P.X,P.Y-18,P.X,P.Y+18,Gold,2);
            }
        }
    }
    if (D->bPaused)
    {
        Panel(0,0,W,720,.82f);
        Text(TEXT("稍作歇息"),W*.5f,230,2.3f,White,true);
        Text(TEXT("Esc / 回车键  继续游戏"),W*.5f,365,1.1f,Gold,true);
        Text(TEXT("F10  退出到桌面"),W*.5f,415,.9f,Muted,true);
        Text(TEXT("每通过一个房间，进度都会自动保存。"),W*.5f,500,.85f,Muted,true);
    }
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDMageAnimInstance.h

<!-- FILE: Source/NEWMyProject/FDMageAnimInstance.h -->
```cpp
#pragma once
#include "Animation/AnimInstance.h"
#include "FDMageAnimInstance.generated.h"

class UAnimSequence;

/** Native blend graph for the imported KayKit rig; capsule movement remains authoritative. */
UCLASS()
class NEWMYPROJECT_API UFDMageAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    UPROPERTY(Transient) TArray<TObjectPtr<UAnimSequence>> Clips;
protected:
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
    virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;
};
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDMageAnimInstance.cpp

<!-- FILE: Source/NEWMyProject/FDMageAnimInstance.cpp -->
```cpp
#include "FDMageAnimInstance.h"
#include "FoldlightDemo.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimSequence.h"
#include "AnimNodes/AnimNode_TwoWayBlend.h"
#include "AnimNodes/AnimNode_LayeredBoneBlend.h"
#include "GameFramework/CharacterMovementComponent.h"

class FFDMageProxy final : public FAnimInstanceProxy
{
public:
    explicit FFDMageProxy(UAnimInstance* Owner) : FAnimInstanceProxy(Owner) {}
    FAnimNode_SequencePlayer_Standalone Idle, Walk, Run, JumpStart, JumpLoop, Land, Cast;
    FAnimNode_TwoWayBlend Gait, Moving, Jump, Air, Landing;
    FAnimNode_LayeredBoneBlend UpperBody;
    float Speed=0, AirTime=0, LandTime=0, CastTime=0;
    bool bAir=false;
    int32 LastCast=0;

    virtual void Initialize(UAnimInstance* InInstance) override
    {
        auto* Instance=CastChecked<UFDMageAnimInstance>(InInstance);
        Instance->Clips.Reset();
        const TCHAR* Names[]={TEXT("Idle"),TEXT("Walking_A"),TEXT("Running_A"),TEXT("Jump_Start"),TEXT("Jump_Idle"),TEXT("Jump_Land"),TEXT("Spellcast_Shoot")};
        FAnimNode_SequencePlayer_Standalone* Players[]={&Idle,&Walk,&Run,&JumpStart,&JumpLoop,&Land,&Cast};
        for (int32 I=0;I<UE_ARRAY_COUNT(Names);++I)
        {
            const FString Path=FString::Printf(TEXT("/Game/Foldlight/External/KayKit/Mage/Mage/SkeletalMeshes/Mage%s.Mage%s"),Names[I],Names[I]);
            UAnimSequence* Clip=LoadObject<UAnimSequence>(nullptr,*Path);
            Instance->Clips.Add(Clip); Players[I]->SetSequence(Clip);
            Players[I]->SetLoopAnimation(I<3 || I==4);
        }
        // Keep walk/run phases in sync as speed changes.
        Walk.SetGroupName(TEXT("Gait")); Run.SetGroupName(TEXT("Gait"));
        Walk.SetGroupMethod(EAnimSyncMethod::SyncGroup); Run.SetGroupMethod(EAnimSyncMethod::SyncGroup);
        JumpStart.SetPlayRate(2.5f); Land.SetPlayRate(2.f);
        Gait.A.SetLinkNode(&Walk); Gait.B.SetLinkNode(&Run);
        Moving.A.SetLinkNode(&Idle); Moving.B.SetLinkNode(&Gait);
        Jump.A.SetLinkNode(&JumpStart); Jump.B.SetLinkNode(&JumpLoop);
        Air.A.SetLinkNode(&Moving); Air.B.SetLinkNode(&Jump);
        Landing.A.SetLinkNode(&Air); Landing.B.SetLinkNode(&Land);
        UpperBody.BasePose.SetLinkNode(&Landing);
        UpperBody.AddPose(); UpperBody.BlendPoses[0].SetLinkNode(&Cast);
        FBranchFilter Filter; Filter.BoneName=TEXT("spine"); Filter.BlendDepth=1;
        UpperBody.LayerSetup[0].BranchFilters.Add(Filter);
        UpperBody.bMeshSpaceRotationBlend=true;
        UpperBody.BlendWeights[0]=0;
        FAnimInstanceProxy::Initialize(InInstance);
    }
    virtual FAnimNode_Base* GetCustomRootNode() override { return &UpperBody; }
    virtual void GetCustomNodes(TArray<FAnimNode_Base*>& Nodes) override
    { Nodes.Append({&Idle,&Walk,&Run,&JumpStart,&JumpLoop,&Land,&Cast,&Gait,&Moving,&Jump,&Air,&Landing,&UpperBody}); }
    virtual void PreUpdate(UAnimInstance* Instance,float Dt) override
    {
        FAnimInstanceProxy::PreUpdate(Instance,Dt);
        const AFDCharacter* Hero=::Cast<AFDCharacter>(Instance->TryGetPawnOwner());
        if (!Hero) return;
        Speed=Hero->GetVelocity().Size2D();
        const bool NewAir=Hero->GetCharacterMovement()->IsFalling();
        if (NewAir && !bAir) { AirTime=0; JumpStart.SetAccumulatedTime(0); }
        if (!NewAir && bAir) { LandTime=.32f; Land.SetAccumulatedTime(0); }
        bAir=NewAir;
        if (Hero->CastSerial!=LastCast)
        { LastCast=Hero->CastSerial; CastTime=.9f; Cast.SetAccumulatedTime(0); }
    }
    virtual void Update(float Dt) override
    {
        AirTime=bAir ? AirTime+Dt : 0;
        LandTime=FMath::Max(0.f,LandTime-Dt); CastTime=FMath::Max(0.f,CastTime-Dt);
        Gait.Alpha=FMath::Clamp((Speed-140.f)/230.f,0.f,1.f);
        Moving.Alpha=FMath::FInterpTo(Moving.Alpha,FMath::Clamp(Speed/100.f,0.f,1.f),Dt,12);
        Walk.SetPlayRate(FMath::Clamp(Speed/160.f,.7f,1.5f));
        Run.SetPlayRate(FMath::Clamp(Speed/420.f,.7f,1.15f));
        Jump.Alpha=FMath::Clamp((AirTime-.12f)/.10f,0.f,1.f);
        Air.Alpha=FMath::FInterpTo(Air.Alpha,bAir ? 1.f : 0.f,Dt,18);
        Landing.Alpha=FMath::FInterpTo(Landing.Alpha,LandTime>.12f && !bAir ? .75f : 0.f,Dt,20);
        const float CastTarget=FMath::Min((.9f-CastTime)/.08f,CastTime/.18f);
        UpperBody.BlendWeights[0]=FMath::Clamp(CastTarget,0.f,1.f);
    }
};

FAnimInstanceProxy* UFDMageAnimInstance::CreateAnimInstanceProxy() { return new FFDMageProxy(this); }
void UFDMageAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) { delete Proxy; }
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDValidation.cpp

<!-- FILE: Source/NEWMyProject/FDValidation.cpp -->
```cpp
#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformMisc.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "FDMageAnimInstance.h"

void AFDWorld::Check(bool Condition,const FString& Label)
{
    if (Condition) { UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TEST PASS: %s"),*Label); }
    else { SmokeFailures++; UE_LOG(LogTemp,Error,TEXT("FOLDLIGHT_TEST FAIL: %s"),*Label); }
}

void AFDWorld::RunSmoke(float DeltaSeconds)
{
    static FTransform InitialChestPose;
    if (FParse::Param(FCommandLine::Get(),TEXT("FoldlightTraversal"))) { RunTraversal(DeltaSeconds); return; }
    if (bFourTest) { RunFourViewTests(DeltaSeconds); return; }
    SmokeClock+=DeltaSeconds;
    if (bCapture)
    {
        int32 CaptureRoom=0; FParse::Value(FCommandLine::Get(),TEXT("CaptureRoom="),CaptureRoom);
        FString CaptureAction; FParse::Value(FCommandLine::Get(),TEXT("CaptureAction="),CaptureAction);
        if (SmokeStep==3 && (CaptureAction==TEXT("Run") || CaptureAction==TEXT("Walk")))
            Player->AddMovementInput(FDProjection::Right(Player->FoldView),1);
        if (SmokeStep==0 && SmokeClock>2)
        {
            EnterRoom(FMath::Clamp(CaptureRoom,0,4),false);
            if (CaptureRoom==0) Player->SetActorLocation(FVector(-610,0,90));
            if (CaptureRoom==0 && !CaptureAction.IsEmpty()) Player->SetActorLocation(FVector(-900,0,90));
            if (CaptureRoom==3) Player->SetActorLocation(RoomOrigin(3)+FVector(0,0,90));
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureTitle"))) bStarted=false;
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==1 && SmokeClock>3)
        {
            int32 View=0; FParse::Value(FCommandLine::Get(),TEXT("CaptureView="),View);
            Player->SelectView(View);
            if (CaptureRoom==4)
            {
                const TCHAR* Platforms[]={TEXT("r4_south"),TEXT("r4_goal"),TEXT("r4_north"),TEXT("r4_west")};
                AFDObject* Landing=Find(Platforms[FDProjection::Wrap(View)]);
                Player->SetActorLocation(Landing->GetActorLocation()+FVector(0,0,120));
                Player->SetActorRotation(FDProjection::Right(View).Rotation());
                Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
            }
            else if (FParse::Param(FCommandLine::Get(),TEXT("Capture2D"))) Player->TrySwitch();
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==2 && SmokeClock>2)
        {
            if (CaptureRoom==4 && FParse::Param(FCommandLine::Get(),TEXT("Capture2D")) && !Player->b2D)
            { Player->TrySwitch(); SmokeClock=0; return; }
            if (CaptureRoom>=2) PlaceFire();
            if (CaptureAction==TEXT("Walk")) Player->GetCharacterMovement()->MaxWalkSpeed=150;
            if (CaptureAction==TEXT("Jump")) Player->LaunchCharacter(FVector(0,0,540),false,true);
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==3 && SmokeClock>(CaptureAction.IsEmpty() ? .7f : .35f))
        {
            FString Name=FString::Printf(TEXT("Foldlight_room%d_view%d_%s.png"),CaptureRoom,Player->FoldView,Player->b2D ? TEXT("2D") : TEXT("3D"));
            if (!CaptureAction.IsEmpty()) Name=Name.Replace(TEXT(".png"),*FString::Printf(TEXT("_%s.png"),*CaptureAction));
            // Capture real UI states without modifying player saves.
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureTitle"))) Name=TEXT("Foldlight_title.png");
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureHint")))
            { HintLevel=2; RequestHint(); Name=TEXT("Foldlight_hint.png"); }
            if (FParse::Param(FCommandLine::Get(),TEXT("CapturePreview")))
            { Player->PreviewStart(); Name=TEXT("Foldlight_preview.png"); }
            if (FParse::Param(FCommandLine::Get(),TEXT("CapturePause")))
            { Player->PauseToggle(); Name=TEXT("Foldlight_pause.png"); }
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureEnd")))
            { bFinished=true; Name=TEXT("Foldlight_end.png"); }
            FScreenshotRequest::RequestScreenshot(FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Screenshots"),Name),true,false);
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==4 && SmokeClock>2) FPlatformMisc::RequestExit(false);
        return;
    }
    if (SmokeClock<1.3f) return;
    SmokeClock=0;
    const auto At=[&](FVector Local)
    {
        Player->SetActorLocation(RoomOrigin(CurrentRoom)+Local,false,nullptr,ETeleportType::TeleportPhysics);
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        Player->SetActorRotation(FRotator(0,0,0));
    };
    switch(SmokeStep++)
    {
    case 0:
        {
            USkeletalMeshComponent* HeroMesh=Player->GetMesh();
            UFDMageAnimInstance* Animation=Cast<UFDMageAnimInstance>(HeroMesh->GetAnimInstance());
            Check(Player->bSkeletalHero && HeroMesh->GetSkeletalMeshAsset() && Animation,TEXT("KayKit skeletal mage and native animation graph are active"));
            bool ClipsValid=Animation && Animation->Clips.Num()==7;
            if (Animation) for (UAnimSequence* Clip : Animation->Clips)
                ClipsValid &= Clip && Clip->GetPlayLength()>0 && Clip->GetSkeleton()==HeroMesh->GetSkeletalMeshAsset()->GetSkeleton();
            Check(ClipsValid,TEXT("All seven runtime animation clips match the imported mage skeleton"));
            Check(HeroMesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("Skeletal character preserves capsule-only gameplay collision"));
            InitialChestPose=HeroMesh->GetBoneTransform(HeroMesh->GetBoneIndex(TEXT("chest")),FTransform::Identity);
            bool ExternalPresent=true;
            for (const TCHAR* Name : {TEXT("pillar_decorated"),TEXT("wall_arched"),TEXT("wall_broken"),TEXT("torch_mounted"),TEXT("shelf_small_candles"),TEXT("table_small_decorated_A"),TEXT("barrel_large_decorated"),TEXT("barrel_small_stack"),TEXT("candle_triple"),TEXT("rubble_large"),TEXT("wall_archedwindow_open"),TEXT("shelves")})
                ExternalPresent &= LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Foldlight/External/KayKit/Environment/SM_KK_%s.SM_KK_%s"),Name,Name))!=nullptr;
            Check(ExternalPresent,TEXT("All twelve licensed environment meshes are cooked into the package"));
            bool ModelsPresent=true, VisualsSafe=true, SceneryInstanced=true;
            for (const TCHAR* Name : {TEXT("SM_Stone"),TEXT("SM_Arch"),TEXT("SM_Ring"),TEXT("SM_Pedestal"),TEXT("SM_Crystal"),TEXT("SM_Flame"),TEXT("SM_Wax"),TEXT("SM_Cape"),TEXT("SM_Root"),TEXT("SM_Leaf"),TEXT("SM_Grass"),TEXT("SM_Banner"),TEXT("SM_Gear"),TEXT("SM_BrokenArch"),TEXT("SM_Sigil")})
                ModelsPresent &= LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Foldlight/Art/Models/%s.%s"),Name,Name))!=nullptr;
            for (AFDObject* Object : Objects)
                for (UStaticMeshComponent* Mesh : Object->Pieces)
                    VisualsSafe &= Mesh->GetStaticMesh()!=nullptr && Mesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision;
            for (USceneComponent* Root : SceneryRoots)
            {
                bool HasInstances=false;
                TArray<USceneComponent*> ScenicParts; Root->GetChildrenComponents(true,ScenicParts);
                for (USceneComponent* Child : ScenicParts)
                    if (UInstancedStaticMeshComponent* Batch=Cast<UInstancedStaticMeshComponent>(Child))
                    { HasInstances |= Batch->GetInstanceCount()>0; VisualsSafe &= Batch->GetCollisionEnabled()==ECollisionEnabled::NoCollision && Batch->GetStaticMesh()!=nullptr; }
                SceneryInstanced &= HasInstances;
            }
            Check(ModelsPresent,TEXT("Fifteen authored art meshes are included in the playable package"));
            Check(VisualsSafe,TEXT("Art meshes exist and cannot obstruct gameplay collision"));
            Check(SceneryInstanced,TEXT("Every room batches repeated scenery into mesh instances"));
            Check(LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Foldlight/Art/M_Spell.M_Spell"))!=nullptr,TEXT("The readable spell surface is included in the package"));
            bool SpellSafe=FireSigils.Num()==2 && FireGlyphs.Num()==12;
            for (UStaticMeshComponent* Wisp : FirePieces) SpellSafe &= Wisp->GetStaticMesh()!=nullptr && Wisp->GetCollisionEnabled()==ECollisionEnabled::NoCollision;
            Check(SpellSafe,TEXT("Animated spell ornaments do not add gameplay collision"));
        }
        Check(Objects.Num()>=25,TEXT("Five procedural rooms have their gameplay objects"));
        Check(Player->GetCharacterMovement()->IsMovingOnGround(),TEXT("Player spawns on a real floor"));
        Check(Player->TrySwitch(),TEXT("Grounded 3D to 2D transition succeeds"));
        Check(Find(TEXT("r0_b"))->Projection->GetCollisionEnabled()!=ECollisionEnabled::NoCollision,TEXT("Distant source creates a walkable projection"));
        break;
    case 1:
        Check(!InitialChestPose.Equals(Player->GetMesh()->GetBoneTransform(Player->GetMesh()->GetBoneIndex(TEXT("chest")),FTransform::Identity),.0001f),TEXT("Imported animation changes bone pose across frames"));
        At(FVector(100,Player->PlaneDepth,232)); break;
    case 2:
        {
            AFDObject* Obstruction=Add(EFDKind::Platform,0,TEXT("test_blocked_destination"),FVector(100,500,230),FVector(100,100,150),FLinearColor::Gray);
            Check(!Player->TrySwitch(),TEXT("An occupied 3D destination rejects unfolding"));
            Check(Player->b2D && FMath::Abs(Player->GetActorLocation().Y-Player->PlaneDepth)<1,TEXT("Rejected transition preserves player dimension and position"));
            Objects.Remove(Obstruction); Obstruction->Destroy();
        }
        Check(Player->TrySwitch(),TEXT("2D support resolves to a safe 3D source"));
        Check(FMath::Abs(Player->GetActorLocation().Y-500)<1,TEXT("Unfold uses source depth, not a global deepest candidate"));
        break;
    case 3:
        At(FVector(100,500,650));
        Check(!Player->TrySwitch(),TEXT("Airborne folding is rejected"));
        EnterRoom(1,false); break;
    case 4:
        Find(TEXT("r1_pillar"))->Slide();
        Check(!Player->TrySwitch(),TEXT("Moving pillar blocks dimension commit")); break;
    case 5: break;
    case 6:
        Check(Find(TEXT("r1_pillar"))->bMoved,TEXT("Rail pillar reaches its authored stop"));
        ResetCurrentRoom();
        Check(!Find(TEXT("r1_pillar"))->bMoved && Find(TEXT("r1_pillar"))->GetActorLocation().Equals(Find(TEXT("r1_pillar"))->InitialLocation),TEXT("Room reset restores pillar transform and state"));
        EnterRoom(2,false); At(FVector(-140,0,90)); break;
    case 7: PlaceFire(); break;
    case 8: break;
    case 9:
        Check(Find(TEXT("r2_vine"))->bBurned,TEXT("3D sigil burns a real target after its burn delay"));
        Check(Find(TEXT("r2_vine"))->Solid->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("Burned source collision is removed"));
        Project(true,0);
        Check(Find(TEXT("r2_vine"))->Projection->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("Burned source cannot leave a ghost projection"));
        ResetCurrentRoom();
        Check(!Find(TEXT("r2_vine"))->bBurned && Find(TEXT("r2_vine"))->Solid->GetCollisionEnabled()!=ECollisionEnabled::NoCollision,TEXT("Reset reconstructs a burned obstacle"));
        EnterRoom(3,false); At(FVector(-730,0,90)); break;
    case 10: Player->TrySwitch(); break;
    case 11: PlaceFire(); break;
    case 12:
        Check(!bFurnaceSolved,TEXT("Narrow fire wall cannot activate separated furnaces"));
        { float Life=FireRemaining; Player->TrySwitch(); Check(FireRemaining<=Life,TEXT("Changing dimensions does not refresh spell lifetime")); }
        break;
    case 13: PlaceFire(); break;
    case 14:
        Check(bFurnaceSolved,TEXT("Single 3D sigil activates both low furnaces"));
        At(FVector(220,0,90)); break;
    case 15: PlaceFire(); break;
    case 16:
        Check(!Find(TEXT("r3_vine"))->bBurning,TEXT("Low 3D fire does not reach suspended roots"));
        Player->TrySwitch(); break;
    case 17: PlaceFire(); break;
    case 18: break;
    case 19:
        Check(Find(TEXT("r3_vine"))->bBurned,TEXT("2D fire hits the projected target at a different source depth"));
        Check(RoomSolved(),TEXT("Both furnace room conditions latch correctly"));
        EnterRoom(4,false);
        bFourTest=true; SmokeStep=0; SmokeClock=0; break;
    }
}

void AFDWorld::RunTraversal(float DeltaSeconds)
{
    SmokeClock+=DeltaSeconds;
    if (SessionTime>240 || Falls>0)
    {
        UE_LOG(LogTemp,Error,TEXT("FOLDLIGHT_TRAVERSAL FAIL room=%d stage=%d falls=%d location=%s"),CurrentRoom,SmokeStep,Falls,*Player->GetActorLocation().ToString());
        FPlatformMisc::RequestExitWithStatus(false,1); return;
    }
    if (!Player->InputAllowed()) return;
    const FVector Local=Player->GetActorLocation()-RoomOrigin(CurrentRoom);
    const auto Walk=[&](FVector2D Target)
    {
        FVector Direction(Target.X-Local.X,Target.Y-Local.Y,0);
        if (Direction.Size2D()<15) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
        Player->AddMovementInput(Direction.GetSafeNormal(),1); return false;
    };
    const auto Advance=[&](int32 Next) { SmokeStep=Next; SmokeClock=0; };
    const auto Leave=[&]()
    {
        const int32 Before=CurrentRoom;
        Player->Interact();
        if (bFinished)
        {
            UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TRAVERSAL COMPLETE: all five rooms traversed with movement and jump physics; falls=%d"),Falls);
            FPlatformMisc::RequestExitWithStatus(false,0);
        }
        else if (CurrentRoom!=Before)
        {
            UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TRAVERSAL PASS: room %d completed with movement physics"),Before+1);
            Advance(0);
        }
    };
    if (SmokeStep==0 && SmokeClock>1)
    {
        if (CurrentRoom==0) { if (Player->TrySwitch()) Advance(10); }
        else if (CurrentRoom==1 || CurrentRoom==4) Advance(1);
        else if (CurrentRoom==2) Advance(20);
        else Advance(30);
    }
    else if (SmokeStep==1)
    {
        if (Walk(FVector2D(CurrentRoom==1 ? -930 : -1000,CurrentRoom==1 ? -280 : -320))) { Player->Interact(); Advance(2); }
    }
    else if (SmokeStep==2)
    {
        AFDObject* Pillar=Find(CurrentRoom==1 ? TEXT("r1_pillar") : TEXT("r4_pillar"));
        if (Pillar->bMoved && Walk(FVector2D(-1100,0))) { if (Player->TrySwitch()) Advance(10); }
    }
    else if (SmokeStep==10)
    {
        TArray<FString> Path;
        if (CurrentRoom==0) Path={TEXT("r0_start"),TEXT("r0_a"),TEXT("r0_b"),TEXT("r0_c"),TEXT("r0_goal")};
        if (CurrentRoom==1) Path={TEXT("r1_start"),TEXT("r1_a"),TEXT("r1_pillar"),TEXT("r1_c"),TEXT("r1_goal")};
        if (CurrentRoom==3) Path={TEXT("r3_start"),TEXT("r3_step"),TEXT("r3_goal")};
        if (CurrentRoom==4)
        {
            switch(Player->FoldView)
            {
            case 0: Path={TEXT("r4_start"),TEXT("r4_pillar"),TEXT("r4_b"),TEXT("r4_south")}; break;
            case 3: Path={TEXT("r4_south"),TEXT("r4_w1"),TEXT("r4_west")}; break;
            case 2: Path={TEXT("r4_west"),TEXT("r4_n1"),TEXT("r4_north")}; break;
            default: Path={TEXT("r4_north"),TEXT("r4_e1"),TEXT("r4_goal")}; break;
            }
        }
        const FVector Right=FDProjection::Right(Player->FoldView);
        const float Position=FVector::DotProduct(Player->GetActorLocation(),Right);
        AFDObject* Support=Cast<AFDObject>(Player->GetCharacterMovement()->CurrentFloor.HitResult.GetActor());
        const int32 Index=Support ? Path.IndexOfByKey(Support->ObjectId) : INDEX_NONE;
        const bool Ground=Player->GetCharacterMovement()->IsMovingOnGround();
        if (Ground && Index==Path.Num()-1 && Position>=FVector::DotProduct(Support->GetActorLocation(),Right))
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            if (CurrentRoom==4)
            {
                Player->SetActorRotation(Right.Rotation());
                if (!Find(FString::Printf(TEXT("r4_seal%d"),Player->FoldView))->bActivated)
                { Player->CastFire(); return; }
                if (Player->TrySwitch()) Advance(Player->FoldView==1 ? 11 : 40);
            }
            else if (Player->TrySwitch()) Advance(11);
            return;
        }
        Player->MoveRight(1);
        if (Ground && Index>=0 && Index<Path.Num()-1)
        {
            AFDObject* Next=Find(Path[Index+1]);
            float Edge=FMath::Min(FVector::DotProduct(Support->GetActorLocation(),Right)+FDProjection::Width(Support->Size,Player->FoldView)*.5f,FVector::DotProduct(Next->GetActorLocation(),Right)-FDProjection::Width(Next->Size,Player->FoldView)*.5f)-75;
            if (Position>=Edge) Player->JumpPressed();
        }
    }
    else if (SmokeStep==40 && SmokeClock>.6f)
    {
        const int32 NextView=Player->FoldView==0 ? 3 : (Player->FoldView==3 ? 2 : 1);
        Player->SelectView(NextView);
        if (Player->TrySwitch()) Advance(10);
    }
    else if (SmokeStep==11 && SmokeClock>.6f) Leave();
    else if (SmokeStep==20)
    {
        if (Walk(FVector2D(-140,0))) { Player->SetActorRotation(FRotator::ZeroRotator); Player->CastFire(); Advance(21); }
    }
    else if (SmokeStep==21 && Find(TEXT("r2_vine"))->bBurned) Advance(22);
    else if (SmokeStep==22) { if (Walk(FVector2D(1030,0))) Leave(); }
    else if (SmokeStep==30)
    {
        if (Walk(FVector2D(-730,0))) { Player->SetActorRotation(FRotator::ZeroRotator); Player->CastFire(); Advance(31); }
    }
    else if (SmokeStep==31 && bFurnaceSolved) Advance(32);
    else if (SmokeStep==32)
    {
        if (Walk(FVector2D(220,0)) && Player->TrySwitch()) Advance(33);
    }
    else if (SmokeStep==33)
    {
        Player->SetActorRotation(FRotator::ZeroRotator); Player->CastFire(); Advance(34);
    }
    else if (SmokeStep==34 && Find(TEXT("r3_vine"))->bBurned) Advance(10);
}
```
<!-- END FILE -->

### 文件 Source/NEWMyProject/FDFourViewTests.cpp

<!-- FILE: Source/NEWMyProject/FDFourViewTests.cpp -->
```cpp
#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

void AFDWorld::RunFourViewTests(float DeltaSeconds)
{
    SmokeClock+=DeltaSeconds;
    const int32 View=SmokeStep/9, Stage=SmokeStep%9;
    const FVector R=FDProjection::Right(View), N=FDProjection::Depth(View);
    const FVector Base=RoomOrigin(4)+FVector(2200,2400,1600);
    const FString Prefix=FString::Printf(TEXT("View %d: "),View);
    const auto Assert=[&](bool OK,const TCHAR* Label) { Check(OK,Prefix+Label); };
    const auto Remove=[&](const TCHAR* Id)
    { if (AFDObject* O=Find(Id)) { Objects.Remove(O); O->Destroy(); } };
    if (Stage==2 && SmokeClock>.35f && SmokeClock<.65f) Player->MoveRight(1);
    if (SmokeClock<(Stage==2 ? .8f : (Stage==7 ? 2.f : .7f))) return;
    SmokeClock=0;
    switch(Stage)
    {
    case 0:
        Player->ApplyDimension(false,Base+FVector(0,0,118));
        Player->FoldView=View;
        Add(EFDKind::Platform,4,TEXT("four_source"),Base-RoomOrigin(4),FVector(600,280,60),FLinearColor::Gray);
        Add(EFDKind::Platform,4,TEXT("four_target"),Base-RoomOrigin(4)+R*900+N*850,FVector(600,280,60),FLinearColor::Gray);
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        break;
    case 1:
        Assert(Player->TrySwitch(),TEXT("grounded fold succeeds"));
        Assert(FVector::DotProduct(Player->GetCharacterMovement()->GetPlaneConstraintNormal(),N)>.99f,TEXT("movement plane matches selected depth axis"));
        {
            AFDObject* Target=Find(TEXT("four_target"));
            Assert(FMath::IsNearlyEqual(FDProjection::Width(Target->Projection->GetUnscaledBoxExtent(),View),FDProjection::Width(Target->Size,View)*.5f),TEXT("asymmetric platform uses the correct projected width"));
            const int32 Before=Player->FoldView;
            Assert(!Player->SelectView(View+1) && Player->FoldView==Before,TEXT("in-plane direction change requires unfolding first"));
        }
        break;
    case 2:
        // The spring-arm socket updates on its component tick after the switch.
        UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_CAMERA view=%d selected=%d camera=%s boom=%s actor=%s absolute=%d control=%d"),View,Player->FoldView,*Player->Camera->GetComponentRotation().ToString(),*Player->Boom->GetComponentRotation().ToString(),*Player->GetActorRotation().ToString(),Player->Boom->IsUsingAbsoluteRotation(),Player->Boom->bUsePawnControlRotation);
        Assert(FVector::DotProduct(Player->Camera->GetForwardVector(),-N)>.99f,TEXT("camera looks from the selected cardinal side after its update"));
        Assert(FVector::DotProduct(Player->GetActorLocation()-Base,R)>10,TEXT("D physically moves toward screen-right, including mirrored views"));
        Assert(FMath::Abs(FVector::DotProduct(Player->GetActorLocation(),N)-Player->PlaneDepth)<1,TEXT("movement preserves collapsed depth"));
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetActorLocation(FDProjection::OnPlane(Find(TEXT("four_target"))->GetActorLocation()+FVector(0,0,118),View,Player->PlaneDepth));
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        break;
    case 3:
        {
            const FVector Before=Player->GetActorLocation();
            const FVector Landing=Find(TEXT("four_target"))->GetActorLocation()+FVector(0,0,118);
            Add(EFDKind::Platform,4,TEXT("four_block"),Landing-RoomOrigin(4),FVector(70,70,130),FLinearColor::Gray);
            Assert(!Player->TrySwitch() && Player->b2D && Player->GetActorLocation().Equals(Before),TEXT("occupied source rejects unfolding without changing state"));
            Remove(TEXT("four_block"));
            Assert(Player->TrySwitch(),TEXT("safe source unfolds"));
            Assert(FMath::Abs(FVector::DotProduct(Player->GetActorLocation()-Landing,N))<1,TEXT("unfold restores the support's true X or Y depth"));
        }
        break;
    case 4:
        {
            const FVector Target=Find(TEXT("four_target"))->GetActorLocation();
            Add(EFDKind::Vine,4,TEXT("four_vine"),Target-RoomOrigin(4)+R*230+N*500+FVector(0,0,210),FVector(70,70,360),FLinearColor::Yellow);
            Assert(Player->TrySwitch(),TEXT("fold for a cross-depth fire target succeeds"));
            Player->SetActorRotation(R.Rotation());
        }
        break;
    case 5: PlaceFire(); break;
    case 6:
        {
            AFDObject* Rune=Add(EFDKind::Receiver,4,TEXT("four_rune"),FireSourceAnchor-RoomOrigin(4)+FVector(0,0,100),FVector(60),FLinearColor::Yellow);
            Rune->RequiredView=(View+1)%4;
        }
        break;
    case 7:
        Assert(Find(TEXT("four_vine"))->bBurned,TEXT("fire wall burns a target across the selected depth axis"));
        Assert(Find(TEXT("four_vine"))->Projection->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("burn removes this view's projected collision"));
        Assert(!Find(TEXT("four_rune"))->bActivated,TEXT("wrong-view fire cannot activate a directional seal"));
        Find(TEXT("four_rune"))->RequiredView=View;
        PlaceFire();
        break;
    case 8:
        {
            Assert(Find(TEXT("four_rune"))->bActivated,TEXT("matching view activates a directional seal"));
            const FVector Source=FireSourceAnchor; const float Lifetime=FireRemaining;
            Assert(Player->TrySwitch(),TEXT("unfold with an active spell succeeds"));
            Assert(FireSourceAnchor.Equals(Source) && FireAnchor.Equals(Source) && FireRemaining<=Lifetime,TEXT("unfold preserves the spell source and remaining lifetime"));
            Remove(TEXT("four_source")); Remove(TEXT("four_target")); Remove(TEXT("four_vine")); Remove(TEXT("four_rune"));
            FireRemaining=0;
            if (View==3)
            {
                EnterRoom(4,false);
                Assert(!Find(TEXT("r4_seal0"))->bActivated && !Find(TEXT("r4_seal1"))->bActivated && !Find(TEXT("r4_seal2"))->bActivated && !Find(TEXT("r4_seal3"))->bActivated,TEXT("room reset clears all directional seals"));
                UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TEST COMPLETE failures=%d"),SmokeFailures);
                FPlatformMisc::RequestExitWithStatus(false,SmokeFailures ? 1 : 0);
            }
        }
        break;
    }
    ++SmokeStep;
}
```
<!-- END FILE -->

### 文件 Config/DefaultEngine.ini

<!-- FILE: Config/DefaultEngine.ini -->
```ini
[/Script/EngineSettings.GameMapsSettings]
EditorStartupMap=/Game/Foldlight/Maps/Trial
GameDefaultMap=/Game/Foldlight/Maps/Trial
GlobalDefaultGameMode=/Script/NEWMyProject.FDGameMode

[/Script/Engine.RendererSettings]
r.DefaultFeature.AutoExposure=False
r.DefaultFeature.MotionBlur=False
r.DefaultFeature.AntiAliasing=2
r.DynamicGlobalIlluminationMethod=0
r.ReflectionMethod=0
r.Shadow.Virtual.Enable=0
r.RayTracing=False
r.AllowStaticLighting=False
r.GenerateMeshDistanceFields=False
r.DefaultFeature.Bloom=True

[/Script/WindowsTargetPlatform.WindowsTargetSettings]
DefaultGraphicsRHI=DefaultGraphicsRHI_DX11
+D3D11TargetedShaderFormats=PCD3D_SM5

[/Script/Engine.Engine]
NearClipPlane=5.0

[DevOptions.Shaders]
ShaderCompilerCoreCountThreshold=999
NumUnusedShaderCompilingThreads=28
NumUnusedShaderCompilingThreadsDuringGame=28
```
<!-- END FILE -->

### 文件 Config/DefaultGame.ini

<!-- FILE: Config/DefaultGame.ini -->
```ini
[/Script/EngineSettings.GeneralProjectSettings]
ProjectID=478C48C64BEF42E995C5F35490437F38
ProjectName=折光遗迹
ProjectDisplayedTitle=NSLOCTEXT("Foldlight", "WindowTitle", "折光遗迹 · 中文试玩版")
ProjectVersion=0.5.0
Description=五重试炼，折叠维度，以火焰解开遗迹谜题。

[/Script/UnrealEd.ProjectPackagingSettings]
BuildConfiguration=PPBC_Development
FullRebuild=False
ForDistribution=False
IncludeDebugFiles=False
bUsePakFile=True
bUseIoStore=True
bCookAll=False
+MapsToCook=(FilePath="/Game/Foldlight/Maps/Trial")
+DirectoriesToAlwaysCook=(Path="/Game/Foldlight")
+DirectoriesToAlwaysCook=(Path="/Game/Vefects/Free_Fire/Shared")
+DirectoriesToAlwaysCook=(Path="/Engine/BasicShapes")
```
<!-- END FILE -->

### 文件 Config/DefaultInput.ini

<!-- FILE: Config/DefaultInput.ini -->
```ini
[/Script/Engine.InputSettings]
-AxisConfig=(AxisKeyName="Gamepad_LeftX",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
-AxisConfig=(AxisKeyName="Gamepad_LeftY",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
-AxisConfig=(AxisKeyName="Gamepad_RightX",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
-AxisConfig=(AxisKeyName="Gamepad_RightY",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
-AxisConfig=(AxisKeyName="MouseX",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=0.07f))
-AxisConfig=(AxisKeyName="MouseY",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=0.07f))
-AxisConfig=(AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=0.07f))
+AxisConfig=(AxisKeyName="Gamepad_LeftX",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Gamepad_LeftY",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Gamepad_RightX",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Gamepad_RightY",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MouseX",AxisProperties=(DeadZone=0.000000,Sensitivity=0.070000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MouseY",AxisProperties=(DeadZone=0.000000,Sensitivity=0.070000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.000000,Sensitivity=0.070000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MouseWheelAxis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Gamepad_LeftTriggerAxis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Gamepad_RightTriggerAxis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Gamepad_Special_Left_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Gamepad_Special_Left_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Vive_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Vive_Left_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Vive_Left_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Vive_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Vive_Right_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="Vive_Right_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Left_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Left_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Left_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Left_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Right_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Right_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Right_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="MixedReality_Right_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Left_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Left_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Left_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Right_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Right_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="OculusTouch_Right_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Grip_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trackpad_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Grip_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trackpad_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
bAltEnterTogglesFullscreen=True
bF11TogglesFullscreen=True
bUseMouseForTouch=False
bEnableMouseSmoothing=False
bEnableFOVScaling=True
bCaptureMouseOnLaunch=True
bEnableLegacyInputScales=True
bEnableMotionControls=True
bFilterInputByPlatformUser=False
bShouldFlushPressedKeysOnViewportFocusLost=True
bAlwaysShowTouchInterface=False
bShowConsoleOnFourFingerTap=True
bEnableGestureRecognizer=False
bUseAutocorrect=False
bEnabledLegacyMappingDeprecationWarnings=True
DefaultViewportMouseCaptureMode=CapturePermanently_IncludingInitialMouseDown
DefaultViewportMouseLockMode=LockOnCapture
FOVScale=0.011110
DoubleClickTime=0.200000
+AxisMappings=(AxisName="FD_Forward",Scale=1.000000,Key=W)
+AxisMappings=(AxisName="FD_Forward",Scale=-1.000000,Key=S)
+AxisMappings=(AxisName="FD_Right",Scale=1.000000,Key=D)
+AxisMappings=(AxisName="FD_Right",Scale=-1.000000,Key=A)
+AxisMappings=(AxisName="FD_Turn",Scale=1.000000,Key=MouseX)
+AxisMappings=(AxisName="FD_Look",Scale=-1.000000,Key=MouseY)
DeprecatedActionAndAxisNames=()
DefaultPlayerInputClass=/Script/Engine.PlayerInput
DefaultInputComponentClass=/Script/Engine.InputComponent
DefaultTouchInterface=/Engine/MobileResources/HUD/DefaultVirtualJoysticks.DefaultVirtualJoysticks
-ConsoleKeys=Tilde
+ConsoleKeys=Tilde
```
<!-- END FILE -->

### 文件 Tools/make_audio.py

<!-- FILE: Tools/make_audio.py -->
```python
"""Original synthesized sounds; no external audio samples are used."""
from pathlib import Path
import math
import random
import struct
import wave

dest = Path(__file__).parent / "GeneratedAudio"
dest.mkdir(exist_ok=True)
rate = 22050
rng = random.Random(613)

def write(name, seconds, generator):
    data = [generator(i / rate, seconds) for i in range(int(seconds * rate))]
    peak = max(max(abs(v) for v in data), 0.01)
    with wave.open(str(dest / (name + ".wav")), "wb") as out:
        out.setparams((1, 2, rate, 0, "NONE", "not compressed"))
        out.writeframes(b"".join(struct.pack("<h", int(v / peak * 22000)) for v in data))

write("S_Fold", .55, lambda t, d: math.sin(math.pi*t/d)**2 * (.4*rng.uniform(-1,1) + .55*math.sin(2*math.pi*(150*t+650*t*t))))
write("S_Fire", .75, lambda t, d: min(t*40,1)*(1-t/d)**2 * (.7*rng.uniform(-1,1)+.15*math.sin(2*math.pi*75*t)))
write("S_Restore", 1.7, lambda t, d: min(t*30,1)*math.exp(-3*t) * sum(math.sin(2*math.pi*f*t)*a for f,a in [(440,.45),(554.37,.3),(659.25,.3),(880,.15)]))
print("Created three original WAV files in", dest)
```
<!-- END FILE -->

### 文件 Tools/make_models.py

<!-- FILE: Tools/make_models.py -->
```python
"""Original Foldlight meshes. Deterministic, dependency-free OBJ authoring in cm.

All assets fit a 100 cm authoring envelope and are collision-free in game.
Source geometry is retained so the art can be edited without an external DCC.
"""
import math
from pathlib import Path

OUT = Path(__file__).parent / 'GeneratedModels'
OUT.mkdir(exist_ok=True)

class Mesh:
    def __init__(self):
        self.vertices, self.faces = [], []
    def face(self, points, reverse=False):
        start = len(self.vertices) + 1
        self.vertices.extend(points)
        for i in range(1, len(points)-1):
            tri = (start, start+i, start+i+1)
            self.faces.append(tri[::-1] if reverse else tri)
    def save(self, name):
        with (OUT / (name + '.obj')).open('w', encoding='ascii') as f:
            f.write('# Foldlight original procedural model; centimeters; Z up\n')
            for v in self.vertices: f.write('v %.6f %.6f %.6f\n' % v)
            f.write('s off\n')
            for tri in self.faces: f.write('f %d %d %d\n' % tri)
        print(name, len(self.faces), 'triangles')

def rings(mesh, rows, caps=True):
    n = len(rows[0])
    for a,b in zip(rows, rows[1:]):
        for i in range(n):
            j=(i+1)%n
            mesh.face([a[i],a[j],b[j],b[i]])
    if caps:
        mesh.face(rows[0], True)
        mesh.face(rows[-1])

def lathe(profile, segments=32, deform=None):
    mesh=Mesh(); rows=[]
    for k,(z,r) in enumerate(profile):
        row=[]
        for i in range(segments):
            a=i*math.tau/segments
            p=(r*math.cos(a),r*math.sin(a),z)
            row.append(deform(p,a,k) if deform else p)
        rows.append(row)
    rings(mesh,rows)
    return mesh

# Beveled slab, with clipped corners and a narrow chamfer on every edge.
m=Mesh(); rows=[]
for z,r,c in [(-50,46,3),(-46,50,5),(46,50,5),(50,46,3)]:
    rows.append([(r-c,r,z),(-r+c,r,z),(-r,r-c,z),(-r,-r+c,z),(-r+c,-r,z),(r-c,-r,z),(r,-r+c,z),(r,r-c,z)])
rings(m,rows);m.save('SM_Stone')

# Voussoir arch: individual wedges include narrow mortar gaps.
m=Mesh()
for i in range(17):
    a=i*math.pi/17+.008; b=(i+1)*math.pi/17-.008
    p=[(r*math.cos(t), y, r*math.sin(t)-25) for y in (-50,50) for r,t in [(38,a),(50,a),(50,b),(38,b)]]
    for ids in [(0,1,2,3),(7,6,5,4),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]:m.face([p[j] for j in ids])
m.save('SM_Arch')

# Ring lies in XY, centered on Z; can be rotated vertically for astrolabes.
m=Mesh()
for i in range(64):
    a=i*math.tau/64;b=(i+1)*math.tau/64
    p=[(r*math.cos(t),r*math.sin(t),z) for z in (-50,50) for r,t in [(44,a),(50,a),(50,b),(44,b)]]
    for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:m.face([p[j] for j in ids])
m.save('SM_Ring')
lathe([(-50,37),(-45,48),(-35,50),(-27,37),(-18,32),(26,29),(32,40),(42,40),(50,34)],24).save('SM_Pedestal')
lathe([(-50,0),(-31,34),(8,50),(29,27),(50,0)],7).save('SM_Crystal')
lathe([(-50,0),(-39,29),(-20,45),(0,40),(23,22),(40,8),(50,0)],24,
      lambda p,a,k:(p[0]+max(0,k-3)**2*1.4,p[1],p[2])).save('SM_Flame')

def wax(p,a,k):
    x,y,z=p
    if k in (3,4,5): z+=2.4*math.sin(5*a)+1.2*math.sin(9*a)
    return x,y,z
lathe([(-50,43),(-46,48),(29,48),(44,49),(48,44),(42,32),(36,20),(36,0)],48,wax).save('SM_Wax')

def cloth(p,a,k):
    x,y,z=p
    ripple=1+.055*math.cos(10*a)
    return x*ripple-8*(1-k/4), y*ripple, z+(3*math.cos(5*a) if k==0 else 0)
lathe([(-46,44),(-39,47),(-10,35),(24,23),(46,21)],48,cloth).save('SM_Cape')

# Twisting root silhouette, a single mesh replacing straight cylinder branches.
m=Mesh();rows=[]
for k in range(17):
    z=-50+k*100/16; x=13*math.sin(k*.58);y=7*math.sin(k*.91)
    r=8*(1-k/22)
    rows.append([(x+r*math.cos(i*math.tau/8),y+r*math.sin(i*math.tau/8),z) for i in range(8)])
rings(m,rows);m.save('SM_Root')

# Folded, double-sided leaf: raised midrib catches light without textures.
m=Mesh()
outline=[(0,0,-50),(-32,2,-18),(-42,1,15),(-18,0,37),(0,0,50),(18,0,37),(42,1,15),(32,2,-18)]
center=(0,10,4)
for i in range(len(outline)):
    tri=[outline[i],outline[(i+1)%len(outline)],center]
    m.face(tri);m.face(tri,True)
m.save('SM_Leaf')

# Individual grass blades in a compact tuft, authored upward from the floor.
m=Mesh()
for i in range(9):
    a=i*2.399963; r=6+(i%3)*8; h=58+(i*17)%40
    x,y=r*math.cos(a),r*math.sin(a)
    side=(-math.sin(a)*3.5,math.cos(a)*3.5,0)
    tri=[(x+side[0],y+side[1],0),(x-side[0],y-side[1],0),(x+22*math.cos(a),y+22*math.sin(a),h)]
    m.face(tri);m.face(tri,True)
m.save('SM_Grass')

# Hanging cloth with a folded surface and an irregular swallowtail hem.
m=Mesh()
for j in range(12):
    for i in range(8):
        def clothpoint(x,z):
            return (x,5*math.sin(x*.12+z*.035),z+(abs(x)*.28 if z<-45 else 0))
        quad=[clothpoint(-50+i*12.5,50-j*100/12),clothpoint(-50+(i+1)*12.5,50-j*100/12),clothpoint(-50+(i+1)*12.5,50-(j+1)*100/12),clothpoint(-50+i*12.5,50-(j+1)*100/12)]
        m.face(quad);m.face(quad,True)
m.save('SM_Banner')

# Chunky beveled gear; oriented like a ring in XY, with a hollow hub.
m=Mesh();n=96
for i in range(n):
    a=i*math.tau/n;b=(i+1)*math.tau/n
    ra=50 if i%8 in (2,3,4,5) else 42
    rb=50 if (i+1)%8 in (2,3,4,5) else 42
    p=[(r*math.cos(t),r*math.sin(t),z) for z in (-50,50) for r,t in [(25,a),(ra,a),(rb,b),(25,b)]]
    for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:m.face([p[k] for k in ids])
m.save('SM_Gear')

# Broken arch segment; missing stones give the arcade a less uniform silhouette.
m=Mesh()
for i in range(17):
    if i in (6,7,11):continue
    a=i*math.pi/17+.012;b=(i+1)*math.pi/17-.012
    p=[(r*math.cos(t),y,r*math.sin(t)-25) for y in (-50,50) for r,t in [(38,a),(50,a),(50,b),(38,b)]]
    for ids in [(0,1,2,3),(7,6,5,4),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]:m.face([p[k] for k in ids])
m.save('SM_BrokenArch')

# Fine-line spell circle; architectural rings intentionally retain thick rims.
m=Mesh()
for i in range(64):
    a=i*math.tau/64;b=(i+1)*math.tau/64
    p=[(r*math.cos(t),r*math.sin(t),z) for z in (-50,50) for r,t in [(49,a),(50,a),(50,b),(49,b)]]
    for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:m.face([p[k] for k in ids])
m.save('SM_Sigil')
```
<!-- END FILE -->

### 文件 Tools/build_content.py

<!-- FILE: Tools/build_content.py -->
```python
"""Editor-only asset authoring for the isolated Foldlight workspace.

Run with UnrealEditor-Cmd -run=pythonscript -script=.../build_content.py.
Writes only /Game/Foldlight assets in this project.
"""
import unreal
from pathlib import Path

ROOT = "/Game/Foldlight"
tools = unreal.AssetToolsHelpers.get_asset_tools()
unreal.EditorAssetLibrary.make_directory(ROOT + "/Art")
unreal.EditorAssetLibrary.make_directory(ROOT + "/Maps")
unreal.EditorAssetLibrary.make_directory(ROOT + "/Audio")
tasks = []
for wav in (Path(__file__).parent / "GeneratedAudio").glob("*.wav"):
    if unreal.EditorAssetLibrary.does_asset_exist(ROOT + "/Audio/" + wav.stem):
        continue
    task = unreal.AssetImportTask()
    task.filename = str(wav)
    task.destination_path = ROOT + "/Audio"
    task.automated = True
    task.save = True
    tasks.append(task)
if tasks:
    tools.import_asset_tasks(tasks)
for wav in (Path(__file__).parent / "GeneratedAudio").glob("*.wav"):
    sound = unreal.EditorAssetLibrary.load_asset(ROOT + "/Audio/" + wav.stem)
    if not sound:
        raise RuntimeError("Missing imported sound: " + wav.stem)
    sound.set_sound_asset_compression_type(unreal.SoundAssetCompressionType.PCM)
    unreal.EditorAssetLibrary.save_loaded_asset(sound)

material_path = ROOT + "/Art/M_Surface"
material = unreal.EditorAssetLibrary.load_asset(material_path) if unreal.EditorAssetLibrary.does_asset_exist(material_path) else None
if not material:
    material = tools.create_asset("M_Surface", ROOT + "/Art", unreal.Material, unreal.MaterialFactoryNew())
    color = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, -100)
    color.set_editor_property("parameter_name", "Tint")
    color.set_editor_property("default_value", unreal.LinearColor(0.2, 0.3, 0.3, 1))
    glow = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, 100)
    glow.set_editor_property("parameter_name", "Glow")
    glow.set_editor_property("default_value", unreal.LinearColor(0, 0, 0, 1))
    rough = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 300)
    rough.set_editor_property("r", 0.72)
    unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

map_path = ROOT + "/Maps/Trial"
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
    if not level.new_level(map_path):
        raise RuntimeError("Could not author the isolated trial map")
else:
    if not level.load_level(map_path):
        raise RuntimeError("Could not load the trial map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not any(isinstance(a, unreal.PlayerStart) for a in actors.get_all_level_actors()):
    start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-1100, 0, 95), unreal.Rotator(0, 0, 0))
    start.set_actor_label("Foldlight_Start")
if not level.save_current_level():
    raise RuntimeError("Could not save the trial map")
unreal.log("FOLDLIGHT_CONTENT_SUCCESS: Trial map and surface material saved")
```
<!-- END FILE -->

### 文件 Tools/import_art.py

<!-- FILE: Tools/import_art.py -->
```python
"""Import original meshes and author a world-space stone grain material."""
import unreal
from pathlib import Path

root='/Game/Foldlight/Art'
tools=unreal.AssetToolsHelpers.get_asset_tools()
for path in sorted((Path(__file__).parent/'GeneratedModels').glob('*.obj')):
    task=unreal.AssetImportTask()
    task.filename=str(path)
    task.destination_path=root+'/Models'
    task.destination_name=path.stem
    task.automated=True
    task.replace_existing=True
    task.save=True
    task.factory=unreal.FbxFactory()
    options=unreal.FbxImportUI()
    options.import_mesh=True
    options.import_materials=False
    options.import_textures=False
    options.import_as_skeletal=False
    options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True
    options.static_mesh_import_data.auto_generate_collision=False
    options.static_mesh_import_data.generate_lightmap_u_vs=False
    options.static_mesh_import_data.convert_scene=False
    task.options=options
    tools.import_asset_tasks([task])
    mesh=unreal.load_asset(root+'/Models/'+path.stem)
    if not mesh: raise RuntimeError('Model import failed: '+path.stem)
    bounds=mesh.get_bounding_box()
    unreal.log('FOLDLIGHT_MODEL '+path.stem+' bounds='+str(bounds))

mat=unreal.load_asset(root+'/M_Surface')
mat.set_editor_property('used_with_instanced_static_meshes',True)
lib=unreal.MaterialEditingLibrary
lib.delete_all_material_expressions(mat)
def node(cls,x,y,**props):
    n=lib.create_material_expression(mat,cls,x,y)
    for k,v in props.items():n.set_editor_property(k,v)
    return n
def wire(a,out,b,slot):lib.connect_material_expressions(a,out,b,slot)
tint=node(unreal.MaterialExpressionVectorParameter,-700,-200,parameter_name='Tint',default_value=unreal.LinearColor(.2,.3,.3,1))
glow=node(unreal.MaterialExpressionVectorParameter,-300,300,parameter_name='Glow')
grain=node(unreal.MaterialExpressionScalarParameter,-700,100,parameter_name='Grain',default_value=0.0)
rough=node(unreal.MaterialExpressionScalarParameter,-300,450,parameter_name='Roughness',default_value=.78)
metal=node(unreal.MaterialExpressionScalarParameter,-300,550,parameter_name='Metallic',default_value=0.0)
noise=node(unreal.MaterialExpressionNoise,-900,0,scale=.045,quality=1,levels=2,output_min=.82,output_max=1.0)
lerp=node(unreal.MaterialExpressionLinearInterpolate,-450,0,const_a=1.0)
wire(noise,'',lerp,'B');wire(grain,'',lerp,'Alpha')
mul=node(unreal.MaterialExpressionMultiply,-200,-100)
wire(tint,'',mul,'A');wire(lerp,'',mul,'B')
for n,prop in [(mul,unreal.MaterialProperty.MP_BASE_COLOR),(glow,unreal.MaterialProperty.MP_EMISSIVE_COLOR),(rough,unreal.MaterialProperty.MP_ROUGHNESS),(metal,unreal.MaterialProperty.MP_METALLIC)]:
    lib.connect_material_property(n,'',prop)
lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat)

# A separate translucent, unlit spell surface prevents dark metallic flame beads
# and avoids the opaque bloom mass of the previous large stock fire emitter.
spell=unreal.load_asset(root+'/M_Spell')
if not spell:spell=tools.create_asset('M_Spell',root,unreal.Material,unreal.MaterialFactoryNew())
spell.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
spell.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
lib.delete_all_material_expressions(spell)
emission=lib.create_material_expression(spell,unreal.MaterialExpressionVectorParameter,-200,0)
emission.set_editor_property('parameter_name','Glow')
emission.set_editor_property('default_value',unreal.LinearColor(1,.2,.025,1))
opacity=lib.create_material_expression(spell,unreal.MaterialExpressionConstant,-200,150)
opacity.set_editor_property('r',.62)
lib.connect_material_property(emission,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
lib.recompile_material(spell)
unreal.EditorAssetLibrary.save_loaded_asset(spell)
unreal.log('FOLDLIGHT_ART_SUCCESS')
```
<!-- END FILE -->

### 文件 Tools/import_chinese_font.py

<!-- FILE: Tools/import_chinese_font.py -->
```python
"""Embed the engine's CJK font as a cooked, self-contained font face.

Use full editor -ExecutePythonScript (optionally -RenderOffscreen), not
-run=pythonscript: UE font import requires an initialized Slate application.
"""
import unreal

destination = "/Game/Foldlight/Fonts"
asset_path = destination + "/DroidSansFallback"
unreal.EditorAssetLibrary.make_directory(destination)
if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
    task = unreal.AssetImportTask()
    task.filename = unreal.Paths.engine_content_dir() + "Slate/Fonts/DroidSansFallback.ttf"
    task.destination_path = destination
    task.automated = True
    task.save = True
    task.factory = unreal.FontFileImportFactory()
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
face = unreal.EditorAssetLibrary.load_asset(asset_path)
if not isinstance(face, unreal.FontFace):
    raise RuntimeError("Missing Chinese font face")
face.set_editor_property("loading_policy", unreal.FontLoadingPolicy.INLINE)
unreal.EditorAssetLibrary.save_loaded_asset(face)
unreal.log("FOLDLIGHT_CHINESE_FONT_SUCCESS: embedded CJK face saved")
```
<!-- END FILE -->

### 文件 Tools/prepare_kaykit.py

<!-- FILE: Tools/prepare_kaykit.py -->
```python
"""Prepare CC0 KayKit assets for UE without requiring an external DCC.

Keeps the original downloads intact. Retains eight gameplay clips and binds
the author's hat/cape/staff rigid geometry to their original attachment bones.
"""
import json, struct, hashlib, shutil
from pathlib import Path
import numpy as np

ROOT=Path(__file__).parent/'ExternalAssets'
OUT=ROOT/'Prepared'
OUT.mkdir(exist_ok=True)
source=next((ROOT/'KayKitAdventurers').rglob('Mage.glb'))
raw=source.read_bytes()
length=struct.unpack_from('<I',raw,12)[0]
data=json.loads(raw[20:20+length])
binary=bytearray(raw[28+length:])
nodes=data['nodes']
parents={c:i for i,n in enumerate(nodes) for c in n.get('children',[])}

def matrix(node):
    if 'matrix' in node:return np.array(node['matrix']).reshape(4,4).T
    x,y,z,w=node.get('rotation',[0,0,0,1])
    r=np.array([[1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)],
                [2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)],
                [2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)]])
    m=np.eye(4);m[:3,:3]=r@np.diag(node.get('scale',[1,1,1]));m[:3,3]=node.get('translation',[0,0,0])
    return m
def world(i):
    return (world(parents[i]) if i in parents else np.eye(4))@matrix(nodes[i])
def read_accessor(index):
    a=data['accessors'][index]; v=data['bufferViews'][a['bufferView']]
    dtype={5126:'<f4',5123:'<u2',5125:'<u4',5121:'u1'}[a['componentType']]
    width={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[a['type']]
    start=v.get('byteOffset',0)+a.get('byteOffset',0)
    return np.ndarray((a['count'],width),dtype=dtype,buffer=binary,offset=start,
                      strides=(v.get('byteStride',np.dtype(dtype).itemsize*width),np.dtype(dtype).itemsize)).copy()
def append(array,kind,component=5126):
    while len(binary)%4:binary.append(0)
    offset=len(binary);blob=array.astype('<f4' if component==5126 else '<u2').tobytes();binary.extend(blob)
    vi=len(data['bufferViews']);data['bufferViews'].append({'buffer':0,'byteOffset':offset,'byteLength':len(blob)})
    ai=len(data['accessors']);a={'bufferView':vi,'componentType':component,'count':len(array),'type':kind}
    if kind=='VEC3':a.update(min=array.min(axis=0).tolist(),max=array.max(axis=0).tolist())
    data['accessors'].append(a);return ai

rig=53
for node_id,bone_id in [(14,15),(16,17),(8,9)]:
    n=nodes[node_id];transform=np.linalg.inv(world(rig))@world(node_id)
    joint=data['skins'][0]['joints'].index(bone_id)
    for primitive in data['meshes'][n['mesh']]['primitives']:
        at=primitive['attributes'];positions=read_accessor(at['POSITION'])
        positions=(transform@np.c_[positions,np.ones(len(positions))].T).T[:,:3]
        at['POSITION']=append(positions,'VEC3')
        if 'NORMAL' in at:
            normals=(np.linalg.inv(transform[:3,:3]).T@read_accessor(at['NORMAL']).T).T
            normals/=np.maximum(np.linalg.norm(normals,axis=1)[:,None],1e-8)
            at['NORMAL']=append(normals,'VEC3')
        at.pop('TANGENT',None)
        joints=np.zeros((len(positions),4));joints[:,0]=joint
        weights=np.zeros((len(positions),4));weights[:,0]=1
        at['JOINTS_0']=append(joints,'VEC4',5123);at['WEIGHTS_0']=append(weights,'VEC4')
    nodes[parents[node_id]]['children'].remove(node_id)
    nodes[rig]['children'].append(node_id)
    for key in ('translation','rotation','scale','matrix'):n.pop(key,None)
    n['skin']=0
for i in (0,1,7):nodes[i].pop('mesh',None)
keep={'Idle','Walking_A','Running_A','Jump_Start','Jump_Idle','Jump_Land','Spellcast_Shoot','Interact'}
data['animations']=[a for a in data['animations'] if a['name'] in keep]
for a in data['animations']:
    a['channels']=[c for c in a['channels'] if not(c['target']['node']==46 and c['target']['path']=='translation')]
data['buffers'][0]['byteLength']=len(binary)
blob=json.dumps(data,separators=(',',':')).encode('utf8')
blob+=b' '*((-len(blob))%4);binary+=b'\0'*((-len(binary))%4)
output=struct.pack('<III',0x46546c67,2,28+len(blob)+len(binary))+struct.pack('<II',len(blob),0x4e4f534a)+blob+struct.pack('<II',len(binary),0x004e4942)+binary
(OUT/'Mage.glb').write_bytes(output)
print('Prepared mage:',len(data['animations']),'clips; hat/cape/staff bound to author bones')
manifest=[]
for name in ('KayKitAdventurers','KayKitDungeon'):
    archive=ROOT/(name+'.zip')
    manifest.append({'pack':name,'sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'bytes':archive.stat().st_size})
    license_file=next((ROOT/name).rglob('LICENSE.txt'))
    shutil.copyfile(license_file,OUT/(name+'-CC0.txt'))
(OUT/'downloads.json').write_text(json.dumps(manifest,indent=2),encoding='utf8')
```
<!-- END FILE -->

### 文件 Tools/import_kaykit.py

<!-- FILE: Tools/import_kaykit.py -->
```python
"""Import selected author assets through UE's glTF Interchange pipeline."""
from pathlib import Path
import unreal, json
root=Path(__file__).parent/'ExternalAssets'
tools=unreal.AssetToolsHelpers.get_asset_tools()
target='/Game/Foldlight/External/KayKit'
def import_file(source,destination):
    task=unreal.AssetImportTask()
    task.filename=str(source);task.destination_path=destination
    task.automated=True;task.save=True;task.replace_existing=True
    tools.import_asset_tasks([task])
    return task.imported_object_paths

mage_path=target+'/Mage/Mage/SkeletalMeshes/Mage'
if not unreal.EditorAssetLibrary.does_asset_exist(mage_path):
    import_file(root/'Prepared/Mage.glb',target+'/Mage')
pack=next((root/'KayKitDungeon').glob('*/addons/kaykit_dungeon_remastered'))
names=['pillar_decorated','wall_arched','wall_broken','torch_mounted',
       'shelf_small_candles','table_small_decorated_A','barrel_large_decorated',
       'barrel_small_stack','candle_triple','rubble_large','wall_archedwindow_open','shelves']
for name in names:
    final=target+'/Environment/SM_KK_'+name
    if unreal.EditorAssetLibrary.does_asset_exist(final):continue
    source=next((pack/'Assets/gltf').glob(name+'.*.glb'))
    import_file(source,target+'/Imports/'+name)
    meshes=[unreal.load_asset(p) for p in unreal.EditorAssetLibrary.list_assets(target+'/Imports/'+name,recursive=True,include_folder=False)]
    meshes=[m for m in meshes if isinstance(m,unreal.StaticMesh)]
    assert len(meshes)==1,(name,len(meshes))
    assert unreal.EditorAssetLibrary.rename_asset(meshes[0].get_path_name(),final)

# Keep the author's palette textures, with a small ambient fill for dark puzzle rooms.
adventurers=next((root/'KayKitAdventurers').glob('*/addons/kaykit_character_pack_adventures'))
for label,source in [('Mage',adventurers/'Textures/mage_texture.png'),('Dungeon',pack/'Assets/texture/dungeon_texture.png')]:
    texture_path=target+'/Textures/'+source.stem
    if not unreal.EditorAssetLibrary.does_asset_exist(texture_path):import_file(source,target+'/Textures')
    texture=unreal.load_asset(texture_path)
    assert texture,texture_path
    material_path=target+'/M_KK_'+label
    material=unreal.load_asset(material_path) if unreal.EditorAssetLibrary.does_asset_exist(material_path) else tools.create_asset('M_KK_'+label,target,unreal.Material,unreal.MaterialFactoryNew())
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    material.set_editor_property('used_with_instanced_static_meshes',True)
    material.set_editor_property('used_with_skeletal_mesh',True)
    tex=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionTextureSample,-500,0)
    tex.texture=texture
    fill=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionMultiply,-200,140)
    fill.set_editor_property('const_b',.20 if label=='Mage' else .10)
    rough=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant,-200,300)
    rough.set_editor_property('r',.85)
    unreal.MaterialEditingLibrary.connect_material_property(tex,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_expressions(tex,'RGB',fill,'A')
    unreal.MaterialEditingLibrary.connect_material_property(fill,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    if label=='Mage':
        mesh=unreal.load_asset(mage_path)
        slots=mesh.get_editor_property('materials')
        for slot in slots:slot.material_interface=material
        mesh.set_editor_property('materials',slots)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    else:
        for name in names:
            mesh=unreal.load_asset(target+'/Environment/SM_KK_'+name)
            for i in range(len(mesh.get_editor_property('static_materials'))):mesh.set_material(i,material)
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
records=[]
for path in unreal.EditorAssetLibrary.list_assets(target,recursive=True,include_folder=False):
    obj=unreal.load_asset(path)
    if obj is None:continue
    record={'path':path,'class':obj.get_class().get_name()}
    if isinstance(obj,unreal.SkeletalMesh):
        record['bounds']=str(obj.get_bounds())
        record['skeleton']=obj.get_editor_property('skeleton').get_path_name()
    if isinstance(obj,unreal.AnimSequence):record['length']=obj.get_play_length()
    if isinstance(obj,unreal.StaticMesh):record['bounds']=str(obj.get_bounds())
    unreal.log('KAYKIT_ASSET '+str(record));records.append(record)
(Path(__file__).parent/'ExternalAssets/Prepared/ue_assets.json').write_text(json.dumps(records,indent=2),encoding='utf8')
unreal.log('KAYKIT_IMPORT_COMPLETE')
```
<!-- END FILE -->

### 文件 Tools/Build_Editor.bat

<!-- FILE: Tools/Build_Editor.bat -->
```bat
@echo off
call "E:\UE5\UE_5.7\Engine\Build\BatchFiles\Build.bat" FoldlightDemoEditor Win64 Development "-Project=%~dp0..\FoldlightDemo.uproject" -WaitMutex -NoHotReloadFromIDE
exit /b %errorlevel%
```
<!-- END FILE -->

### 文件 Tools/Package_Windows.bat

<!-- FILE: Tools/Package_Windows.bat -->
```bat
@echo off
call "E:\UE5\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=%~dp0..\FoldlightDemo.uproject" -target=FoldlightDemo -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive "-archivedirectory=%~dp0..\Build" -utf8output -unattended
if errorlevel 1 exit /b %errorlevel%
if not exist "%~dp0..\Build\Windows\Licenses" mkdir "%~dp0..\Build\Windows\Licenses"
copy /Y "%~dp0..\Documentation\Fonts\AndroidOpenSourceProject_License.txt" "%~dp0..\Build\Windows\Licenses\DroidSansFallback.txt" >nul
copy /Y "%~dp0ExternalAssets\Prepared\KayKitAdventurers-CC0.txt" "%~dp0..\Build\Windows\Licenses\KayKitAdventurers-CC0.txt" >nul
copy /Y "%~dp0ExternalAssets\Prepared\KayKitDungeon-CC0.txt" "%~dp0..\Build\Windows\Licenses\KayKitDungeon-CC0.txt" >nul
exit /b %errorlevel%
```
<!-- END FILE -->

### 文件 Tools/Validate_Playtest.ps1

<!-- FILE: Tools/Validate_Playtest.ps1 -->
```powershell
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
```
<!-- END FILE -->

### 文件 Tools/Capture_Art.ps1

<!-- FILE: Tools/Capture_Art.ps1 -->
```powershell
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
```
<!-- END FILE -->

### 文件 Launch_Playtest.bat

<!-- FILE: Launch_Playtest.bat -->
```bat
@echo off
setlocal
set "FOLDLIGHT_ROOT=%~dp0"
if exist "%FOLDLIGHT_ROOT%Build\Windows\FoldlightDemo.exe" (
    start "Foldlight" "%FOLDLIGHT_ROOT%Build\Windows\FoldlightDemo.exe" -windowed -ResX=1280 -ResY=720
) else (
    start "Foldlight" "E:\UE5\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "%FOLDLIGHT_ROOT%FoldlightDemo.uproject" -game -windowed -ResX=1280 -ResY=720 -NoSplash
)
```
<!-- END FILE -->

### 文件 Documentation/Fonts/AndroidOpenSourceProject_License.txt

<!-- FILE: Documentation/Fonts/AndroidOpenSourceProject_License.txt -->
```text
   Copyright (c) 2005-2008, The Android Open Source Project
   Licensed under the Apache License, Version 2.0 (the "License"); you may not
   use this file except in compliance with the License.
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
   License for the specific language governing permissions and limitations under
   the License.

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright [yyyy] [name of copyright owner]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```
<!-- END FILE -->

### 文件 Tools/ExternalAssets/Prepared/KayKitAdventurers-CC0.txt

<!-- FILE: Tools/ExternalAssets/Prepared/KayKitAdventurers-CC0.txt -->
```text

	KayKit : Adventurers Character Pack (1.0)

	Created/distributed by Kay Lousberg (www.kaylousberg.com)
	Creation date: 13/03/2023 09:00

			------------------------------

	License: (Creative Commons Zero, CC0)
	http://creativecommons.org/publicdomain/zero/1.0/

	This content is free to use in personal, educational and commercial projects.

	Support me by using a brand resource provided in this pack or by crediting Kay Lousberg, www.kaylousberg.com (this is not mandatory)

			------------------------------

    This asset pack is here thanks to all the wonderful people who support KayKit on Patreon and those who buy our EXTRA or SOURCE packs on itch.io.

	And a big, special thank you to my Super Supporters:

	- Brian McBarron
	- Silva



			------------------------------


	Patreon:  http://patreon.com/kaylousberg

	Follow on Twitter for updates:
	http://twitter.com/KayLousberg
```
<!-- END FILE -->

### 文件 Tools/ExternalAssets/Prepared/KayKitDungeon-CC0.txt

<!-- FILE: Tools/ExternalAssets/Prepared/KayKitDungeon-CC0.txt -->
```text

	KayKit : Dungeon Remastered (1.0)

	Created/distributed by Kay Lousberg (www.kaylousberg.com)
	Creation date: 13/03/2023 09:00

			------------------------------

	License: (Creative Commons Zero, CC0)
	http://creativecommons.org/publicdomain/zero/1.0/

	This content is free to use in personal, educational and commercial projects.

	Support me by crediting Kay Lousberg, www.kaylousberg.com (this is not mandatory)

			------------------------------

	This asset pack is here thanks to all the wonderful people who support KayKit on Patreon and those who buy our EXTRA or SOURCE packs on itch.io.

	And a big, special thank you to my Super Supporters:

	- Brian McBarron
	- Silva



			------------------------------


	Patreon:  http://patreon.com/kaylousberg

	Follow on Twitter for updates:
	http://twitter.com/KayLousberg
```
<!-- END FILE -->

