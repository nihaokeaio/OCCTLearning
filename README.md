# OcctImgui — ObjectTree 学习分支

这是 `OcctImgui` 项目的 **ObjectTree 学习分支**。

本分支受 Qt `QObject` 对象树启发，目标不是完整复刻 Qt，而是只使用 ISO C++20，通过五个逐步演进的版本理解三个容易混在一起的问题：

```text
对象关系：一个对象位于树的什么位置？
内存所有权：谁负责销毁这个对象？
生命周期观察：不拥有对象的一方如何知道它是否仍然存活？
```

核心实现保留了 `Version1`～`Version5` 的完整学习过程。重点不只是最终代码能够运行，而是每一版遇到了什么问题，以及为什么必须改变原来的设计。

## 为什么需要对象树？

GUI、场景对象和文档模型天然具有层级关系：

```text
Window
├── Toolbar
│   └── Button
└── View
    ├── Camera
    └── Selection
```

如果层级关系与生命周期没有统一规则，销毁一个根对象时，调用者必须手动记住整棵子树的清理顺序。随着提前删除、动态换父和跨作用域引用出现，很快就会遇到：

- 忘记释放对象，造成内存泄漏；
- 多个父对象认为自己拥有同一个 child，造成重复释放；
- 父对象仍保存已经死亡的子对象；
- 销毁过程中修改 children，导致迭代器失效；
- 外部保存的裸指针在对象死亡后继续被使用。

对象树的价值不只是把对象排成一棵树，而是建立确定性的生命周期协议：

```text
父对象拥有子对象
→ 父对象销毁时级联销毁整棵子树
→ 子对象提前销毁时自动脱离父对象
```

## 版本演进

| 版本 | 遭遇的问题 | 主要结果 |
|---:|---|---|
| V1 | 父对象遍历 children 时，子对象析构会回头修改同一个容器 | 确立“先解除关系，再销毁对象”，避免迭代器失效 |
| V2 | `SetParent` 与 `AddChildren` 都试图维护完整双向关系，容易递归或只更新一侧 | 引入动态换父，并暴露双向关系必须集中维护的问题 |
| V3 | 仅靠多个公开接口“各自写对”无法保证对象树始终一致 | 以 `SetParent` 作为唯一高层事务入口，建立无环、唯一父对象等不变量 |
| V4 | 裸指针容器无法从类型上表达所有权，栈对象和堆对象也无法区分 | 使用 `unique_ptr` 建立严格所有权树，让换父显式表现为所有权转移 |
| V5 | `unique_ptr` 解决了谁负责删除，却没有解决外部裸指针悬空 | 引入 `LifetimeToken + ObjectPtr`，为非拥有引用增加生命周期观察 |

完整的设计过程、失败案例和每一版的取舍见：

**[从零实现一棵 C++ 对象树：关系、所有权与生命周期观察](docs/object-tree-learning-notes.md)**

## 最终得到的三层模型

```text
关系层
    parent / children
    描述对象位于树中的什么位置

所有权层
    QObject 风格裸指针协议
    或 unique_ptr 严格所有权
    描述谁负责销毁对象

观察层
    ObjectPtr / LifetimeToken
    描述外界如何安全保存非拥有引用
```

它们相互关联，但不能互相替代：

- `parent` 指针能表示关系，却无法证明父对象拥有 child；
- `unique_ptr` 能保证所有权，却不会自动让外部裸指针失效；
- `ObjectPtr` 能观察对象是否存活，却不参与对象销毁。

## 两种可用的所有权模型

### Version 3：QObject 风格的裸指针树

```cpp
auto* root = new Object("root");
auto* child = new Object("child", root);

child->SetParent(otherParent);
child->SetParent(nullptr);
```

这一版使用裸指针表达树关系，通过析构协议表达所有权。

优点：

- API 简洁；
- 动态换父自然；
- 接近 Qt `QObject` 的使用方式。

代价：

- 所有权规则主要存在于协议中，而不是指针类型中；
- 裸指针无法判断对象来自堆、栈、成员还是静态存储；
- 调用者必须遵守对象构造和销毁契约。

### Version 4 / Version 5：unique_ptr 严格所有权树

```cpp
auto root = std::make_unique<Object>("root");
auto child = std::make_unique<Object>("child");

Object* childView = root->AttachChild(std::move(child));
childView->Reparent(otherParent);
```

这一版让 children 容器真正保存 `unique_ptr<Object>`：

```text
谁持有 unique_ptr
→ 谁就是对象的唯一所有者
```

换父不再只是修改关系，而是：

```text
旧父对象交出 unique_ptr
→ 临时所有者接住
→ 新父对象接收 unique_ptr
```

