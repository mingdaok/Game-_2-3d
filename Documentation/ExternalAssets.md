# 0.5 外部美术与骨骼动画

## 来源与许可

本节记录新增使用的 Kay Lousberg / KayKit 官方仓库免费版本，两个包均附 CC0 1.0 Universal 许可。这里的来源与许可结论仅覆盖这两个 KayKit 包，不覆盖原工程继承的 Free_Fire、SciFITrooper、Fab / Megascans 等内容。完整分类和待核实项见 [美术与音效来源说明](AssetCredits.md)。

- [KayKit Character Pack: Adventurers 1.0](https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0)：Mage 模型、骨骼、帽子、披风、法杖、动画和原始渐变色图集。
- [KayKit Dungeon Remastered 1.0](https://github.com/KayKit-Game-Assets/KayKit-Dungeon-Remastered-1.0)：雕花石柱、拱墙、破墙、拱窗、壁炬、蜡烛架、装饰桌、木桶、桶堆、三联蜡烛、碎石和置物架，共 12 种环境模型及原始图集。

下载日期：2026-09-20。下载归档保存在 `Tools/ExternalAssets`，不作为游戏资源打包。

| 下载包 | ZIP 字节数 | SHA-256 |
| --- | ---: | --- |
| KayKitAdventurers.zip | 6141833 | e19176d799036452bc783c00dd1036e158c66c5a43b7285ccbfea3466d6f57bf |
| KayKitDungeon.zip | 6312130 | 8e92aa460f5be4252dff0cf030f92846ab74b875ce4756076ecce75b5d0c9bee |

两份完整许可证保存在 `Tools/ExternalAssets/Prepared/*-CC0.txt`，同时随 Windows 试玩版放入 `Build/Windows/Licenses`。

## 实际接入范围

默认主角切换为骨骼法师。帽子随头部、披风随胸部、法杖随右手骨骼运动。保留旧蜡烛角色代码与资源，启动参数 `-CandleHero` 可用于外观对比。身体碰撞仍只由原有角色胶囊体负责。

运行时使用 7 段动作：Idle、Walking_A、Running_A、Jump_Start、Jump_Idle、Jump_Land、Spellcast_Shoot。步行与跑步同步相位并按速度混合；起跳、空中和落地根据真实运动状态衔接；施法从 spine 骨骼向上叠加，保留腿部移动。暂停时冻结骨骼动画。额外导入 Interact 动作作为后续扩展资源，当前未接入交互。

`FDMageAnimInstance` 是原生动画图，使用 UE Sequence Player、Two Way Blend、Layered Bone Blend 节点。动画不驱动角色位移，仍由原来的移动组件控制，避免破坏二维约束和跨维度落点。

环境保留原有机关、平台与关卡配色；专业包模型替换近景石柱并补充后景墙体和陈设。重复模型按网格实例化，装饰没有游戏碰撞。布景随当前房间及投影方向调整，避免前景装饰挡住四向解谜路线。

## 转换与材质

`Tools/prepare_kaykit.py` 基于原始 GLB 生成 UE 专用副本，保留源文件：将帽子、披风和法杖转换为绑定作者骨骼的刚性蒙皮；隐藏未使用的书本、魔杖变体；筛选 8 段动画，移除根骨骼平移轨迹。

`Tools/import_kaykit.py` 通过 UE Interchange 导入，输出资源清单 `Tools/ExternalAssets/Prepared/ue_assets.json`。应使用完整 UE 编辑器的 `-ExecutePythonScript` 模式，避免 commandlet 保存资源时缺少 Slate。

游戏材质保留作者原始贴图，使用较高粗糙度和少量环境自发光补偿暗场景。材质支持骨骼网格与实例化网格。外部素材位于 `Content/Foldlight/External/KayKit`。

编辑器可能自动把 DefaultInput.ini 改为 EnhancedInput 默认类；导入完成后需保持 Engine.PlayerInput / Engine.InputComponent，现有试玩输入使用这两个类。

## 验证结果

Windows 0.5.0 打包成功，101 项机制与资源检查通过；实际物理移动自动通关五关，坠落 0 次。13 张实机截图覆盖五关、四向塔视角和走跑跳施法。完整证据见 `Validation.md`；画面见 `ExternalArt`。

## 范围边界

这一版使用现成的低多边形专业资源，仍是五关试玩；没有更换核心谜题，也没有加入战斗、布料物理或动态脚底 IK。法杖是随骨骼运动的模型，火印仍按原有地面判定位置生成。
