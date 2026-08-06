# 从输入变脏到派生值更新：C++ 依赖图执行器的设计主线

> 本文梳理 `DependencyGraph` 分支的核心设计。
>
> 这一模块没有按学习版本逐步保留代码，而且调度、调试、GUI 和外部绑定代码交织在一起，因此阅读时很容易失去主线。本文先把所有辅助设施暂时拿开，只回答三个问题：图中保存了什么、依赖怎样注册、一个输入变化后计算怎样传播；最后再把语法糖、调试设施和外部绑定放回各自的位置。

## 1. 为什么需要依赖图？

假设草图中有三个点：

```text
j0 ----- s0 ----- j1
                   |
                   s1
                   |
                  j2
```

线段长度依赖端点位置，矩形面积依赖两条边长，尺寸标注也依赖两个点的位置：

```text
j0.position ─┐
             ├─ distance(s0) ─→ s0.length ─┬─→ rectangle.area
j1.position ─┘                             └─→ circle.area

j1.position ─┐
             ├─ distance(s1) ─→ s1.length ───→ rectangle.area
j2.position ─┘

j0.position ─┐
             ├─ distance(d0) ─→ d0.length
j1.position ─┘
```

如果没有依赖图，每次移动一个点，业务代码都要手工记住：

```text
重算哪些线段？
哪些面积依赖这些线段？
哪些尺寸标注也受影响？
更新顺序是什么？
怎样避免同一个计算执行多次？
```

依赖图把这些“谁依赖谁”的知识从交互代码中抽出来：

```text
业务层只负责声明计算关系
调度器负责找出受影响的节点并按正确顺序执行
```

因此它解决的不是某个具体几何公式，而是一类通用问题：

> 当少量输入变化时，只重新计算受影响的派生数据，并保证每个计算读取到的上游结果已经是最新值。

---

## 2. 先校准整体认识

对当前实现，可以先记住一句话：

> `DGContext` 拥有值和计算节点，`GraphExecutor` 保存依赖索引并调度执行，`SketchModel` 把通用图接口包装成草图领域 API。

原先的理解大体正确，但有三个地方需要更精确。

### 2.1 这不是普通链表，而是二部有向图

图中有两类节点：

```text
ValueHandle   值节点
ComputerNode  计算节点
```

边只能交替出现：

```text
Value → ComputerNode → Value
```

一个值可以被多个计算节点消费，一个计算节点也可以有多个输入和多个输出，所以图允许分叉和汇合。它不是“每个节点只有一个 next”的链表。

因此环检测不能使用链表常见的快慢指针，而是要做一般图的可达性搜索。当前实现使用 BFS。

### 2.2 GraphExecutor 实际维护三类拓扑索引

核心索引不是两张，而是三张：

```cpp
// Value → 消费这个值的计算节点
std::unordered_map<ValueId, std::vector<ComputerNodeId>> dependNodeLists;

// 计算节点 → 它产生的输出值
std::unordered_map<ComputerNodeId, std::vector<ValueId>> nodeOutputs;

// 输出值 → 唯一的生产者计算节点
std::unordered_map<ValueId, ComputerNodeId> valueProducers;
```

前两张表足以沿正方向传播：

```text
Value → consumers → outputs
```

第三张表则快速回答：

```text
这个值是不是派生值？
谁生产了它？
它是否已经有生产者？
```

当前系统规定一个输出值最多只有一个生产者，因此 `valueProducers` 是一对一映射，而不是列表。

### 2.3 调度不是简单的全图 BFS

整个传播过程更准确地说是：

```text
按 dirty value 做波次推进
+
每个波次内部对计算节点做拓扑排序
```

dirty 队列决定“这一批有哪些节点受影响”，拓扑排序决定“这一批节点以什么顺序执行”。

---

## 3. 模块分层与职责