接口更显式，也会比裸指针版稍显复杂。这种“别扭”并不是实现失败，而是所有权转移被真实地呈现在 API 中。

## ObjectPtr：观察对象，而不是拥有对象

即使所有对象都被 `unique_ptr` 正确管理，外部仍可能保存：

```cpp
Object* childView = child.get();
```

对象销毁后，这个指针不会自动变空。V5 因此加入：

```cpp
ObjectPtr<Object> observer(child.get());
```

每个 `Object` 持有一枚独立的 `LifetimeToken`，`ObjectPtr` 保存原始地址和指向令牌的 `weak_ptr`：

```text
令牌仍然存在
→ 对象仍然存活
→ ObjectPtr::Get() 返回对象地址

令牌已经过期
→ 对象已进入析构
→ ObjectPtr::Get() 返回 nullptr
```

由此可以明确区分三种指针语义：

```cpp
std::unique_ptr<Object> owner; // 拥有对象并负责销毁
Object* borrowed;             // 函数调用期间的短期借用
ObjectPtr<Object> observer;   // 可跨作用域保存的非拥有观察
```

`ObjectPtr` 的目标与 Qt `QPointer` 类似，但当前实现只面向单线程对象模型。它不能解决另一个线程在检查后、调用前并发销毁对象的问题。

## 对象树不变量

公开操作完成后，对象树应始终满足：

```text
child.parent == parent
当且仅当
parent.children 中包含 child
```

此外还包括：

- 一个对象最多只有一个父对象；
- 对象不能成为自己的父对象；
- 对象不能把自己的后代设置为父对象；
- 对象树中不能出现环；
- 重复设置相同父对象是无副作用操作；
- `SetParent(nullptr)` 可以让对象脱离当前树；
- 销毁时先解除结构关系，再释放所有权。

这些不变量比某一个成员函数的具体写法更重要。V3 以后，设计重点也从“功能能不能跑”转向“任意公开操作后结构是否仍然成立”。

## 主要文件

```text
ObjectTreeManager.h
    Version1～Version5 的完整实现与对应测试代码

docs/object-tree-learning-notes.md
    对象树学习过程和设计总结

CMakeLists.txt
    OcctImgui 示例程序的 CMake 配置
```

历史版本被有意保留在不同命名空间中，便于直接比较同一个问题在不同阶段的解法：

```cpp
namespace Version1 { /* 最初的裸指针对象树 */ }
namespace Version3 { /* 收束不变量后的 QObject 风格版本 */ }
namespace Version4 { /* unique_ptr 所有权版本 */ }
namespace Version5 { /* 增加 ObjectPtr 生命周期观察 */ }
```

## 测试与运行

当前分支的历史测试位于 `ObjectTreeManager.h` 的 `ObjectTreeManager::test1()`～`test5()` 中，默认通过全局管理对象运行最终的 `test5()`。

测试覆盖：

- 父对象级联销毁 children；
- 子对象提前销毁并自动脱离；
- 动态换父和主动脱离对象树；
- 拒绝自己成为自己的父对象；
- 拒绝将祖先挂到后代之下形成环；
- `unique_ptr` 所有权转移；
- `ObjectPtr` 的复制、移动、重置和自动失效。

构建项目：

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

当前 CMake 工程需要：

- CMake 3.15 或更高版本；
- 支持 C++20 的编译器；
- 已可被 `find_package` 找到的 GLFW3 与 OpenCASCADE。

## 当前边界

这是一个用于理解原理的教学实现，目前有意没有继续处理：

- 跨线程并发访问和销毁；
- 延迟删除与事件循环；
- 共享所有权图；
- 弱引用提升为强引用；
- 完整 Qt `QObject` API；
- 面向生产环境的独立测试目标。

后续最自然的方向，是把对象树与信号槽模块融合，统一对象身份、层级所有权、非拥有观察和连接生命周期。

## 原项目

本学习分支建立在 `OcctImgui` 示例项目之上。原项目组合了：

- [Open CASCADE Technology](https://dev.opencascade.org/)：三维几何建模、CAD 数据交换与可视化能力；
- [Dear ImGui](https://github.com/ocornut/imgui)：即时模式图形界面；
- [GLFW](https://github.com/glfw/glfw)：跨平台窗口、输入和 OpenGL 上下文管理。

原始示例相关链接：

- [OCCT Issue #33485](https://tracker.dev.opencascade.org/view.php?id=33485)
- [Open CASCADE GitHub](https://github.com/Open-Cascade-SAS/OCCT)

![OCCT + ImGui](occt-imgui.png "OpenCASCADE + ImGui")
