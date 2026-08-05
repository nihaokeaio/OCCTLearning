# OcctImgui 学习仓库：分支成果与简历素材档案

> 整理日期：2026-08-04  
> 分析范围：全部本地分支、对应远程跟踪分支、提交历史、分支差异及已有文档  
> 基准分支：`main`（与 `develop` 当前均指向 `c26d84d`）

## 1. 项目定位

该仓库以一个 OpenCASCADE + GLFW + Dear ImGui 示例程序为起点，逐步扩展为个人 C++/CAD 图形开发学习与原型验证仓库。新增内容覆盖二维/三维几何、交互拾取与捕捉、AIS 可视化、Shader 与多纹理、渲染结果读取、智能指针、依赖图、现代 C++、信号槽和对象生命周期管理。

这个项目适合在简历中表述为“个人学习与技术原型项目”，重点展示持续迭代、底层机制理解、工程抽象和复杂生命周期问题的处理。除非后续补充了生产环境证据，不应包装为已上线产品。

## 2. 分支关系总览

| 分支 | 时间 | 相对 `main` 的提交数 | 核心主题 | 当前判断 |
|---|---:|---:|---|---|
| `feature` | 2025-08 至 2025-09 | 3 | 几何特征提取、拾取、吸附、BVH | 大型实验原型 |
| `clipper` | 2025-08 | 1 | Clipper2 二维轮廓偏移 | 功能原型 |
| `BVHTest` | 2025-09 | 1 | OCCT BVH 索引几何管理 | 明确未编译完成 |
| `learnAis` | 2025-09 至 2025-12 | 3 | AIS 自定义对象、Owner、高亮、裁剪 Shader | 学习/功能原型 |
| `UseTexture` | 2026-01 | 1 | OpenGL 多纹理与 Shader | 学习实验 |
| `UseGeometry` | 2026-01 | 3 | OCCT 曲线、边、线框、面、曲面、实体 | 系统学习实验 |
| `Handle` | 2026-03 | 2 | 自定义弱句柄与控制块 | 内存模型实验 |
| `renderCut` | 2026-05 | 1 | 深度缓冲读取与可见性判断 | 功能原型 |
| `modernCpp` | 2026-05 | 1 | 可变参数模板、`if constexpr`、结构化绑定 | 语法学习实验 |
| `DependencyGraph` | 2026-05 至 2026-06 | 11 | 属性依赖图、求值、环检测、拓扑排序 | 较完整独立模块 |
| `SignalSlotFeat` | 2026-04 至 2026-07 | 8 | 单线程信号槽机制 | 较完整独立模块 |
| `ObjectTree` | 2026-07 | 2 | 对象树、所有权、生命周期观察 | 较完整模块且有文档 |
| `ObjectRuntime` | 2026-07 | 12 | 合并对象树与信号槽 | 集成成果 |
| `MetaObject` | 2026-07 | 12 | 与 `ObjectRuntime` 指向同一提交 | 当前工作分支 |

关系说明：

- `main` 与 `develop` 没有差异，主要保留上游示例基线。
- `feature`、`clipper`、`learnAis`、`UseTexture`、`UseGeometry`、`Handle`、`renderCut`、`modernCpp`、`DependencyGraph`、`SignalSlotFeat` 基本都是从共同基线独立演进的实验线。
- `ObjectTree` 从基线实现对象树并补充专项文档。
- `ObjectRuntime` 合并 `SignalSlotFeat`，再加入并调整对象树，形成二者的集成版本。
- `MetaObject` 与 `ObjectRuntime` 当前同为 `381c30d`；名称不同但已提交内容完全相同。
- 除 `learnAis` 本地比 `origin-zqd/learnAis` 多 1 个提交外，存在对应远程跟踪分支的其他分支均已同步。

## 3. 各分支成果详解

### 3.1 `feature`：三维拾取、特征提取与吸附系统

这是代码增量最大的早期分支之一：23 个文件，相对基线约新增 3401 行。

