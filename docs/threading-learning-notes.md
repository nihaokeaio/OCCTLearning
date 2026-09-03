# C++ 线程学习记录

## 课程主线

这个分支从 `ObjectRuntime` 创建，因此保留对象树与信号槽的融合代码。但线程模块先保持独立，按照下面的顺序演进：

```text
线程生命周期
→ 协作取消
→ 阻塞任务队列
→ 事件循环与 Executor
→ 对象的线程归属
→ queued signal-slot
→ 线程池与真实项目集成
```

代码采用“可复用头文件 + GoogleTest 可执行课程”的结构。只用于观察语言或标准库语义的代码留在测试中；能够形成稳定职责的组件放入 `Threading/`。

## Lesson 1：线程生命周期、RAII 与协作取消

第一课对应：

- [`Threading/StoppableWorker.h`](../Threading/threadWorker.h)
- [`test/ThreadingTest.cpp`](../test/ThreadingTest.cpp)

### 1. `std::thread` 对象不是线程函数

线程函数在新的执行流中运行，`std::thread` 对象则是该执行流的所有权句柄。一个可联结的 `std::thread` 在析构前必须选择：

```cpp
worker.join();   // 等待线程结束
worker.detach(); // 放弃所有权，让线程独立运行
```

若可联结的 `std::thread` 直接析构，程序会调用 `std::terminate`。本课程的架构代码不使用 `detach`，因为 detached thread 很难与对象生命周期和程序退出建立可靠关系。

### 2. `join` 同时提供等待和同步

`join()` 不只是“等一会儿”。工作线程结束前的写入 happens-before `join()` 返回后的读取。因此测试可以在线程写入普通 `bool`、主线程 join 后再读取，而不构成数据竞争。

这不代表普通 `bool` 可以被两个仍在并发运行的线程同时读写；安全来自明确的 join 边界。

### 3. `std::jthread` 把所有权收进 RAII

`std::jthread` 的析构过程可以建立下面的心智模型：

```text
request_stop()
→ join()
```

这使异常、提前 return 和多条控制路径下的资源清理更加可靠。它仍然要求避免在工作线程内部销毁代表自身的 `jthread`，因为线程不能 join 自己。

### 4. stop request 不是强制终止

`request_stop()` 只修改共享停止状态。任务必须主动观察：

```cpp
while (!stopToken.stop_requested())
{
    DoOneSmallStep();
}
```

因此任务需要定期到达取消点。一次长时间、不可中断的阻塞调用不会因为 stop request 自动返回。

这种机制称为协作取消。它允许任务在退出前恢复不变量、释放资源或发布最终状态，避免强制杀线程破坏进程内共享数据。

`request_stop()` 第一次真正把状态从“未请求”改成“已请求”时返回 `true`；重复请求返回 `false`。停止请求是幂等的，但它是否已经被任务观察到，仍要通过 join 或其他同步协议确认。

### 5. `StoppableWorker` 为什么很薄？

`StoppableWorker` 没有尝试重新实现线程库。它只固定本课程后续组件需要的策略：

- 任务必须接收 `std::stop_token`；
- 工作线程由一个不可复制、可以移动的对象独占；
- 可以显式 `RequestStop`、`Join` 或 `StopAndJoin`；
- 忘记显式停止时，析构仍由 `std::jthread` 完成 stop + join。

后续 EventLoop 会复用相同的所有权与退出语义。

### 6. 为什么测试不使用 `sleep_for`？

“睡 10 毫秒，猜工作线程应该已经启动”会让测试依赖机器负载和调度时机。本课使用 `std::latch` 建立确定的同步点：

```text
工作线程启动
→ count_down
→ 测试线程 wait 返回
→ 发出 stop request
```

`yield()` 目前只用于极短的停止观察循环。下一课实现条件变量与阻塞队列后，工作线程将不再忙等。

## 第一课复习题

1. 为什么可联结的 `std::thread` 不能直接析构？
2. `join()` 除了等待，还建立了什么内存可见性关系？
3. `std::jthread` 析构时做了哪两件事？
4. 为什么 `request_stop()` 不能保证任务立刻退出？
5. 为什么本课程不把 `detach()` 作为后台任务方案？
6. 为什么并发测试不应该依赖固定时间的 `sleep_for`？

## 构建与运行

```powershell
cmake -S . -B cmake-build-debug-visual-studio -DBUILD_TESTING=ON
cmake --build cmake-build-debug-visual-studio --target ThreadingTest --config Debug
ctest --test-dir cmake-build-debug-visual-studio -C Debug --output-on-failure
```

下一课将实现一个泛型 `BlockingQueue<T>`，学习 `mutex`、`unique_lock`、`condition_variable_any`、谓词等待以及停止时如何唤醒阻塞线程。
