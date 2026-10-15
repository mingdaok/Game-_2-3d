# 折光遗迹：四向折叠中文试玩（火焰升级版）

**素材声明：本项目并非全部美术自创。** 法师角色、动画和部分地牢模型来自 Kay Lousberg 的 KayKit；字体和引擎基础资源也属于第三方。项目脚本生成的内容，以及原工程继承、来源待核实的资源，分别列在 [美术与音效来源说明](Documentation/AssetCredits.md) 和 [逐文件来源清单](Documentation/AssetSources.csv) 中。第三方素材的转换、调色和场景组合不改变原作者归属。

本版接入 KayKit CC0 冒险者角色与地牢美术包。默认主角为带帽子、披风、法杖的骨骼法师，支持待机、步行、跑步、起跳、空中、落地与上半身施法动画；五个房间使用 12 种地牢环境模型。素材来源、修改方式和许可见 `Documentation/ExternalAssets.md`。

最新火焰实机截图位于 [Documentation/FireUpgrade](Documentation/FireUpgrade)，角色与场景截图位于 [Documentation/ExternalArt](Documentation/ExternalArt)。可使用命令行参数 `-CandleHero` 查看保留的程序几何蜡烛角色外观。

当前版本的施法火环和火墙使用项目实现的 `M_FireFlow` 程序材质，加入分层火舌、火星、轻烟、暖光与渐隐。藤蔓燃烧仍调用原工程已有的 `Vefects/Free_Fire` Niagara 素材，两套效果的来源不同。

早期美术迭代记录见 [Documentation/PolishNotes.md](Documentation/PolishNotes.md) 和 [Documentation/ArtDirection.md](Documentation/ArtDirection.md)；素材归属以当前 [来源说明](Documentation/AssetCredits.md) 为准。

场景同时使用项目脚本生成的拱券、石台、圆环、晶体等几何，以及 KayKit 的地牢模型。默认法师模型和原始动作属于 KayKit，不属于项目自创；项目负责导入、材质调整、动画接入和场景组合，开发过程中使用 AI 辅助。

## 下载与打开

