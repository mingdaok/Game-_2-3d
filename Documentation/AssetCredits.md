# 美术与音效来源说明

本项目使用第三方素材、原工程已有资源，以及由项目脚本生成和调整的内容，**并非全部美术自创**。模型转换、材质调整、动画接入和场景布置不改变原素材作者归属。

## 已确认的第三方素材

| 内容 | 原作者与来源 | 项目位置 | 许可与改动 |
| --- | --- | --- | --- |
| 法师角色、骨骼、帽子、披风、法杖、角色动画和图集 | Kay Lousberg / [KayKit Adventurers 1.0](https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0) | `Content/Foldlight/External/KayKit/Mage`、`Imports` 等；原文件在 `Tools/ExternalAssets/KayKitAdventurers` | 随包许可为 CC0。项目做了格式转换、配件蒙皮、动画筛选和 UE 动画图接入；角色与原始动作不是本项目自创。 |
| 地牢石柱、墙体、拱窗、壁炬、木桶、蜡烛、陈设、碎石及图集 | Kay Lousberg / [KayKit Dungeon Remastered 1.0](https://github.com/KayKit-Game-Assets/KayKit-Dungeon-Remastered-1.0) | `Content/Foldlight/External/KayKit/Environment`、`Textures` 等；原文件在 `Tools/ExternalAssets/KayKitDungeon` | 随包许可为 CC0。项目导入了选定模型、调整材质并组合场景；这些原模型和贴图不是本项目自创。 |
| 中文字体 DroidSansFallback | Android Open Source Project；从本机 UE 的 `Engine/Content/Slate/Fonts/DroidSansFallback.ttf` 导入 | `Content/Foldlight/Fonts`；导入脚本 `Tools/import_chinese_font.py` | 随项目保留 AOSP 的 Apache 2.0 许可文本；字体不是本项目自创。 |
| 基础几何体、基础材质、引擎运行资源 | Unreal Engine / Epic Games | 代码引用 `/Engine/BasicShapes`、`/Engine/EngineMaterials` 等 | UE 内置内容，不能标为本项目自创，也不因使用 KayKit 而成为 CC0。 |

KayKit 两份许可原文位于 `Tools/ExternalAssets/Prepared/KayKitAdventurers-CC0.txt` 和 `KayKitDungeon-CC0.txt`；字体许可位于 `Documentation/Fonts/AndroidOpenSourceProject_License.txt`。试玩 ZIP 的 `Licenses` 文件夹保留这三份许可。KayKit 的原始包许可不等于整个仓库采用同一许可。

## 本项目脚本生成或实现的内容

以下内容具有本地生成脚本或实现代码。这里标为“项目脚本生成”，不将其表述为全部由人工手工制作；本次开发有 AI 辅助。

| 内容 | 证据与位置 | 范围 |
| --- | --- | --- |
| 程序几何模型，如石台、拱券、圆环、底座、晶体、旧版蜡烛、藤根等 | `Tools/make_models.py`、`Tools/GeneratedModels`、`Content/Foldlight/Art/Models` | 与 KayKit 模型分开；仅这些生成模型属于此类。 |
| 项目表面材质与法术材质 | `Tools/import_art.py`、`Content/Foldlight/Art/M_Surface`、`M_Spell` | 材质脚本生成；使用材质的第三方模型仍保留其原作者归属。 |
| 升级后的施法火环和火墙 | `Tools/build_fire_vfx.py`、`Content/Foldlight/Art/M_FireFlow`、`Source/NEWMyProject/FDWorld.cpp` | 程序遮罩、色彩分层、火星、轻烟、排列与动画由本次项目实现；承载火焰的平面来自 UE。不能据此称所有燃烧特效均为自创。 |
| 三段合成音效 S_Fold、S_Fire、S_Restore | `Tools/make_audio.py`、`Tools/GeneratedAudio`、`Content/Foldlight/Audio` | 通过数学波形和噪声合成；不包括第三方火焰资源包内的音频。 |
| 五关布局与场景组合 | `Source/NEWMyProject/FDWorld.cpp`、`FDArt.cpp` 等 | 项目布局和组合代码；组合后的场景仍含上述第三方素材。 |

AI 生成的效果预测图只作为设计参考，不能当作真实游戏截图；`Documentation/FireUpgrade` 保存的是引擎实机截图，截图中仍包含第三方角色与场景模型。

## 原工程继承资源与待核实项目

以下文件随原工程保留。当前资料不足以确认全部原作者、下载页面和具体许可，**不标为原创，不推定为 CC0，也不声称已完成再分发许可核验**。

| 目录或资源 | 当前可确认的情况 |
| --- | --- |
| `Content/Vefects/Free_Fire` | 原工程已有的 Niagara 火焰资源包。`FDObject.cpp` 的藤蔓燃烧仍调用 `NS_Fire_Big_Simple`，且打包配置包含此目录。它与新增的 `M_FireFlow` 施法火焰不是同一套实现。原作者与具体许可待补充。 |
| `Content/SciFITrooper_Man_03` | 原工程已有科幻角色资源；不是当前默认 KayKit 法师。作者、来源和许可待补充，不能仅凭目录名判定。 |
| `Content/Fab/Megascans` | 原工程已有资源，目录标识为 Fab / Megascans。具体资源页面、获取条件与适用许可待补充；不作统一免费或开放许可声明。 |
| 其他原工程蓝图、地图、材质与资源 | 与 Foldlight 新增内容分开登记，未逐项确认的条目标为“原工程继承，待核实”。不能根据文件名推定作者。 |

完整的 Content 文件定位清单见 [AssetSources.csv](AssetSources.csv)。清单按目录和可追溯脚本分类，适合定位来源组；不是逐个二进制资源的独立版权鉴定。继承内容的准确作者与许可证需要结合原始购买、下载或授权记录补全。

## 对外介绍用语

“折光遗迹是一款使用 Unreal Engine 开发的维度切换解谜试玩。法师角色、动画和部分地牢模型使用 Kay Lousberg 的 KayKit 素材；部分几何、材质、火焰动画与合成音效由项目脚本生成，开发过程中使用 AI 辅助。项目另保留部分原工程资源，其来源与许可核实状态见本说明。”