主要完成：

- 实现 `SnapSystem`，支持设置吸附半径、按类型启停吸附、从鼠标位置和拾取射线提取局部/全局特征，并显示吸附点、辅助线和容差范围。
- 实现 `SelectMgr`，从屏幕坐标生成射线，对 AIS 对象执行相交检测，并提供基于 BRepExtrema 等方式的形状求交实验。
- 实现 `FeatureExtractor`，处理点在边上的判断、点到边投影等几何特征计算。
- 设计点、直线、线段等 POI 数据模型及语义类型，配套交叉检测上下文。
- 实现模板化 `POI_BVHTree`，构建不同几何元的 AABB/BVH 加速结构，并分派点、线、线段间的几何求交。
- 增加调试可视化、计时工具和吸附测试代码。

可体现的能力：OCCT 拾取体系、计算几何、空间加速结构、模板编程、交互反馈设计。

注意：该分支属于探索型实现，代码规模大但尚未发现独立设计文档或自动化测试报告；简历中宜写“设计并实现原型”，不宣称生产成熟度或性能提升比例。

### 3.2 `clipper`：二维轮廓偏移封装

主要完成：

- 封装 `ClipperManager`，将 `gp_Pnt` 路径转换为 Clipper2 数据并执行 Offset。
- 支持 JoinType、EndType、偏移距离和容差配置，并将结果转换回 OCCT 点集。
- 将轮廓偏移能力接入现有 OCCT/ImGui 示例进行显示验证。

可体现的能力：第三方几何库集成、数据结构转换、二维轮廓处理。

### 3.3 `BVHTest`：OCCT BVH 学习实验

新增约 915 行的 `BVHIndexGeomtriesManager.h`，尝试建立几何索引与 BVH 管理能力，并调整构建配置。

提交信息明确注明“未实现编译”。因此它适合作为学习轨迹和未完成探索保留，不建议作为简历核心成果；面试中若提及，应主动说明它是失败/暂停的技术实验，并总结遇到的接口或编译问题。

### 3.4 `learnAis`：AIS 自定义显示、选择与 Shader

主要完成：

- 派生 `AIS_InteractiveObject` 实现 `MyAisObject`，实验自定义显示模式和 Presentation 计算。
- 派生 `SelectMgr_EntityOwner` 实现 `MyAisOwner`，自定义选择实体的高亮与取消高亮行为。
- 实现 `RUIClipSectionController`，管理裁剪平面、鼠标滚轮交互和 Shader uniform 更新。
- 编写 `fade_clip.vert` / `fade_clip.frag`，为 AIS 对象加入裁剪渐隐视觉效果。

可体现的能力：理解 OCCT 的 AIS/PrsMgr/SelectMgr 显示选择链路、扩展交互对象、结合 GLSL 实现定制渲染效果。

本地分支比远程跟踪分支多出的提交是 `78dc6c3`（“为AIS对象创建shader”）。

### 3.5 `UseTexture`：多纹理渲染

主要完成：

- 新增顶点/片元 Shader，在渲染流程中实验多纹理采样与组合。
- 在 `GlfwOcctView` 中接入纹理资源和渲染逻辑。
- 增加轻量计时工具用于实验观察。

可体现的能力：OpenGL Shader、纹理管线、OCCT 渲染环境中的底层图形实验。

### 3.6 `UseGeometry`：OCCT 几何与拓扑基础

通过 3 次提交逐步覆盖：

- 使用 `Geom_Line`、`Geom_Curve`、`Geom_Surface`、`Geom_Plane` 等几何对象。
- 从曲线构造 Edge，从 Edge 构造 Wire/Face，并理解几何对象与拓扑对象的区别。
- 使用 `BRep_Tool` 读取 Face 的底层 Surface、Edge 的三维曲线及其在 Surface 上的二维参数曲线。
- 使用 BSpline 截面、旋转/造型操作生成瓶状实体。
- 遍历 Solid 的 Face/Edge/Vertex，识别旋转曲面，读取 UV 边界并进行离散可视化。