```text
DependencyGraph/Core
├── DependencyGraphIds    ValueId / ComputerNodeId
├── PropertyValue         运行时属性值
├── ValueHandle           真正保存数据的值节点
├── ComputerNode          真正保存计算函数的计算节点
├── ComputerView          计算函数访问输入和输出的受限视图
├── DGContext             对象所有权、注册入口和一致性规则
└── GraphExecutor         拓扑索引、dirty 状态和调度算法

DependencyGraph/Binding
├── ExternalPropertyKey   外部对象属性的稳定标识
└── ValueBindingRegistry  ValueId 与外部属性之间的双向映射

Sketch
└── SketchModel           草图业务层封装

DependencyGraphManager
└── OCCT/ImGui 示例、交互、调试显示与局部场景刷新
```

### 3.1 DGContext：事实数据的所有者

[`DGContext`](../DependencyGraph/Core/DGContext.h) 真正拥有所有值节点和计算节点：

```cpp
std::unordered_map<ValueId, std::unique_ptr<ValueHandle>> m_Values;
std::unordered_map<ComputerNodeId, std::unique_ptr<ComputerNode>> m_Nodes;
std::unique_ptr<GraphExecutor> m_GraphExecutor;
```

可以把它理解为整个依赖图的数据库和事务入口：

- 创建、查询和删除值；
- 注册、查询和删除计算节点；
- 校验依赖是否合法；
- 区分用户输入值和派生值；
- 将外部修改转成 dirty 标记；
- 调用执行器完成求值。

### 3.2 GraphExecutor：拓扑索引与调度状态

[`GraphExecutor`](../DependencyGraph/Core/GraphExecutor.h) 不拥有 `ValueHandle` 或 `ComputerNode` 的真实对象。它保存的是：

```text
依赖索引
dirty value 队列
去重集合
本轮 changedValues
批次抑制状态
调试追踪状态
```

因此它更像：

```text
查询加速索引 + 增量计算调度器
```

### 3.3 ValueHandle：值节点其实是一个属性包

[`ValueHandle`](../DependencyGraph/Core/ValueHandle.h) 内部保存：

```cpp
std::unordered_map<std::string, PropertyValue> properties;
```

当前草图示例中，一个 `ValueId` 通常只对应一个属性：

```text
point.position
segment.length
area.area
```

但类型本身允许一个 `ValueHandle` 保存多个命名属性。

需要注意：依赖图的粒度仍然是 `ValueId`，不是单个 property。只要这个 handle 中任意属性通过外部入口改变，调度器标脏的是整个 `ValueId`。

`PropertyValue` 目前使用封闭的 `std::variant`：

```cpp
std::variant<
    std::monostate,
    bool,
    int,
    double,
    gp_Pnt,
    std::string
>;
```

它提供了足够的教学和草图示例类型，但还不是开放的通用类型系统。

### 3.4 ComputerNode：输入、输出和一个计算函数

[`ComputerNode`](../DependencyGraph/Core/ComputerNode.h) 保存：

```cpp
std::vector<ValueId> m_Inputs;
std::vector<ValueId> m_Outputs;
ComputeFunc computeFunc;
```

执行时，它创建一个 `ComputerView`，再调用用户注册的 lambda：

```cpp
void ComputerNode::Evaluator(DGContext& context)
{
    ComputerView view(context, m_Inputs, m_Outputs);
    computeFunc(view);
}
```

计算节点本身不保存业务结果。结果始终写回输出 `ValueHandle`，因此值节点是系统中的状态，计算节点是状态之间的转换规则。

### 3.5 ComputerView：计算函数的能力边界

[`ComputerView`](../DependencyGraph/Core/ComputerView.h) 为 lambda 提供简洁接口：

```cpp
const auto p0 = view.Input<gp_Pnt>(0, "position");
const auto p1 = view.Input<gp_Pnt>(1, "position");
view.SetOutput(0, "length", p0.Distance(p1));
```

它隐藏了：

- `DGContext` 的内部容器；
- `ValueId` 到 `ValueHandle` 的查询；
- `PropertyValue` 的类型提取；
- 输入、输出 ID 的边界检查。

