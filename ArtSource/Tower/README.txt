四向折光塔 — Blender 可编辑结构模型

打开 Foldlight_Tower.blend（Blender 4.5 LTS）。
Foldlight_Tower.glb 为第一帧静态模型，保留材质，便于其他软件导入。

布局：从当前 FDWorld.cpp 解析起点、移动石柱和八块高架平台。
尺寸：1 Blender 单位 = 1 米；UE 房间局部厘米坐标除以 100。
Blender 与 UE 的坐标手性不同，回导 UE 时需核对轴向。
这是独立建模文件，尚未替换 UE 关卡资产或接入游戏逻辑。

集合：
01_Playable_Platforms：命名对应 UE 对象的石台主体。
02_Stone_and_Brass_Details：石台面层、拼缝、铜饰边、石柱轨道。
03_Seals_and_Core：四向印记与终点星环核心。
04_Layout_Guides：石柱终点线框，不参与渲染或 GLB 导出。
00_Presentation：相机、灯光。

石柱在时间轴第 1–90 帧从起点移至目标位置，装饰随主体移动。
相机 Overview / Front / Side / Top；对应同名 PNG 预览。
平台顶面由独立石板构成，主体上缘为面层让出 0.18 米，组合高度遵循源代码。
装饰为根据源代码重建的简化模型，未包含背景拱廊、植被、角色和火焰特效。

重建：
blender --background --python-exit-code 1 --python ../../Tools/build_tower_blender.py