可体现的能力：OCCT 几何/拓扑数据模型、B-Rep 遍历、参数曲面与 UV 空间、基础实体造型。

### 3.7 `Handle`：自定义句柄与生命周期实验

主要完成：

- 实现 `MiniDocument`、`Object`、`ElmHandle` 和控制块原型。
- 通过引用计数和对象有效性状态，让外部句柄能判断文档对象是否已失效。
- 实验句柄复制、析构计数和文档删除后的安全访问。

可体现的能力：RAII、引用计数、控制块、观察句柄和悬空指针风险意识。

注意：该实现是学习性质的简化模型，不能等同于完整 `shared_ptr`/`weak_ptr`，也不应宣称线程安全。

### 3.8 `renderCut`：渲染结果快捷读取

主要完成：

- 实现 `ShortCutManager`，持有 `V3d_View` 与 OpenGL GraphicDriver。
- 读取/保存深度缓冲，并依据深度结果判断三维点是否可见。
- 将渲染结果读取能力接入现有视图。

可体现的能力：理解视图投影、深度缓冲和屏幕空间可见性判断，能够跨 OCCT 与 OpenGL 层排查问题。

### 3.9 `modernCpp`：现代 C++ 语法练习

主要实验：

- `nullptr` 与 `NULL` 的类型差异。
- 可变参数模板与递归展开。
- `if constexpr` 终止编译期分支。
- 结构化绑定、泛型 callable 和 Lambda 对容器元素的更新。

该分支适合证明主动学习过程，但内容规模较小，不建议单独作为简历亮点；可以并入信号槽、对象树等模块，说明这些语法后来如何用于真实设计。

### 3.10 `DependencyGraph`：属性依赖与增量求值系统

这是仓库中最适合作为简历核心的独立模块之一。11 次提交体现了从试验到抽象和业务化的连续演进，相对基线新增约 2631 行，最终拆分为 Core、Binding、业务管理和 Sketch 模型多个层次。

核心设计：

- `DGContext` 统一管理 Value、ComputerNode 及 ID，区分用户输入值与派生值，提供属性读写、调试命名、图清理和求值入口。
- `GraphExecutor` 维护值到计算节点、节点到输出值的关联，支持脏值传播和增量求值。
- 使用可达性判断检测潜在环路，避免加入会形成循环依赖的节点。
- 使用 Kahn 拓扑排序确定相互依赖节点的执行顺序。
- 提供流转追踪、变化值记录和图结构日志，便于观察计算链路。
- `PropertyValue`、`ValueHandle` 与外部 PropertyKey/BindingRegistry 形成轻量属性和外部绑定体系。
- 从核心模块移除渲染依赖，把图求值与 OCCT/ImGui 业务展示分离，提高可移植性。
- 以 `SketchModel` 建模点、线段、面积和距离标注；拖拽控制点后触发依赖图求值，并只刷新发生变化的 AIS 场景对象。
- 实现节点和值的增删改查及删除清理逻辑。

可体现的能力：图算法、增量计算、数据驱动架构、脏标记传播、模块解耦、调试可观测性、CAD 参数关联原型。

面试时可重点讲述的演进：

1. 从直接存对象逐步转向统一 ID 查询，明确图内对象所有权。
2. 从简单依赖关系增加环检测与拓扑排序，保证求值合法且有序。
3. 从渲染耦合原型抽象出纯核心层，再用 Binding 和业务管理层接回 OCCT 场景。
4. 从全量刷新演进到记录变化值并局部刷新场景。

### 3.11 `SignalSlotFeat`：从零演进的单线程信号槽机制

通过 8 次提交保留 Version 1 至 Version 9，最终收束为 `MiniSignal`。这条分支的价值在于设计问题和修复过程清晰可追溯。

演进内容：