这既是语法糖，也是一层约束：计算函数只拿到注册时声明的输入和输出视图，而不是直接操作整张图。

---

## 4. 图是怎样注册进去的？

### 4.1 值节点可以直接创建

用户输入值：

```cpp
ValueId pointPosition =
    context.CreateInputValue("position", gp_Pnt{0, 0, 0});
```

派生值：

```cpp
ValueId segmentLength =
    context.CreateDerivedValue("length", 0.0);
```

二者的数据结构相同，区别是角色：

```cpp
enum class ValueRole
{
    UserInput,
    Derived
};
```

外部 API 只允许修改 `UserInput`。`Derived` 必须由计算节点通过 `ComputerView::SetOutput()` 写入。

这样可以阻止调用者绕过依赖关系，直接篡改本应由公式产生的结果。

### 4.2 注册计算节点前需要维护不变量

```cpp
context.AddComputeNode(
    {startPosition, endPosition},
    {segmentLength},
    computeLambda);
```

[`DGContext::AddComputerNode`](../DependencyGraph/Core/DGContext.cpp) 在真正修改图之前检查：

1. 所有输入值必须存在；
2. 所有输出值必须存在；
3. 同一节点的输出列表不能包含重复值；
4. 每个输出值不能已经有其他生产者；
5. 新依赖不能形成环。

通过后才会：

```text
input ValueId → 注册到 dependNodeLists
nodeId/output ValueId → 注册到 nodeOutputs 和 valueProducers
所有 output → 标记为 Derived
ComputerNode → 移入 DGContext::m_Nodes
```

### 4.3 环检测为什么是“从输出找输入”？

准备加入的新节点会新增：

```text
input → new node → output
```

如果旧图中已经存在：

```text
output → ... → input
```

加入新节点后就会闭合成环：

```text
input → new node → output → ... → input
```

因此，对每一对 `output` 和 `input`，系统检查：

```cpp
CanReachValue(outputId, inputId)
```

`CanReachValue` 使用 BFS：

```text
当前 Value
→ 查 dependNodeLists，找到所有消费者节点
→ 查 nodeOutputs，找到这些节点的所有输出 Value
→ 继续入队搜索
```

`visited` 集合避免重复访问。只要任意输出已经能够到达任意输入，注册就被拒绝。

这不是链表的快慢指针环检测，而是一般有向图的可达性问题。

### 4.4 为什么执行时还要再检查一次环？

注册阶段已经拒绝环，但 `BatchNodeSort` 仍会检查最终排序数量：

```cpp
if (sortedCount != dirtyNodeIds.size())
{
    throw std::runtime_error(
        "Cycle detected inside dirty node batch");
}
```

这是防御性校验。它保护调度器免受：

- 注册逻辑未来发生回归；
- 内部索引不一致；
- 非标准入口绕过校验；
- 批次拓扑构建错误。

---

## 5. 从一个输入变化开始：完整执行主线

整个系统最值得记住的是下面这条链：

```text
修改 UserInput
→ MarkDirty(value)
→ 收集直接受影响的计算节点
→ 对本批节点做拓扑排序
→ 按序执行计算节点
→ 输出值进入下一波 dirty 传播
→ 队列为空，求值结束
```

### 5.1 第一步：外部只能修改 UserInput

例如移动草图点：

```cpp
SketchModel::MovePoint(point, newPosition)
```

最终调用：

```cpp
DGContext::SetInputValueProperty(
    point.position,
    "position",
    newPosition);
```

这个入口先检查值存在且角色为 `UserInput`，然后写入属性并调用：

```cpp
m_GraphExecutor->MarkDirty(valueId, this);
```

### 5.2 第二步：MarkDirty 去重入队

`GraphExecutor` 同时使用：

```cpp
std::queue<ValueId> dirtyQueue;
std::unordered_set<ValueId> m_DirtyValues;
```

队列保存处理顺序，集合负责去重：

