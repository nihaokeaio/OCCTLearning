# MiniSignal / Signal-Slot Learning Notes

这个仓库最初是一个 OpenCASCADE + GLFW + Dear ImGui 示例工程。后来我在其中加入了一段仿 Qt 信号槽机制的学习代码，用多个 `Version` 记录从朴素回调到一个单线程 MiniSignal 的演进过程。

核心代码目前主要在：

- `SignalConnectManager.h`
- `CommonTraits.h`

## 背景

在 C++ 项目中，经常需要事件通知、消息分发、回调注册等机制。传统回调通常能解决“一处触发、一处响应”的问题，但当需求变成：

- 一个事件通知多个接收者
- 接收者析构后自动失效
- 可以主动断开连接
- 连接可以跟随作用域自动释放
- 触发回调时允许回调内部断开其他连接

普通函数指针或简单 `std::function` 容器就会逐渐变得难以维护。

Qt 的信号槽机制非常成熟，但如果只是想要一个轻量的事件通知机制，引入 Qt 会显得过重。因此这个项目尝试用原生 C++ 仿写一个简化版信号槽系统，作为学习 C++ 生命周期、模板、RAII、`std::invoke`、`std::is_invocable` 的练习。

## 最终能力

当前 MiniSignal 单线程最终版支持：

- `Connection` 手动断开连接
- `ScopeConnection` 作用域自动断开
- `Trackable` 在 receiver 析构时自动断开相关连接
- `Signal` 内部使用 `State`，避免 sender/signal 先析构后 `Connection` 悬空访问
- `emit` 时复制 slot 快照，并使用 `connected` 标记避免遍历期间修改容器导致迭代器失效
- 支持成员函数槽
- 支持 lambda / 函数对象槽
- 使用 `std::invoke` 执行槽函数
- 使用 `std::is_invocable` / C++20 `concept requires` 做可调用性检查

这是一个单线程版本，暂不处理跨线程投递、队列连接、锁保护等问题。

## 基本用法

示例类：

```cpp
class Sender : public MiniSignal::Trackable
{
public:
    MiniSignal::Signal<int> valueChanged;

    void changeValue(int value)
    {
        valueChanged.emit(value);
    }
};

class Receiver : public MiniSignal::Trackable
{
public:
    void onValueChanged(int value)
    {
        std::cout << "value = " << value << "\n";
    }
};
```

连接成员函数槽：

```cpp
Sender sender;
Receiver receiver;

auto conn = MiniSignal::connect(
    &sender,
    &Sender::valueChanged,
    &receiver,
    &Receiver::onValueChanged
);

sender.changeValue(42);
conn->DisConnect();
```

连接 lambda 槽：

```cpp
auto scoped = MiniSignal::connectScope(
    &sender,
    &Sender::valueChanged,
    [](int value)
    {
        std::cout << "lambda got " << value << "\n";
    }
);

sender.changeValue(100);
// scoped 离开作用域后自动断开
```

## 版本演进

### Version1: 最小回调列表

第一版只有一个 `std::vector<std::function<void(Args...)>>`，提供 `connect` 和 `emit`。

它证明了信号槽机制的最小模型：

```text
Signal 保存多个 Slot
emit 时依次调用所有 Slot
```

问题是接收对象析构后，回调中保存的对象指针可能悬空。

### Version2: 使用 weak_ptr 观察 receiver 生命周期

第二版把成员函数槽拆成：

```text
对象 weak_ptr
成员函数指针
lambda 包装调用逻辑
```

关键点是 lambda 捕获 `weak_ptr`，不能捕获 `shared_ptr`。否则 slot 自己会延长 receiver 生命周期，导致 `weak_ptr::expired()` 永远无法按预期生效。

这一版解决了 receiver 析构后的调用风险，但还不能主动断开连接。

### Version3: 引入 Connection

第三版增加 `Connection`，让 `connect` 返回一个连接句柄。

外部可以通过：

```cpp
conn.DisConnect();
```

主动断开连接。

这一版开始区分：

```text
Signal 负责保存和触发槽
Connection 负责断开槽
```

问题是 `Connection` 内部仍然可能捕获 `Signal* this`，当 signal 先析构时存在悬空风险。

### Version4: Qt 风格 connect 语法

第四版增加类似 Qt 的外部连接接口：

```cpp
connect(&a, &A::sig, &b, &B::onSig);
```

并引入 `FunctionTraits` 萃取类型：

- 普通函数参数
- 成员函数参数
- const 成员函数参数
- 成员变量类型

这一版的重点是语法糖和编译期参数检查，但 receiver 生命周期仍未完全解决。

