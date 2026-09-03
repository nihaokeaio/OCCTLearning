# OcctImgui
OpenCASCADE + GLFW + IMGUI Sample.

## ThreadEventLoop 学习分支

本分支从 `ObjectRuntime` 创建，保留对象树与信号槽的融合代码，同时新增一个独立的 ISO C++20 线程学习模块。课程目标不是罗列线程
API，而是逐步实现能够与现有对象系统融合的事件循环和 queued signal-slot。

当前完成第一课：线程生命周期、RAII 与协作取消。

- 可复用实现：[`Threading/StoppableWorker.h`](Threading/threadWorker.h)
- GoogleTest 课程测试：[`test/ThreadingTest.cpp`](test/ThreadingTest.cpp)
- 学习记录：[`docs/threading-learning-notes.md`](docs/threading-learning-notes.md)

测试目标：

```powershell
cmake -S . -B cmake-build-debug-visual-studio -DBUILD_TESTING=ON
cmake --build cmake-build-debug-visual-studio --target ThreadingTest --config Debug
ctest --test-dir cmake-build-debug-visual-studio -C Debug --output-on-failure
```

![occt imgui](occt-imgui.png "opencascade imgui")

https://tracker.dev.opencascade.org/view.php?id=33485

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