```cpp
if (m_DirtyValues.insert(id).second)
{
    dirtyQueue.push(id);
}
```

同一个值在被消费前多次标脏，只会进入队列一次。

`MarkDirty` 还会把这个值加入 `m_ChangedValues`。需要注意，当前实现没有比较新旧值；即使写入相同数据，它也会被认为“发生过变化”。因此这里的 `changedValues` 更接近：

```text
本轮被写入或被计算触及的值
```

而不是严格意义上的数值差异集合。

### 5.3 第三步：CollectDirtyNodes 形成当前批次

`Evaluate()` 每轮创建：

```cpp
std::queue<ComputerNodeId> dirtyNodeQueue;
std::unordered_set<ComputerNodeId> dirtyNodeIds;
```

`CollectDirtyNodes` 会消费当前 `dirtyQueue` 中的所有值。对每个 dirty value：

```text
查 dependNodeLists[value]
→ 找到直接消费者
→ 记录“哪个值触发了哪个节点”
→ 将节点加入本批
```

`dirtyNodeIds` 保证同一个计算节点即使被多个脏输入同时触发，也只执行一次：

```text
inputA dirty ─┐
              ├─→ NodeX 只入队一次
inputB dirty ─┘
```

到这里还没有执行任何计算，只是得到“本批需要运行哪些节点”。

### 5.4 第四步：BatchNodeSort 保证批内顺序

多个节点同时进入一批，不代表它们可以按任意顺序执行。

例如：

```text
A ─→ N1 ─→ X ─→ N2 ─→ Y
B ─────────────→ N2
```

如果 A 和 B 同时变脏：

```text
A 触发 N1
B 触发 N2
```

于是 N1、N2 同时进入当前批次。但 N2 还依赖 N1 产生的 X，因此必须先执行 N1。

`BatchNodeSort` 临时把二部图压缩成“计算节点之间的图”：

```text
N1 的 output X 被 N2 消费
→ 建立 N1 → N2
```

它只考虑 `dirtyNodeIds` 内部的节点，得到本批诱导子图，然后使用 Kahn 算法：

```text
统计每个节点入度
→ 所有入度为 0 的节点入 ready queue
→ 弹出节点并降低下游入度
→ 下游入度变为 0 时入队
```

排序后的队列替换原 `dirtyNodeQueue`。

这里不是每次对整张图排序，而是只对本次受影响的计算节点排序。这减少了无关节点参与，但也意味着每个批次都要重新构造局部拓扑。

### 5.5 第五步：EvaluateDirtyNodes 按序执行

对排序后的每个 `ComputerNode`：

```text
DGContext 根据 nodeId 取得 ComputerNode
→ ComputerNode 创建 ComputerView
→ 调用 computeFunc
→ lambda 读取 inputs，写入 outputs
```

然后调度器遍历节点的每个输出值：

1. 将输出加入 `changedValues`；
2. 如果没有下游消费者，传播在这里结束；
3. 如果有下游消费者，决定哪些需要留到下一批；
4. 必要时将输出 `MarkDirty`。

### 5.6 第六步：输出推动下一批

为什么输出不直接在当前循环中递归执行下游？

因为批次模型能够：

- 合并多个上游共同触发的节点；
- 在执行前统一去重；
- 对同批节点统一拓扑排序；
- 避免深递归调用栈；
- 形成清晰的传播波次。

`Evaluate()` 外层持续执行：

```cpp
while (!dirtyQueue.empty())
{
    CollectDirtyNodes(...);
    BatchNodeSort(...);
    EvaluateDirtyNodes(...);
}
```

直到没有新的输出值需要传播。

---

## 6. 为什么需要“消费者抑制”优化？

这是当前调度器中最容易让主线变乱的部分。

仍使用这个图：

```text
A ─→ N1 ─→ X ─→ N2 ─→ Y
B ─────────────→ N2
```

当 A、B 同时变脏时，N1 和 N2 已经进入同一批。拓扑排序保证：