- Version 1：使用 `std::function` 保存并调用槽函数，建立基础发布/订阅模型。
- Version 2/3：通过弱引用感知接收者死亡，并加入显式 `Connection` 断连能力。
- Version 4：利用函数特征萃取和 `static_assert` 在编译期检查 Signal/Slot 参数类型。
- Version 5：引入 `Trackable`，使 receiver 析构时自动断开关联连接。
- Version 6：将 Signal 内部状态置于共享 State 中，修复 sender 先于 receiver 析构时 Connection 访问失效对象导致的崩溃。
- Version 7：加入 `ConnectionScope`，使用 RAII 管理连接生命周期。
- Version 8：支持参数匹配式连接调用。
- Version 9：支持 Lambda/匿名函数，并通过发射期间的槽列表快照/状态管理，允许槽函数在回调过程中安全断连。
- `MiniSignal`：整理最终单线程版本，保留成员函数槽、匿名函数槽、自动断连和 RAII 连接等能力。

可体现的能力：模板元编程、类型萃取、`std::function`、弱引用、RAII、回调重入与容器迭代失效处理、生命周期设计。

边界：代码明确定位为单线程机制，未提供并发同步保证。

### 3.12 `ObjectTree`：对象树、唯一所有权与安全观察

该分支包含约 683 行实现和一份约 805 行的专项文档 `docs/object-tree-learning-notes.md`，是最有完整学习叙事的分支。

五个版本的主要演进：

- Version 1：建立 parent/children 关系，让父对象析构时级联销毁子树，子对象提前析构时主动脱离父对象。
- Version 2：允许动态换父，并禁止复制，避免双向关系被浅拷贝破坏。
- Version 3：把 `SetParent` 作为维护双方关系的事务入口，分离 `AttachChild`/`DetachChild` 底层原语；检查自父子关系、祖先/后代成环和重复操作等不变量。
- Version 4：使用 `unique_ptr` 表达父对子对象的唯一所有权；换父变为显式所有权转移，并用两阶段方式解除关系后统一释放子对象。
- Version 5：加入 `LifetimeToken` 与非拥有型 `ObjectPtr`，使外部观察者能判断目标是否已经死亡，降低悬空指针风险。

最终将问题拆为三个相对独立的层次：对象关系层、所有权层、生命周期观察层。文档还主动指出对象树不是垃圾回收、观察有效性不等于并发安全，以及 `ObjectPtr` 不能替代共享所有权。

可体现的能力：所有权建模、树结构不变量、异常/边界情况处理、API 设计、RAII、技术写作和方案权衡。

### 3.13 `ObjectRuntime` / `MetaObject`：对象树与信号槽集成

两个分支当前指向同一提交，代表对象树和信号槽两条学习线的融合：

- Object 析构时断开其参与的全部连接。
- 使用生命周期观察能力避免连接继续访问已销毁对象。
- 提供 destroyed 信号，在对象销毁阶段通知外部观察者。
- 测试 receiver 子树销毁后 `ObjectPtr` 失效、连接自动断开，以及根对象销毁通知。

这条集成线展示了从单点机制学习走向“元对象/对象运行时”设计的趋势：对象层级负责所有权，观察指针负责非拥有访问，信号槽负责解耦通知，析构协议统一收束生命周期。

当前工作区在 `MetaObject` 上还有未提交修改（`GlfwOcctView.cpp`、`MetaObjectManager.h`、`ObjectTreeManager.h` 和 `.idea/`）。这些内容没有计入上述已完成成果，待提交后应重新更新本文档。

## 4. 可用于简历的核心项目描述（初稿）

### 版本 A：偏 C++ 基础设施 / 架构

**OcctImgui CAD 技术原型与 C++ 机制学习项目｜个人项目**