试玩请下载 [Windows Release](https://github.com/mingdaok/Game-_2-3d/releases/tag/v0.5.1-fire) 中的 `Foldlight_Playtest_2026-10-15.zip`，完整解压后双击 `Start_Game.bat`，无需安装 UE 编辑器。包内 `Licenses` 附有素材来源说明和已收集的许可证。

本仓库是独立 UE 5.7 C++ 工程，资源使用 Git LFS。安装 Git LFS 后克隆仓库并执行 `git lfs pull`，再打开 `FoldlightDemo.uproject`；编译需要相应的 Visual Studio C++ 工具。仓库不包含生成缓存和 Windows 试玩二进制。

工程内的 `Launch_Playtest.bat` 优先运行本地 `Build/Windows` 试玩包，否则尝试启动 UE。该脚本和构建脚本中的 UE 安装路径需要按开发者本机位置调整。

## 操作

| 按键 | 功能 |
| --- | --- |
| Enter | 开始／继续；结束后重新开始 |
| N | 标题页开始新旅程 |
| WASD、鼠标 | 3D移动和观察；2D使用A/D左右移动 |
| Space | 跳跃，支持短暂跳跃输入缓冲和离台容错 |
| 按住V，松开V | 查看切换检查提示，松开后切换维度 |
| Q / F | 在三维或折叠预览中切换南、东、北、西视角；二维中需先展开 |
| G | 第三房间起施放火魔法；新法术替换旧法术 |
| E | 在3D中移动附近轨道石柱，或激活出口 |
| R | 重置当前房间 |
| H | 逐级提示，可按三次 |
| Esc | 暂停／继续 |
| F10 | 退出 |

## 五个房间

1. **初次折叠**：V压缩前后错开的岛屿，横向跳到终点；在终点平台恢复3D，E离开。
2. **错位之桥**：在3D起点附近找到带金色标记的轨道石柱，E移动到终点，然后切换2D跨越。
3. **余烬花园**：面朝藤蔓，用G在前方约2.3米放置火印；等待1.5秒燃烧结束，走到出口。
4. **双生熔炉**：在3D里把一个火印放在两个低位接收器中间；机关激活后用2D火墙烧高处藤蔓，再前往出口。
5. **四向折光塔**：先按E移动起点石柱，以南视折叠，沿阶梯到第一祭台；朝屏幕右侧按G点亮南视印记。展开三维，Q/F选择西视，再折叠并沿新阶梯前进。依次完成南视→西视→北视→东视四段路线，在每个祭台用同方向的二维火墙点亮印记。最高祭台展开为三维，按E修复核心。

## 本版规则

- 二维支持南、东、北、西四个固定视角，以右上角的选中方向为准。南/北视压缩Y轴深度，东/西视压缩X轴深度；A/D始终对应屏幕左/右。相对的两个方向在几何上互为镜像，四向祭台通过对应方向的魔法印记区分它们。
- Q/F只选择视角，不传送角色、不改变碰撞。按住V时也能换方向比较轮廓，松开才提交转换；在二维中需要先展开回脚下平台的实际位置，再换方向。
- 必须站稳后切换；移动中的石柱、被占用的目标位置会阻止转换。
- 平台投影保存来源身份。返回3D使用实际脚下平台的真实深度。
- 3D火印半径2米、低矮；2D火墙宽1.2米、高4.4米。持续4秒，跨维度不刷新寿命。
- 2D火焰按当前房间的投影范围命中，忽略目标原始深度；其他房间不受影响。
- 每个房间的完成进度自动保存。跌落或R重置当前房间；本版不保存房间内部的临时状态。
- 四向印记只接受对应方向的二维火墙；三维火印或错误方向不会点亮。已点亮的印记在本关内保持，R或坠落会重置。
- 本版没有冰、光、战斗或任意角度投影；这些属于后续内容。

## 开发与验证

- 入口：`AFDGameMode`，地图 `/Game/Foldlight/Maps/Trial`。
- 关卡在 `AFDWorld::BuildWorld` 中以确定性方式生成；打开地图后需开始游玩，才会看到完整房间。
- `AFDObject` 管理来源碰撞、投影碰撞、燃烧、机关和轨道移动。
- `AFDCharacter` 负责输入、角色、镜头与切换请求。
- `AFDHUD` 提供标题、玩法提示、暂停和结算界面。
- 旧C++和蓝图保留于副本中作参考；试玩的启动流程使用新的FD类。
- `Tools/make_audio.py` 通过波形和噪声合成 `S_Fold`、`S_Fire`、`S_Restore` 三段音效，此说明不覆盖第三方资源包内的音频。`Tools/build_content.py` 通过UE Python生成试玩专用材质、导入音效并创建入口地图，仅写入 `/Game/Foldlight`；命令行运行时需附加 `-AllowCommandletAudio`。
- `Tools/build_fire_vfx.py` 生成升级后的施法火焰材质；`FDWorld.cpp` 控制火环、火墙、火星和轻烟的排列与动画。
- `Tools/Build_Editor.bat` 编译编辑器模块；`Tools/Package_Windows.bat` 打包Windows。
- `-FoldlightSmoke` 启动自动场景功能验证，日志搜索 `FOLDLIGHT_TEST`。该验证不写玩家存档。
- `-FoldlightTraversal` 通过真实角色移动与跳跃物理、施法和交互走完五关；不同于机制测试中的场景定位，它不传送玩家跨越谜题。
- `-FoldlightCapture -CaptureRoom=0` 自动截图；附加 `-Capture2D` 截取2D状态。该模式不写玩家存档。
- 附加 `-CaptureView=0/1/2/3` 分别检查南/东/北/西视角，第五关截图会定位到对应祭台。四向设计详见 `Documentation/FourDirections.md`。

## 美术与音效归属

| 类型 | 内容与归属 |
| --- | --- |
| 第三方角色与场景 | 法师角色、骨骼、配件、原始动画，以及部分地牢模型和图集来自 Kay Lousberg 的 KayKit，所用两个包随附 CC0 许可。格式转换、调色和组合不改变原作者归属。 |
| 项目脚本生成 | `Tools/make_models.py` 生成的部分几何、项目表面材质、升级后的施法火焰及三段合成音效具有对应脚本；开发有 AI 辅助，不表述为全部人工手工制作。 |
| 原工程继承 | `Vefects/Free_Fire`、`SciFITrooper_Man_03`、`Fab/Megascans` 等保留资源不标为原创，其原作者、具体来源或许可仍有待核实项；藤蔓燃烧仍实际使用 Free_Fire。 |
| 字体与引擎资源 | DroidSansFallback 字体来自 Android Open Source Project，经 UE 导入；基础几何体与引擎资源来自 Unreal Engine，不属于项目自创。 |

完整来源、改动范围和许可证位置见 [AssetCredits.md](Documentation/AssetCredits.md)，逐文件定位见 [AssetSources.csv](Documentation/AssetSources.csv)。KayKit 的 CC0 许可仅适用于相应素材，不代表整个工程或全部继承资源采用 CC0；来源标注也不等于已核实所有继承资源的再分发许可。

## 中文界面与当前范围

游戏内菜单、关卡名称、目标、操作说明、机关反馈、三级提示、暂停、结算和窗口标题均为简体中文，保留实际键帽字母。中文字体随游戏包内嵌，无需安装系统字体。

字体使用引擎随附的 Droid Sans Fallback，许可见 `Documentation/Fonts/AndroidOpenSourceProject_License.txt`。`Tools/import_chinese_font.py` 可通过编辑器的 `-ExecutePythonScript` 重新导入字体；不要使用缺少 Slate 初始化的 Python 命令行工具模式。

这是机制试玩，使用程序搭建的场景；尚未完成正式关卡美术、手柄支持或用户可调设置。