```text
N1 先更新 X
N2 随后读取最新 X
```

N1 执行后，如果仍然无条件把 X 标脏，下一批又会由 X 触发 N2，导致 N2 重复执行。

因此 `SuppressCurrentBatchConsumers` 将 X 的消费者分成两类：

```text
本批已经包含的消费者
    已经由拓扑排序安排到正确位置
    下一批应该跳过

本批之外的消费者
    当前还没有执行
    仍然需要由 X 的 dirty 状态触发下一批
```

`m_SuppressedConsumers` 保存：

```cpp
ValueId → 本批已经满足的消费者节点集合
```

下一批 `CollectDirtyNodes` 处理 X 时：

- 跳过已经在上一批执行过的消费者；
- 让其他消费者正常入队；
- 随后删除 X 对应的抑制记录。

这是一项减少重复计算的跨批次优化，不是依赖图成立的基础。第一次阅读时，可以先把它想象成：

```text
“这个消费者已经吃过 X 的最新值了，下一轮不要再喂一次。”
```

### 6.1 当前实现值得补测试的一处细节

当前 `SuppressCurrentBatchConsumers` 会先写入抑制表，再返回是否存在批外消费者。如果所有消费者都已经在本批中，输出值不会再次入 dirty 队列，但抑制记录也没有立刻清除。

这可能让记录残留到未来一次独立求值，并错误跳过消费者。一个典型场景是：

```text
第一次：A、B 同时 dirty，N1 与 N2 同批执行，X 不再入队
第二次：只有 A dirty，N1 更新 X，N2 本应在下一批执行
```

因此这里应优先增加回归测试，并考虑：

- 仅在确实会将 output 入队时写入抑制表；
- 或在本轮求值结束时清理所有抑制状态。

这不改变主调度模型，但说明优化状态的生命周期必须比基础 dirty 队列更加谨慎。

---

## 7. 用草图示例走一遍真实传播

[`SketchModel`](../Sketch/SketchModel.cpp) 创建的主要关系是：

```text
j0.position ─┐
             ├─ distance s0 ─→ s0.length ─┬─ rectangle area ─→ rect.area
j1.position ─┘                            └─ circle area ────→ circle.area

j1.position ─┐
             ├─ distance s1 ─→ s1.length ─── rectangle area
j2.position ─┘

j0.position ─┐
             ├─ distance d0 ─→ d0.length
j1.position ─┘
```

现在移动 `j0`：

```cpp
sketch.MovePoint(j0, newPosition);
sketch.Evaluate();
```

### Batch 1：位置触发直接计算

```text
dirty value:
    j0.position

直接消费者:
    distance s0
    distance d0

执行结果:
    更新 s0.length
    更新 d0.length
```

`d0.length` 没有下游计算消费者，传播结束。

`s0.length` 被矩形面积和圆面积消费，因此进入下一波 dirty 队列。

### Batch 2：长度触发面积计算

```text
dirty value:
    s0.length

直接消费者:
    rectangle area
    circle area

执行结果:
    更新 rect.area
    更新 circle.area
```

面积没有继续被其他计算节点消费，整个求值结束。

本轮 `EvaluationResult::changedValues` 大致包含：

```text
j0.position
s0.length
d0.length
rect.area
circle.area
```

GUI 层可以据此只刷新受影响的场景元素，而不是重建整张草图。

---

## 8. 语法糖与业务封装放在什么位置？

这些代码会让模块看起来比核心算法大很多，但它们的作用主要是改善使用方式。

### 8.1 DGContext 的模板 API

```cpp
CreateInputValue<T>()
CreateDerivedValue<T>()
GetValueProperty<T>()
SetInputValueProperty<T>()
```

它们隐藏了：

- `unique_ptr<ValueHandle>` 创建；
- 属性的 `PropertyValue` 包装；
- ID 查询；
- UserInput/Derived 权限检查；
- dirty 标记。

### 8.2 ComputerView 的位置式访问

