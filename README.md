# OcctImgui：依赖图学习分支

这是一个基于 **OpenCASCADE、Dear ImGui 与 GLFW** 的 C++ 示例项目。

当前 `DependencyGraph` 分支的学习重点，是实现一个小型依赖图执行器：业务层只声明“哪些值参与计算、计算会产生哪些结果”，当输入发生变化时，由执行器自动找出受影响的计算，并按依赖顺序完成增量更新。

完整的设计推导、源码索引与当前实现边界见：

- [《从输入变脏到派生值更新：C++ 依赖图执行器的设计主线》](docs/dependency-graph-learning-notes.md)

## 为什么需要依赖图？

在草图或 CAD 场景中，一个输入往往会间接影响很多派生数据。例如：

```text
端点位置 → 线段长度 → 矩形面积
                    └→ 圆形面积

端点位置 → 尺寸标注长度
```

如果由交互代码手动更新这些数据，它不仅需要知道所有依赖关系，还必须负责更新顺序、重复计算与遗漏更新。依赖图把这些职责集中起来：

- 业务代码声明值之间的计算关系；
- 输入变化时只重算受影响的部分；
- 上游计算完成后再执行下游计算；
- 同一轮传播中尽量避免重复执行。

## 核心模型

当前实现是一张由两类节点组成的二部有向图：

```text
ValueHandle → ComputerNode → ValueHandle
```

- `ValueHandle`：保存一组具有业务含义的属性，例如点的位置、线段长度和面积；
- `ComputerNode`：封装一次计算，以及它读取的输入值和写入的输出值；
- `DGContext`：拥有全部值节点和计算节点，是图中数据的生命周期中心；
- `GraphExecutor`：保存依赖索引，负责环检测、脏传播和拓扑调度；
- `ComputerView`：向计算函数提供受约束的输入读取与输出写入接口。

`GraphExecutor` 的核心索引可以概括为：

```text
值节点 → 消费它的计算节点
计算节点 → 它产生的值节点
值节点 → 它唯一的生产者
```

其中，一个值可以被多个计算节点读取，但一个派生值只能有一个生产者。

## 一次更新如何传播

```text
修改用户输入
    ↓
标记对应 ValueId 为脏
    ↓
收集直接依赖这些值的 ComputerNode
    ↓
对当前批次进行拓扑排序
    ↓
按顺序执行计算并写入派生值
    ↓
把输出传播给下一批消费者
    ↓
直到没有新的脏值
```

这不是简单地遍历整张图，而是以变化值为起点进行分批传播。每个批次内部通过拓扑排序保证执行顺序，集合去重和消费者抑制机制则用于减少重复调度。

注册新的计算节点时，执行器还会检查：

- 输入值和输出值是否存在；
- 输出是否重复；
- 输出是否已经有生产者；
- 新关系是否会让图形成环。

环检测的本质是判断：在旧图中，新的输出能否沿依赖关系到达新的输入。如果能够到达，再加入 `输入 → 计算 → 输出` 就会闭合成环。

## 目录结构

```text
DependencyGraph/
├─ Core/
│  ├─ DependencyGraphIds.*   强类型节点 ID
│  ├─ PropertyValue.*        属性值类型
│  ├─ ValueHandle.*          值节点与属性集合
│  ├─ ComputerNode.*         计算节点
│  ├─ ComputerView.*         计算过程的受控数据视图
│  ├─ DGContext.*            数据所有权与外部入口
│  └─ GraphExecutor.*        图索引、环检测与调度执行
├─ Binding/
│  ├─ ExternalPropertyKey.*  外部对象属性标识
│  └─ ValueBindingRegistry.* 外部属性与 ValueId 的双向绑定
└─ DependencyGraphManager.*  OCCT/ImGui 演示与交互集成

Sketch/
└─ SketchModel.*             面向草图业务的接口封装
```

## 辅助能力

除了核心调度流程，这个分支还包含几类辅助设施：

- 模板接口与 `ComputerView`：减少属性读写和类型转换的样板代码；
- `SketchModel`：把通用依赖图包装成点、线段、尺寸和面积等领域操作；
- `DumpGraph`、调试名称和执行跟踪：观察图结构与传播过程；
- `ValueBindingRegistry`：维护外部对象属性与依赖图值节点之间的双向映射。

这些设施服务于可用性、调试和集成，并不改变依赖图的核心主线。

## 当前边界

这是用于理解设计思想的学习实现，而不是成熟的生产级依赖图框架。目前仍有一些值得继续验证或演进的部分：

- `PropertyValue` 只支持当前列出的有限类型；
- 属性变化按 `ValueId` 粒度传播，而不是更细的属性粒度；
- 写入相同值仍会触发更新，`changedValues` 更接近“本轮被触及的值”；
- 计算异常没有事务回滚机制；
- 调度器当前是同步、单线程模型；
- 消费者抑制状态需要补充跨轮更新的回归测试；
- 现有演示代码不能替代系统化的自动测试。

## 构建

项目使用 CMake，并要求支持 C++20。需要提前安装并配置 OpenCASCADE 与 GLFW。

```bash
cmake -S . -B build
cmake --build build --config Debug
```

不同机器上的 OpenCASCADE 路径和运行时环境可能不同，请按本地安装位置调整 CMake 配置。

## 基础组件

- [Open CASCADE Technology](https://dev.opencascade.org/)：三维建模、CAD 数据交换与可视化库；
- [Dear ImGui](https://github.com/ocornut/imgui)：用于调试工具和实时应用的即时模式 GUI；
- [GLFW](https://github.com/glfw/glfw)：负责窗口、输入与 OpenGL 上下文管理。

项目最初参考了 OCCT 社区中的 GLFW + Dear ImGui 集成示例：
[OCCT Issue 33485](https://tracker.dev.opencascade.org/view.php?id=33485)。

![OcctImgui 示例](occt-imgui.png)