- 基于 C++、OpenCASCADE、GLFW 与 Dear ImGui 搭建 CAD 图形实验环境，围绕几何造型、AIS 交互显示、Shader、拾取吸附和对象运行时持续迭代多个原型分支。
- 设计轻量属性依赖图，将 Value、计算节点和外部属性绑定解耦；实现环检测、Kahn 拓扑排序、脏值传播、增量求值与执行链路追踪，并接入参数化草图点/线段/尺寸的局部刷新。
- 从零迭代单线程 C++ 信号槽机制，通过类型萃取完成参数编译期校验，使用弱引用与 RAII 处理 sender/receiver 析构、作用域断连及回调中断连等生命周期边界。
- 迭代对象树所有权模型，使用 `unique_ptr` 表达父子唯一所有权，以事务化换父维护树结构不变量，并设计 `LifetimeToken`/`ObjectPtr` 支持非拥有生命周期观察。
- 实现三维拾取与特征吸附原型，抽象点/线/线段语义数据并使用 BVH 加速候选查询，结合 AIS 提供吸附点、辅助线与容差范围可视化。

### 版本 B：偏 CAD / 图形开发

**OpenCASCADE 交互式 CAD 原型｜个人学习项目**

- 基于 OpenCASCADE + OpenGL/GLFW + ImGui 开发交互式 CAD 实验程序，实践 B-Rep 拓扑遍历、曲线/曲面参数空间、BSpline 截面与旋转实体构造。
- 扩展 OCCT AIS/SelectMgr 体系，实现自定义交互对象、选择 Owner 与高亮逻辑，并通过 GLSL Shader 和裁剪平面实现裁剪渐隐、多纹理等渲染效果。
- 构建屏幕拾取与几何吸附原型：由鼠标坐标生成空间射线，提取局部/全局几何特征，使用 BVH 组织点/线/线段候选并可视化吸附反馈。
- 实现属性依赖图驱动的参数化草图实验，通过环检测、拓扑排序、脏传播和增量求值联动点、线段、距离标注及 AIS 场景局部刷新。

使用建议：正式简历只保留与目标岗位最匹配的 3 至 4 条；代码行数可用于内部判断投入规模，但不建议直接当作成果指标。

## 5. 推荐的面试主线

### 主线一：依赖图如何从 Demo 演进成独立核心

- 最初要解决什么业务问题：控制点变化后，关联几何和标注如何按依赖顺序更新。
- 为什么需要统一 ID、ValueRole、ComputerNode 和 Binding。
- 如何检测新增依赖是否成环；为什么还需要 Kahn 排序。
- 脏值如何传播，如何避免每次重算/重绘整个场景。
- 为什么要把渲染接口移出 Core；这对移植和测试有什么帮助。

### 主线二：信号槽最难的是生命周期，而不是调用函数

- 直接保存回调为什么会在 receiver 析构后崩溃。
- receiver 先死与 sender 先死分别会造成什么问题。
- Connection、Trackable、共享 State、ConnectionScope 分别解决哪一层问题。
- 为什么回调执行过程中断连会引发迭代器/容器状态问题。
- 当前为什么只保证单线程；若扩展到多线程，需要怎样的同步和调用语义。

### 主线三：对象树中的关系、所有权和观察为何必须分开

- parent/children 双向关系需要维护哪些不变量。
- 为什么裸指针版本支持栈对象，却无法可靠表达所有权。
- 为什么改用 `unique_ptr` 后换父 API 必须表达所有权转移。
- `ObjectPtr` 与 `weak_ptr` 的相同点和不同点；为什么“检查后有效”在并发环境仍不充分。
- 对象树与信号槽融合时，析构顺序应该如何设计。

### 主线四：CAD 拾取吸附的数据流

- 屏幕坐标如何变为世界空间拾取射线。
- AIS 选择结果、B-Rep 几何和吸附语义如何衔接。
- 为什么需要区分点、无限直线和有限线段。
- BVH 负责候选裁剪还是精确求交；如何避免把二者混为一谈。
- 吸附容差应使用世界空间距离还是屏幕空间像素，以及不同缩放级别下的体验差异。

## 6. 风险、证据缺口与后续补强