```cpp
view.Input<T>(inputIndex, propertyName);
view.SetOutput(outputIndex, propertyName, value);
```

计算 lambda 不需要保存 `DGContext*` 或手动查 `ValueId`。

### 8.3 SketchModel 的领域 API

通用依赖图只理解：

```text
ValueId
ComputerNodeId
PropertyValue
```

`SketchModel` 则提供：

```cpp
CreatePoint()
CreateSegment()
CreateRectangleArea()
CreateDistanceDimension()
MovePoint()
GetLength()
Evaluate()
```

它把字符串属性名、输入输出次序和公式注册藏在业务对象后面。

因此三层接口可以这样理解：

```text
GraphExecutor：调度算法层，业务代码通常不直接使用
DGContext：通用依赖图 API
SketchModel：面向草图领域的易用 API
```

---

## 9. 调试代码怎样从主算法中剥离理解？

当前实现提供三组调试能力。

### 9.1 DebugName 与 DumpGraph

随机 ID 对人不友好：

```text
Value#37 → Node#12
```

`DGContext::SetDebugName` 可以赋予业务名：

```text
j0.position → distance s0 → s0.length
```

`DumpGraph()` 则输出：

- 所有值及其角色；
- 所有计算节点；
- 每个节点的 inputs/outputs；
- 完整的 `Value → Node → Value` 边。

它描述的是图的静态结构。

### 9.2 DG_ENABLE_TRACE：逐步骤控制台追踪

Debug 构建通过 CMake 定义：

```cmake
DG_ENABLE_TRACE
```

编译后，还要由 `SetTraceEnabled(true)` 打开运行时开关。日志包括：

```text
MarkDirty
BeginBatch / EndBatch
PopDirty
QueueNode
EvaluateNode
PropagateOutput
SkipNodeAlreadyQueued
SkipNodeSatisfiedInPreviousBatch
```

它描述的是调度器如何作出每一步决定。

阅读核心代码时，可以暂时忽略所有：

```cpp
#ifdef DG_ENABLE_TRACE
```

留下的就是 dirty 收集、拓扑排序和执行传播。

### 9.3 FlowTraceCallback：面向 UI 的摘要

调度器还记录：

```text
哪个 value 触发了哪个 node
这个 node 产生了哪些 outputs
```

求值结束后通过回调输出：

```text
j0.position -> distance s0 -> s0.length
s0.length -> rectangle area -> rect.area
```

`DependencyGraphManager` 将这些行显示在 ImGui 的 `DGFlow` 面板中。

它与逐步骤控制台日志是两套机制：

```text
console trace：解释调度器内部决策
flow summary：向用户展示业务数据流
```

当前代码中，`SetTraceEnabled` 只控制 `DG_ENABLE_TRACE` 下的逐步骤日志；FlowTraceCallback 的事件收集和回调并不受这个布尔值控制。理解这一点可以避免误以为 GUI 勾选框会关闭所有追踪。

---

## 10. 外部绑定层解决什么？

依赖图内部使用随机 `ValueId`，而外部系统通常使用稳定业务身份：

```text
对象 j0 的 position 属性
对象 s0 的 length 属性
对象 rect 的 area 属性
```

[`ExternalPropertyKey`](../DependencyGraph/Binding/ExternalPropertyKey.h) 将它表示为：

```cpp
struct ExternalPropertyKey
{
    std::string objectId;
    std::string propertyKey;
};
```

[`ValueBindingRegistry`](../DependencyGraph/Binding/ValueBindingRegistry.h) 保存双向一对一映射：

```text
ValueId → ExternalPropertyKey
ExternalPropertyKey → ValueId
```

注册新绑定时，如果任意一侧已经绑定到其他对象，会先删除旧的反向关系，从而保持双向表一致。

### 10.1 它不参与核心调度

绑定表不会：

- 保存真正的 `PropertyValue`；
- 决定依赖顺序；
- 标记 dirty；
- 执行计算节点。

它只是连接两个身份空间：

