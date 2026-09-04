#include <gtest/gtest.h>

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

TEST(BlockingQueue, ConsumerWaitsForProducer) {
  BlockQueue<std::unique_ptr<int>> queue;
  int value = 0;
  std::thread producer(
      [&] {
        queue.Push(std::make_unique<int>(1));
      });
  std::thread consumer(
      [&] {
        const auto vp = queue.Pop();
        value = *vp;
      });
  producer.join();
  consumer.join();
  EXPECT_EQ(value, 1);
}

TEST(BlockingQueue, ValuePushedBeforeWaitCanBeRead) {
}

TEST(BlockingQueue, ValuesArePoppedInFifoOrder) {
}
}// namespace Version8
