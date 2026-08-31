# OcctImgui
OpenCASCADE + GLFW + IMGUI Sample.

![occt imgui](occt-imgui.png "opencascade imgui")

https://tracker.dev.opencascade.org/view.php?id=33485

## modernCpp 分支

这个分支在原有 OcctImgui 示例之外，增加了一组以 GoogleTest 驱动的 Modern C++ 模板学习代码。示例按照 `Version1` 到 `Version8` 逐步演进，覆盖：

- 模板参数推导、左值/右值、引用折叠与完美转发；
- 类模板、全特化、偏特化和 `if constexpr`；
- 自定义类型萃取、concept 与 requires；
- 参数包、折叠表达式、tuple 与 `index_sequence`；
- 普通函数、成员函数、函数对象及 lambda 的 `FunctionTraits`。

学习代码位于 [`test/ModernTemplateTest.cpp`](test/ModernTemplateTest.cpp)，完整的版本演进、关键结论和复习建议见 [`docs/modern-cpp-template-learning.md`](docs/modern-cpp-template-learning.md)。

模板测试目标使用 C++23，可单独构建和运行：

```powershell
cmake -S . -B cmake-build-debug-visual-studio -DBUILD_TESTING=ON
cmake --build cmake-build-debug-visual-studio --target ModernCPPTest --config Debug
ctest --test-dir cmake-build-debug-visual-studio -C Debug --output-on-failure
```

## OpenCASCADE
  https://dev.opencascade.org/
  
  https://github.com/Open-Cascade-SAS/OCCT

  Open CASCADE Technology (OCCT) a software
development platform providing services for 3D surface and solid modeling, CAD 
data exchange, and visualization. Most of OCCT functionality is available in 
the form of C++ libraries. OCCT can be best applied in development of software 
dealing with 3D modeling (CAD), manufacturing / measuring (CAM) or numerical 
simulation (CAE).
  
## IMGUI
  https://github.com/ocornut/imgui

  Dear ImGui is a bloat-free graphical user interface library for C++. It outputs optimized vertex buffers that you can render anytime in your 3D-pipeline-enabled application. It is fast, portable, renderer agnostic, and self-contained (no external dependencies).

Dear ImGui is designed to enable fast iterations and to empower programmers to create content creation tools and visualization / debug tools (as opposed to UI for the average end-user). It favors simplicity and productivity toward this goal and lacks certain features commonly found in more high-level libraries.

Dear ImGui is particularly suited to integration in game engines (for tooling), real-time 3D applications, fullscreen applications, embedded applications, or any applications on console platforms where operating system features are non-standard.

## GLFW
  https://github.com/glfw/glfw

  GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development. It provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems. On Linux both X11 and Wayland are supported.

GLFW is licensed under the zlib/libpng license.

## Build
Use Premake5 to build OcctImgui or with CMake:

```
cmake -DCMAKE_CXX_STANDARD=17 ..
```