```text
依赖图内部 ID
↔
外部业务对象属性
```

### 10.2 当前绑定仍是原型设施

目前示例主要用绑定表在 ImGui 中展示：

```text
s0.length [Derived] -> s0.length
```

场景刷新仍然直接把 `changedValueId` 与 `SketchPoint`、`SketchSegment`、`SketchDistanceDimension` 中保存的 ID 比较，并没有完全通过绑定表自动分发更新。

因此它更像未来数据绑定系统的基础，而不是已经完成的自动绑定引擎。

---

## 11. DependencyGraphManager：演示集成，不是核心图算法

[`DependencyGraphManager`](../DependencyGraph/DependencyGraphManager.cpp) 同时负责：

- 创建演示草图依赖；
- 保存 OCCT `AIS_Shape` 和文字标注；
- 处理 ImGui 控件；
- 将鼠标屏幕坐标投影到草图平面；
- 拖动点并触发求值；
- 根据 changedValues 局部刷新场景；
- 显示 trace 和 binding 信息。

它的真实调用链是：

```text
鼠标拖动 / ImGui 修改坐标
→ DependencyGraphManager::MovePoint
→ SketchModel::MovePoint
→ DGContext::SetInputValueProperty
→ GraphExecutor::MarkDirty
→ SketchModel::Evaluate
→ GraphExecutor::Evaluate
→ EvaluationResult.changedValues
→ RefreshChangedScene
→ 更新对应 OCCT 表示对象
```

因此阅读核心依赖图时，不需要先理解拾取、投影、AIS 显示和 ImGui。它们只是证明调度结果能够驱动真实外部系统。

初始化阶段将每个点“设置为当前值”也是有意的：

```cpp
MovePoint(j0, GetPosition(j0));
```

虽然数值没有变化，但它会标记输入 dirty，让初始派生长度、面积和尺寸也走与真实编辑完全相同的求值路径。

---

## 12. 删除节点时为什么也需要维护两套状态？

图结构同时存在于：

```text
DGContext 的对象容器
GraphExecutor 的拓扑索引和调度状态
```

删除 `ComputerNode` 时，需要：

- 从所有 value 的消费者列表中移除 node；
- 删除 node 的输出列表；
- 删除这些输出对应的 producer；
- 清理 trigger、suppression 和 flow event；
- 从 `DGContext::m_Nodes` 释放真实节点；
- 将失去生产者的输出值重新设为 `UserInput`。

删除 `ValueHandle` 更严格：如果它仍有消费者或生产者，`DGContext` 会拒绝删除。调用者必须先拆除关联计算节点。

这体现了一个重要原则：

> 保存冗余索引可以加速查询和调度，但每一次结构修改都必须同步维护所有副本。

---

## 13. 当前实现的边界与值得补充的测试

### 13.1 changedValues 是保守集合

当前没有比较 `oldValue == newValue`：

- 输入写入相同值仍会标脏；
- 计算节点执行后，所有输出都会记录为 changed；
- 下游计算可能因此执行，即使结果数值没有变化。

这保证实现简单和传播安全，但可能产生额外计算与刷新。

### 13.2 ID 生成器是教学实现

`ValueId` 与 `ComputerNodeId` 共享一个范围为 1～100 的随机唯一 ID 生成器。

它适合小型演示，但真实系统通常会使用：

- 单调递增 64 位 ID；
- 分类型 ID 空间；
- 可回收句柄加 generation；
- 或稳定 UUID。

### 13.3 PropertyValue 是封闭类型集合

增加新业务类型需要修改 `PropertyValue::Storage`。如果依赖图继续泛化，可以借鉴元对象章节中的 `MetaType + std::any`，或者让图层模板化。

### 13.4 计算异常没有事务回滚

如果一个计算节点已经写入部分输出，随后抛出异常，当前系统不会自动恢复旧值，也不会回滚已执行的上游节点。

目前更接近“所有计算函数都应成功”的同步执行模型。

### 13.5 缺少独立自动化测试