- **构建状态**：本次以 Git 对象和代码结构为主进行只读分析，没有逐分支切换并完整编译；`BVHTest` 已由提交信息明确标记为未编译完成，其余分支也不能据此宣称当前环境全部可构建。
- **测试证据**：多个模块含 `assert`/Demo 测试代码，但尚未看到统一自动化测试框架、覆盖率或 CI 报告。
- **性能证据**：虽然引入 BVH、脏传播和局部刷新等优化思路，但没有基准测试数据，因此暂不写具体性能倍数。
- **原创范围**：`main`/`develop` 含上游示例与其他作者提交；本文只把相对基线的分支增量视为个人学习成果线索。正式对外使用前可进一步按作者和逐提交 diff 审核。
- **工程完整度**：许多实验直接接入 `GlfwOcctView`，后期的 DependencyGraph 已开始解耦；可继续补充模块级 CMake target、测试和 README 运行说明。
- **当前未提交代码**：`MetaObject` 工作区变化尚未纳入结论，避免把进行中的设计写成已完成成果。

建议后续按优先级补强：

1. 为 `DependencyGraph`、`MiniSignal`、`ObjectTree` 各补一组可独立运行的单元测试和边界用例。
2. 给核心模块补充简短 README：问题、设计图、运行方法、限制、示例输出。
3. 对 BVH 吸附和依赖图增量刷新建立小型 benchmark，记录数据规模、机器环境和可复现结果。
4. 选择一个整合分支，把最成熟的三个模块整理成清晰目录和独立构建目标。
5. 在简历定稿前，根据目标岗位选择“C++ 基础设施”或“CAD/图形”叙事，避免一段经历塞入过多主题。

## 7. 证据索引

| 主题 | 分支/提交 | 主要文件 |
|---|---|---|
| 拾取与吸附 | `feature` / `ed7d15b`—`1da1c44` | `SnapSystem.*`、`SelectMgr.*`、`FeatureExtractor.*`、`POI_*` |
| Clipper Offset | `clipper` / `77f627e` | `ClipperManager.*` |
| BVH 未完成实验 | `BVHTest` / `0ec383b` | `BVHIndexGeomtriesManager.h` |
| AIS 与 Shader | `learnAis` / `9ec270a`—`78dc6c3` | `MyAisObject.*`、`MyAisOwner.*`、`RUIClipSectionController.*`、`fade_clip.*` |
| 多纹理 | `UseTexture` / `f225489` | `texture.vert`、`texture.frag`、`GlfwOcctView.*` |
| OCCT 几何 | `UseGeometry` / `22facbf`—`1877450` | `GlfwOcctView.cpp` |
| 自定义句柄 | `Handle` / `d33138f`—`483c463` | `MiniDocument.*` |
| 深度读取/可见性 | `renderCut` / `4e3c3ba` | `ShortCutManager.*` |
| 现代 C++ | `modernCpp` / `e856a12` | `ModernCPPTest.*` |
| 依赖图 | `DependencyGraph` / `f270bd1`—`2a77de7` | `DependencyGraph/Core/*`、`DependencyGraph/Binding/*`、`Sketch/*` |
| 信号槽 | `SignalSlotFeat` / `cdf54a4`—`7e58a09` | `SignalConnectManager.h`、`CommonTraits.h` |
| 对象树 | `ObjectTree` / `db5145c`—`dd8f501` | `ObjectTreeManager.h`、`docs/object-tree-learning-notes.md` |
| 集成对象运行时 | `ObjectRuntime`、`MetaObject` / `381c30d` | `ObjectTreeManager.h`、`SignalConnectManager.*`、`CommonTraits.h` |

## 8. 维护说明

后续每完成一个分支，可在本文件中补充：目标、关键设计、验证方式、量化结果、已知限制和代表提交。模拟面试时应优先从第 5 节四条主线提问，并持续用第 7 节的代码证据校验回答，确保每个简历表述都能落回真实实现。
