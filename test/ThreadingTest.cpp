#include <gtest/gtest.h>

#include <future>
#include <queue>
#include <queue>
#include <thread>

TEST(ThreadBasics, CreateAndJoin) {
  const std::thread::id mainThreadId = std::this_thread::get_id();

  std::thread::id workerThreadId;

  std::thread worker(
      [&workerThreadId] {
        workerThreadId = std::this_thread::get_id();
      });

  EXPECT_TRUE(worker.joinable());

  worker.join();
  EXPECT_FALSE(worker.joinable());
  EXPECT_NE(workerThreadId, mainThreadId);
}

TEST(ThreadBasics, ArgumentsAreCopiedByDefault) {
  // 按值传入，保存副本
  int value = 10;
  std::thread worker0(
      [](int v) {
        v = 20;
      }, value);
  worker0.join();
  EXPECT_EQ(value, 10);

  // 使用std::ref实现引用
  std::thread worker1(
      [](int &v) {
        v = 20;
      }, std::ref(value));
  worker1.join();
  EXPECT_EQ(value, 20);

  // 移动所有权
  auto source = std::make_unique<int>(30);
  std::thread worker2(
      [&value](std::unique_ptr<int> &&v) {
        value = *v;
      }, std::move(source));
  worker2.join();
  EXPECT_EQ(source, nullptr);
  EXPECT_EQ(value, 30);
}

TEST(ThreadSynchronization, MutexProtectsSharedCounter) {
  struct ThreadSafeCounter {
    void Increment() {
      std::lock_guard lock(m_Mutex);
      ++m_Value;
    }

    int GetValue() {
      std::lock_guard lock(m_Mutex);
      return m_Value;
    }

  private:
    int m_Value = 0;
    std::mutex m_Mutex;
  };

  ThreadSafeCounter counter;
  auto incrementManyTimes = [&counter] {
    for (int i = 0; i < 10000; ++i) {
      counter.Increment();
    }
  };
  std::thread thread0(incrementManyTimes);
  std::thread thread1(incrementManyTimes);

  thread0.join();
  thread1.join();
  EXPECT_EQ(counter.GetValue(), 20000);
}

// 死锁以及std::scope_lock的使用
namespace Version4 {
class Account {
public:
  explicit Account(const int balance) : m_Balance(balance) {
  }

  int Balance() {
    std::lock_guard lock(m_Mutex);
    return m_Balance;
  }

private:
  friend void Transfer(Account &from, Account &to, int amount);
  std::mutex m_Mutex;
  int m_Balance = 0;
};

void Transfer(Account &from, Account &to, int amount) {
  if (&from == &to)
    return;
  //std::scoped_lock 尝试持有多把锁，全部锁持有后开始保护共享资源，离开作用域时释放全部锁
  std::scoped_lock lock(from.m_Mutex, to.m_Mutex);
  //前面已经加锁，后续不应该再尝试调用Balance再次加锁
  from.m_Balance -= amount;
  to.m_Balance += amount;
}

TEST(MultipleMutexes, ScopedLockAvoidsDeadlock) {
  Account firstAccount{10000};
  Account secondAccount{10000};

  std::thread firstWorker(
      [&] {
        for (int i = 0; i < 1000; ++i) {
          Transfer(firstAccount, secondAccount, 1);
        }
      });

  std::thread secondWorker(
      [&] {
        for (int i = 0; i < 1000; ++i) {
          Transfer(secondAccount, firstAccount, 1);
        }
      });

  firstWorker.join();
  secondWorker.join();

  EXPECT_EQ(firstAccount.Balance(), 10000);
  EXPECT_EQ(secondAccount.Balance(), 10000);
}
}// namespace Version4

// 信号量
namespace Version5 {
class Demo {
public:
  void SetValue(int value) {
    {
      std::lock_guard lock(m_Mutex);
      m_Value = value;
    }
    m_CV.notify_one();
  }

  int Wait() {
    std::unique_lock lock(m_Mutex);
    m_CV.wait(
        lock, [this] {
          return m_Value.has_value();
        });
    return m_Value.value();
  }

private:
  std::mutex m_Mutex;
  std::condition_variable m_CV;
  std::optional<int> m_Value;
};

TEST(ConditionVariable, ConsumerWaitsForResult) {
  Demo demo;
  int value = 0;
  std::thread consumer(
      [&] {
        value = demo.Wait();
      });

  std::thread producer(
      [&] {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        demo.SetValue(10);
      });

  consumer.join();
  producer.join();

  EXPECT_EQ(value, 10);
}
}// namespace Version5

namespace Version6 {
TEST(Atomic, FetchAddDoesNotLoseUpdates) {
  constexpr int incrementCount = 10000;

  std::atomic<int> counter{0};

  auto incrementManyTimes = [&counter] {
    counter.fetch_add(1);
  };

  std::thread thread0(
      [&] {
        for (int i = 0; i < incrementCount; ++i)
          incrementManyTimes();
      });
  std::thread thread1(
      [&] {
        for (int i = 0; i < incrementCount; ++i)
          incrementManyTimes();
      });

  thread0.join();
  thread1.join();
  EXPECT_EQ(counter.load(), incrementCount * 2);
}
}// namespace Version6