`DependencyGraphManager::Test()` 是一个演示辅助函数，并没有形成覆盖算法边界的测试集。后续最值得加入 GoogleTest 的案例包括：

- 线性链传播；
- 分叉与汇合；
- 多个输入同时触发同一节点时只执行一次；
- 批内拓扑顺序；
- 环注册被拒绝；
- 一个 output 不能有两个 producer；
- UserInput/Derived 写权限；
- 删除节点后的索引一致性；
- 消费者抑制不会跨 Evaluate 残留；
- changedValues 的精确定义。

### 13.6 当前执行模型是同步、单线程的

所有 dirty 收集、拓扑排序和计算都在调用 `Evaluate()` 的线程中完成。

如果未来并行执行，还需要解决：

- 同批无依赖节点的任务调度；
- `DGContext` 和值写入的并发保护；
- 多输出节点的原子发布；
- 异常传播与取消；
- 确定性执行顺序。

---

## 14. 推荐的重新阅读顺序

如果隔一段时间后再次回顾，不建议从 `DependencyGraphManager.cpp` 开始。比较清晰的顺序是：

1. [`DependencyGraphIds.h`](../DependencyGraph/Core/DependencyGraphIds.h)：只认识两种 ID；
2. [`PropertyValue.h`](../DependencyGraph/Core/PropertyValue.h)：理解值如何存储；
3. [`ValueHandle.h`](../DependencyGraph/Core/ValueHandle.h)：理解值节点；
4. [`ComputerNode.h`](../DependencyGraph/Core/ComputerNode.h)：理解计算节点；
5. [`ComputerView.h`](../DependencyGraph/Core/ComputerView.h)：理解 lambda 怎样读写数据；
6. [`DGContext.h/.cpp`](../DependencyGraph/Core/DGContext.h)：理解对象所有权、注册约束和环检测；
7. [`GraphExecutor.h/.cpp`](../DependencyGraph/Core/GraphExecutor.h)：只看 `MarkDirty → Evaluate → Collect → Sort → Execute`；
8. 再看 suppression 与 trace；
9. [`SketchModel.cpp`](../Sketch/SketchModel.cpp)：看通用图怎样包装成业务 API；
10. 最后看 [`DependencyGraphManager.cpp`](../DependencyGraph/DependencyGraphManager.cpp) 和 Binding。

第一次重读 `GraphExecutor.cpp` 时，可以主动忽略：

```text
#ifdef DG_ENABLE_TRACE
RecordNodeTrigger / RecordNodeOutputs / EmitFlowSummary
RemoveNode / RemoveValue / Clear
SuppressCurrentBatchConsumers
```

只留下五个函数：

```text
MarkDirty
Evaluate
CollectDirtyNodes
BatchNodeSort
EvaluateDirtyNodes
```

理解主线后，再把去重、抑制、调试和删除逻辑逐层加回来。

---

## 15. 总结

整个依赖图可以浓缩为两类节点、三张拓扑索引和一个批次循环。

两类节点：

```text
ValueHandle   保存状态
ComputerNode  保存状态转换规则
```

三张索引：

```text
Value → consumers
ComputerNode → outputs
Value → producer
```

一个循环：

```text
dirty values
→ 收集直接消费者
→ 批内拓扑排序
→ 执行计算节点
→ outputs 继续标脏
→ 直到队列为空
```

`DGContext` 保证图在结构上成立，`GraphExecutor` 保证变化按正确顺序传播，`SketchModel` 让业务代码不必面对底层 ID 和属性容器，`DependencyGraphManager` 则证明这套结果可以驱动真实的 OCCT 场景与 ImGui 交互。

真正应该抓住的不是 `m_SuppressedConsumers` 或 trace 宏等细节，而是下面这条主线：

> 输入值只负责宣布“我变脏了”；调度器沿依赖关系找出受影响的计算节点，在局部拓扑序中执行它们，并把新产生的值继续传播，直到所有派生状态重新一致。

