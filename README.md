# OcctImgui — MetaObject 学习分支

这是 `OcctImgui` 项目的 **MetaObject 学习分支**。

本分支受 Qt 元对象系统启发，目标不是直接使用 Qt，也不是完整复刻 `QMetaObject`，而是只使用 ISO C++20，从一个只能报告类名的最小原型开始，逐步理解运行时类型描述为什么存在，以及对象工厂、属性系统、动态方法调用、元类型和序列化怎样建立在它之上。

核心代码保留了 `Version1`～`Version17` 的完整演进过程。每一版都不是简单增加功能，而是由上一版暴露出的具体问题推动。

## 为什么要做这个分支？

C++ 编译器知道一个类有哪些成员和函数，但程序运行后，如果调用者只有下面这些动态数据：

```text
class = Button
property = M_Length
value = 3.0
method = F_SetPosition
```

普通 C++ 不会自动提供以下能力：

- 根据类名查询类型和继承关系；
- 根据类名创建一个具体派生对象；
- 根据属性名读取、修改对象；
- 根据方法名和运行时参数调用成员函数；
- 查询方法参数、返回值和 `const` 信息；
- 在不知道具体派生类型的地方完成对象序列化。

本分支尝试用模板、宏、成员指针、`std::function`、`std::any`、RTTI 和类型擦除逐层构建这些能力，从而学习 Qt 元对象系统背后的 C++ 思想。

## 版本演进

| 阶段 | 版本 | 核心问题与结果 |
|---|---:|---|
| 类型描述 | V1 | 建立 `MetaObject`，支持类名、父类和继承关系查询 |
| 动态创建 | V2～V3 | 建立名字到工厂的注册；区分“类型已注册”和“类型可以实例化” |
| 样板收束 | V4 | 使用宏统一生成静态元对象和虚函数入口 |
| 属性系统 | V5～V6 | 将成员变量或 getter/setter 映射为统一的 `MetaProperty::Read/Write` |
| 动态调用 | V7～V9 | 从无参方法推进到可变参数模板和运行时参数列表 |
| 返回与重载 | V10～V11 | 支持返回值、`void`、同名重载和 `const` 成员函数 |
| 开放元类型 | V12 | 用 `std::any + MetaType` 取代封闭的 `variant` 类型列表 |
| 自我描述 | V13 | 查询属性类型、方法参数和返回值，并输出完整方法签名 |
| 类型注册 | V14 | 建立唯一、稳定、可按名字查询的 `MetaType` 注册表 |
| 字符串序列化 | V15 | 将每种 C++ 类型的编解码能力注册到 `MetaType` |
| 通用数据层 | V16 | 引入 `SerializedValue`，将业务类型映射与 JSON 等外部格式解耦 |
| 对象级闭环 | V17 | 基于元属性完成整个对象的序列化、动态创建与反序列化 |

详细的设计过程、每一版遭遇的问题以及为什么必须继续演化，见：

**[从类名到对象序列化：从零实现 C++ 元对象系统的 17 次演进](meta-object-system-learning-notes.md)**

## Version 17 最终结构

```text
Object 实例
└── MetaObject：描述类名、父类、属性和方法
    ├── MetaProperty：统一读取和写入对象状态
    └── MetaMethod：统一匹配并调用成员函数

MetaProperty / MetaMethod
└── MetaType：类型身份、稳定名字和编解码能力
    └── MetaValue：携带类型擦除后的运行时 C++ 值

RegisterObject
└── className → MetaObject + 对象创建函数

SerializedObject
└── propertyName → typeName + SerializedValue
```

最终可以在不知道对象具体派生类型的情况下完成：

```text
运行时类名
→ 创建具体对象
→ 查找继承属性
→ 写入运行时值
→ 序列化整个对象
→ 根据快照重新创建并恢复对象
```

这里的“运行时”不是生成新的 C++ 类型或函数，而是根据运行时名字和数据，选择已经在编译期注册的类型、成员指针、工厂和编解码器。

## 主要文件

```text
MetaObjectManager.h
    Version1～Version17 的完整学习实现与历史测试代码

CommonTraits.h
    成员函数 traits 等模板辅助设施

tests/MetaObjectTests.cpp
    Version17 的 GoogleTest 测试入口

meta-object-system-learning-notes.md
    完整学习过程和设计总结

CMakeLists.txt
    主程序与 GoogleTest 测试目标配置
```

历史版本被有意保留在不同命名空间中，便于直接比较同一问题在不同阶段的解法。当前相对完整的教学版本位于：

```cpp
namespace Version17
{
    // ...
}
```

## 测试

本分支引入 GoogleTest，将最终版本的行为测试与 GUI 主程序分离。

当前 CMake 工程需要：

- CMake 3.15 或更高版本；
- 支持 C++20 的编译器；
- 已可被 `find_package` 找到的 GLFW3 与 OpenCASCADE；
- 首次配置时可访问网络，以便 `FetchContent` 获取 GoogleTest。

测试重点包括：

- 注册并按名字创建对象；
- 通过元属性写入状态；
- 序列化派生类及其继承属性；
- 根据对象快照恢复实例；
- 核验恢复后的属性值；
- 逐步补充未知类、类型不匹配和畸形数据等错误路径。

构建并运行测试：

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

首次配置时，CMake 会通过 `FetchContent` 获取 GoogleTest。

也可以使用 IDE 测试面板，运行全部测试或单独运行某个：

```cpp
TEST(ObjectSerializationTest, RoundTripsButton)
```

## 当前边界

这是一个用于理解原理的教学实现，目前有意没有覆盖：

- 带参数构造函数的动态选择；
- 枚举、类信息和完整构造函数元数据；
- 方法调用时的隐式类型转换；
- 容器、指针和循环对象图的自动序列化；
- 序列化版本和字段迁移；
- 线程安全的动态注册；
- `moc` 一类的自动代码生成；
- 完整 Qt 元对象系统的线程与事件循环语义。

这些内容可以在真实需求出现后继续演化，而不是提前把教学系统扩张为框架。

## 原项目

本学习分支建立在 `OcctImgui` 示例项目之上。原项目组合了：

- [Open CASCADE Technology](https://dev.opencascade.org/)：三维几何建模、CAD 数据交换与可视化能力；
- [Dear ImGui](https://github.com/ocornut/imgui)：即时模式图形界面；
- [GLFW](https://github.com/glfw/glfw)：跨平台窗口、输入和 OpenGL 上下文管理。

原始示例相关链接：

- [OCCT Issue #33485](https://tracker.dev.opencascade.org/view.php?id=33485)
- [Open CASCADE GitHub](https://github.com/Open-Cascade-SAS/OCCT)

![OCCT + ImGui](occt-imgui.png "OpenCASCADE + ImGui")