namespace Version7 {
TEST(StopToken, SourceAndTokenShareState) {
  std::stop_source stopSource;
  std::stop_token firstToken = stopSource.get_token();
  std::stop_token secondToken = firstToken;
  bool callbackInvoked = false;
  auto callback = std::stop_callback(
      firstToken, [&callbackInvoked] {
        callbackInvoked = true;
      });

  EXPECT_TRUE(stopSource.stop_possible());
  EXPECT_FALSE(firstToken.stop_requested());
  EXPECT_FALSE(secondToken.stop_requested());
  EXPECT_FALSE(callbackInvoked);

  EXPECT_TRUE(stopSource.request_stop());
  EXPECT_TRUE(stopSource.stop_possible());
  EXPECT_TRUE(callbackInvoked);

  EXPECT_TRUE(firstToken.stop_requested());
  EXPECT_TRUE(secondToken.stop_requested());

  EXPECT_FALSE(stopSource.request_stop());
}

TEST(JThread, ExplicitStopRequest) {
  bool stopObserved = false;

  std::jthread worker(
      [&stopObserved](const std::stop_token token) {
        while (!token.stop_requested()) {
          std::this_thread::yield();
        }

        stopObserved = true;
      });
  //发出停止请求
  EXPECT_TRUE(worker.request_stop());
  worker.join();

  // 若交换，则程序会卡在工作线程中等待停止信号处
  // worker.join();
  // EXPECT_TRUE(worker.request_stop());

  EXPECT_TRUE(stopObserved);
}

TEST(JThread, DestructorRequestsStopAndJoins) {
  bool stopObserved = false;
  {
    std::jthread thread(
        [&stopObserved](const std::stop_token stopToken) {
          while (!stopToken.stop_requested()) {
            std::this_thread::yield();
          }
          stopObserved = true;
        });
  }
  EXPECT_TRUE(stopObserved);
}
}// namespace Version7

namespace Version8 {
template<typename T>
class BlockQueue {
public:
  void Push(T v) {
    {
      std::lock_guard lock(m_Mutex);
      m_Queue.push(std::move(v));
    }
    m_CV.notify_one();
  }

  T Pop() {
    std::unique_lock lock(m_Mutex);
    m_CV.wait(
        lock, [this] {
          return !m_Queue.empty();
        });
    T v = std::move(m_Queue.front());
    m_Queue.pop();
    return v;
  }

private:
  std::queue<T> m_Queue;
  std::mutex m_Mutex;
  std::condition_variable m_CV;
};

TEST(BlockingQueue, MoveOnlyValueCanBeTransferredBetweenThreads) {
  BlockQueue<std::unique_ptr<int>> queue;
  int value = 0;
  std::thread producer(
      [&] {
        queue.Push(std::make_unique<int>(1));
      });
  std::thread consumer(
      [&] {
        auto vp = queue.Pop();
        value = *vp;
      });
  producer.join();
  consumer.join();
  EXPECT_EQ(value, 1);
}

//先发送，再监听
TEST(BlockingQueue, ValuePushedBeforeWaitCanBeRead) {
  BlockQueue<std::unique_ptr<int>> queue;
  queue.Push(std::make_unique<int>(10));
  int value = 0;
  // 消费时queue已经有数据，故不再等待
  std::thread consumer(
      [&] {
        const auto vp = queue.Pop();
        value = *vp;
      });
  consumer.join();
  EXPECT_EQ(value, 10);
}

TEST(BlockingQueue, ValuesArePoppedInFifoOrder) {
  BlockQueue<int> queue;
  queue.Push(1);
  queue.Push(2);
  queue.Push(3);
  EXPECT_EQ(queue.Pop(), 1);
  EXPECT_EQ(queue.Pop(), 2);
  EXPECT_EQ(queue.Pop(), 3);
}
}// namespace Version8

namespace Version9 {
template<typename T>
class BlockQueue {
public:
  void Push(T v) {
    {
      std::lock_guard lock(m_Mutex);
      if (m_Closed) {
        throw std::runtime_error("Blocking queue is closed");
      }
      m_Queue.push(std::move(v));
    }
    m_CV.notify_one();
  }

  std::optional<T> Pop() {
    std::unique_lock lock(m_Mutex);
    m_CV.wait(
        lock, [this] {
          return m_Closed || !m_Queue.empty();
        });
    if (m_Queue.empty())
      return std::nullopt;
    T v = std::move(m_Queue.front());
    m_Queue.pop();
    return v;
  }