### Version5: 引入 Trackable

第五版加入 `Trackable`，由 receiver 保存自己相关的连接。

当 receiver 析构时：

```text
Trackable 析构
遍历所有连接
自动 DisConnect
```

这解决了 receiver 提前析构的问题。设计上接近 Qt 中 `QObject` 的生命周期语义：参与信号槽连接的对象具有身份，不建议复制。

### Version6: 引入 Signal State

第六版将 `slots` 放入共享状态对象：

```text
Signal -> shared_ptr<State>
Connection -> weak_ptr<State>
```

这样当 sender/signal 先析构后，外部再调用 `Connection::DisConnect()` 时，只需要尝试 `weak_ptr<State>::lock()`。如果 state 已经不存在，说明 signal 已经销毁，断开操作安全退出。

这一版解决了 sender/signal 提前析构导致的悬空访问问题。

### Version7: 作用域连接

第七版加入 `ScopeConnection`。

普通连接：

```text
Connection: 手动断开
```

作用域连接：

```text
ScopeConnection: 析构时自动断开
```

这样可以让连接生命周期绑定到局部作用域或某个对象成员，减少忘记断开连接的问题。

### Version8: std::invoke 与 invocable 检查

前面的版本使用 `std::is_same_v<std::tuple<...>, std::tuple<...>>` 检查参数是否完全一致。

第八版改为检查“是否可调用”：

```cpp
std::is_invocable_v<SlotType, Receiver*, Args...>
```

并使用：

```cpp
std::invoke(slot, receiver, args...);
```

统一调用成员函数、函数对象和 lambda。

这里也练习了模板偏特化：

```text
std::tuple<Args...> 保存参数包
偏特化再把 tuple 拆回 Args...
```

### Version9: emit 期间断开连接

第九版解决一个真实使用场景：slot 执行时断开自己或其他连接。

如果遍历 `std::vector` 时直接 `erase`，可能导致迭代器失效。

最终策略是：

```text
emit 开始时复制 slots 快照
disconnect 时只把 slot 标记为 connected = false
emit 结束后统一 cleanUp
```

这样可以保证 emit 过程中修改连接关系时不会破坏当前遍历。

### MiniSignal: 最终整理版

MiniSignal 基于第九版做了命名和语义整理：

- `Connection` 幂等断开
- `ScopeConnection` 不可拷贝
- `connectScope()` 返回值对象
- `Trackable` 不可拷贝
- lambda 捕获使用 move
- 成员函数槽和 lambda 槽分别做可调用性检查

## 和 Qt 的差异

Qt 中可以写：

```cpp
signals:
    void mySignal();
    void sendData(QString data);
```

看起来类里没有显式的 `Signal` 对象，这是因为 Qt 依赖 `Q_OBJECT` 和 moc 代码生成。moc 会生成元对象信息，Qt 内部通过 method index 和 `QObject` 私有数据结构维护连接关系。

MiniSignal 没有 moc，也没有反射系统，因此每个信号都需要显式写成一个对象：

```cpp
MiniSignal::Signal<> mySignal;
MiniSignal::Signal<std::string> sendData;
```

如果一个类有多个信号，就需要多个 `Signal` 成员。这是无代码生成、纯 C++ 实现中比较自然的方式。

## 设计限制

当前 MiniSignal 是学习用单线程版本，有以下限制：

- 不保证线程安全
- 不支持跨线程 queued connection
- `std::function` 不支持 move-only callable
- `std::is_invocable` 会允许隐式转换，例如 `Signal<double>` 可以连接到 `void onSig(int)`，但可能产生窄化转换 warning
- `Trackable` 不可复制，因此继承 `Trackable` 的对象也不建议复制

这些限制是有意保留的，目的是让当前版本保持短小、清晰，专注于信号槽机制的核心模型。

## Build

使用现有 CMake 构建目录：

```powershell
cmake --build cmake-build-debug-visual-studio
```

或者重新生成构建目录：

```powershell
cmake -S . -B cmake-build-debug-visual-studio -DCMAKE_CXX_STANDARD=20
cmake --build cmake-build-debug-visual-studio
```

## 原始工程背景

本仓库仍包含 OpenCASCADE + GLFW + Dear ImGui 示例代码：

- OpenCASCADE: https://dev.opencascade.org/
- Dear ImGui: https://github.com/ocornut/imgui
- GLFW: https://github.com/glfw/glfw

原始示例用于验证 OCCT、GLFW、ImGui 的集成；MiniSignal 是在该工程中额外加入的 C++ 学习模块。