  void Close() {
    {
      std::lock_guard lock(m_Mutex);
      m_Closed = true;
    }
    m_CV.notify_all();
  }

private:
  std::queue<T> m_Queue;
  std::mutex m_Mutex;
  std::condition_variable m_CV;
  bool m_Closed = false;
};

TEST(CloseableBlockingQueue, CloseOnEmptyQueueReturnsNullopt) {
  BlockQueue<std::unique_ptr<int>> queue;
  int value = 0;
  std::optional<std::unique_ptr<int>> ret;
  std::thread consumer(
      [&] {
        ret = queue.Pop();
        if (ret.has_value())
          value = *ret.value();
      });
  queue.Close();
  consumer.join();
  EXPECT_FALSE(ret.has_value());
  EXPECT_EQ(value, 0);
}

TEST(CloseableBlockingQueue, CloseOnPopElement) {
  BlockQueue<int> queue;
  queue.Push(1);
  queue.Push(2);
  queue.Push(3);
  queue.Close();
  EXPECT_EQ(queue.Pop(), 1);
  EXPECT_EQ(queue.Pop(), 2);
  EXPECT_EQ(queue.Pop(), 3);
  // 所有元素弹出后，返回std::nullopt
  EXPECT_FALSE(queue.Pop().has_value());
  // 关闭后拒绝push
  EXPECT_THROW(queue.Push(4), std::runtime_error);
}

}


namespace Version10 {

using Task = std::function<void()>;

class EventLoop {
public:
  void Post(Task task) {
    m_Queue.Push(std::move(task));
  }

  void Run() {
    while (true) {
      auto task = m_Queue.Pop();
      if (!task.has_value())
        return;
      task.value()();
    }
  }

  void Stop() {
    m_Queue.Close();
  }

private:
  Version9::BlockQueue<Task> m_Queue;
};

/// 任务队列中的任务在事件循环线程中执行
TEST(EventLoop, PostedTaskRunsOnLoopThread) {
  EventLoop loop;
  std::thread::id id0;
  std::thread::id id1;
  std::thread::id id2;
  std::thread::id mainLoopId;
  loop.Post(
      [&] {
        id0 = std::this_thread::get_id();
      });
  loop.Post(
      [&] {
        id1 = std::this_thread::get_id();
      });
  loop.Post(
      [&] {
        id2 = std::this_thread::get_id();
      });

  std::jthread worker(
      [&] {
        mainLoopId = std::this_thread::get_id();
        loop.Run();
      });
  loop.Stop();
  worker.join();
  EXPECT_EQ(mainLoopId, id0);
  EXPECT_EQ(mainLoopId, id1);
  EXPECT_EQ(mainLoopId, id2);
}

/// 调度顺序以加入任务队列为序
TEST(EventLoop, TasksRunInPostingOrder) {
  EventLoop loop;
  std::vector<int> orders;
  loop.Post(
      [&] {
        orders.push_back(1);
      });
  loop.Post(
      [&] {
        orders.push_back(2);
      });
  loop.Post(
      [&] {
        orders.push_back(3);
      });

  std::jthread worker(
      [&] {
        loop.Run();
      });
  loop.Stop();
  worker.join();
  EXPECT_EQ(orders, std::vector<int>({ 1, 2, 3 }));
}

/// 调度循环关闭后，积压任务仍可继续执行
TEST(EventLoop, StopDrainsPostedTasksAndExits) {
  EventLoop loop;
  std::vector<int> orders;
  loop.Post(
      [&] {
        orders.push_back(1);
      });
  loop.Post(
      [&] {
        orders.push_back(2);
      });
  loop.Post(
      [&] {
        orders.push_back(3);
      });
  loop.Stop();
  loop.Run();
  EXPECT_EQ(orders, std::vector<int>({ 1, 2, 3 }));
}

}

namespace Version11 {
using Task = std::function<void()>;

class EventLoopThread {
public:
  EventLoopThread() {
    m_Thread = std::jthread(
        [this] {
          m_Loop.Run();
        });
  }

  ~EventLoopThread() {
    m_Loop.Stop();
  }

  void Post(Task task) {
    m_Loop.Post(std::move(task));
  }

  EventLoopThread(const EventLoopThread &) = delete;
  EventLoopThread &operator=(const EventLoopThread &) = delete;
  EventLoopThread(EventLoopThread &&) = delete;
  EventLoopThread &operator=(EventLoopThread &&) = delete;

private:
  Version10::EventLoop m_Loop;
  std::jthread m_Thread;
};

TEST(EventLoopThread, DestructorDrainsPostedTasks) {
  std::vector<int> orders;
  // 析构后，loop正常join，队列循环可以正常退出
  {
    EventLoopThread loop;
    loop.Post(
        [&] {
          orders.push_back(1);
        });
    loop.Post(
        [&] {
          orders.push_back(2);
        });
    loop.Post(
        [&] {
          orders.push_back(3);
        });
  }
  EXPECT_EQ(orders, std::vector<int>({ 1, 2, 3 }));
}

TEST(EventLoopThread, EmptyWorkerCanBeDestroyed) {
  {
    EventLoopThread loop;
  }
  //即便是空的队列，析构后，程序可以继续正常进行
  SUCCEED();
}

TEST(EventLoopThread, TaskResultCanBeReadBeforeWorkerDestruction) {
  EventLoopThread loop;
  std::promise<int> promise;
  std::future<int> future = promise.get_future();
  loop.Post(
      [&promise] {
        promise.set_value(10);
      });
  EXPECT_EQ(future.get(), 10);
  //loop无需析构，仍可以继续队列
}
}
