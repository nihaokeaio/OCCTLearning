//
// Created by ZQD on 26-8-6.
//
#include <gtest/gtest.h>
#include "SignalConnectManager.h"

// 测试成员函数的调用
TEST(MiniSignal, CallsMemberSlot)
{
    using namespace MiniSignal;
    struct Receiver : Trackable
    {
        int count = 0;
        void OnSig(const int x) { count = x; }
    };
    struct Sender
    {
        Signal<int> sig;
    };
    Receiver receiver;
    Sender sender;
    connect(&sender, &Sender::sig, &receiver, &Receiver::OnSig);
    sender.sig.emit(5);
    EXPECT_EQ(receiver.count, 5);
}

// 测试匿名函数的调用
TEST(MiniSignal, CallsLambdaSlot)
{
    using namespace MiniSignal;
    int count = 0;
    struct Sender
    {
        Signal<int> sig;
    };
    Sender sender;
    connect(&sender, &Sender::sig, [&count](int x)
    {
        count = x;
    });
    sender.sig.emit(5);
    EXPECT_EQ(count, 5);
}

TEST(MiniSignal, ManualDisconnect)
{
    using namespace MiniSignal;
    int count = 0;
    struct Sender
    {
        Signal<int> sig;
    };
    Sender sender;
    auto c = connect(&sender, &Sender::sig, [&count](int x)
    {
        count = x;
    });
    sender.sig.emit(5);
    EXPECT_EQ(count, 5);
    //断开链接
    c->DisConnect();
    sender.sig.emit(10);
    EXPECT_EQ(count, 5);
}

TEST(MiniSignal, ScopedDisconnect)
{
    using namespace MiniSignal;
    int count = 0;
    struct Sender
    {
        Signal<int> sig;
    };
    Sender sender;
    {
        auto c = connectScope(&sender, &Sender::sig, [&count](int x)
        {
            count = x;
        });
        sender.sig.emit(5);
        EXPECT_EQ(count, 5);
    }
    sender.sig.emit(10);
    EXPECT_EQ(count, 5);
}

// 测试接收者析构后自动断开连接
TEST(MiniSignal, ReceiverDestructionDisconnects)
{
    using namespace MiniSignal;
    struct Receiver : Trackable
    {
        int count = 0;
        void OnSig(const int x) { count = x; }
    };
    struct Sender
    {
        Signal<int> sig;
    };
    Sender sender;
    {
        Receiver receiver;
        connect(&sender, &Sender::sig, &receiver, &Receiver::OnSig);
        sender.sig.emit(5);
        EXPECT_EQ(receiver.count, 5);
    }
    sender.sig.emit(10);
    SUCCEED();
}

// 测试发送者，接收者析构后，连接能够正常调用不UB
TEST(MiniSignal, SenderDestructionDoesNotBreakConnection)
{
    using namespace MiniSignal;
    struct Receiver : Trackable
    {
        int count = 0;
        void OnSig(const int x) { count = x; }
    };
    struct Sender
    {
        Signal<int> sig;
    };
    std::shared_ptr<Connection> connection;

    Receiver receiver;
    {
        Sender sender;
        connection = connect(&sender, &Sender::sig, &receiver, &Receiver::OnSig);
        sender.sig.emit(5);
        EXPECT_EQ(receiver.count, 5);
    }
    EXPECT_NO_THROW(connection->DisConnect());
}

/// emit 过程中断开后续 slot，本轮跳过
TEST(MiniSignal, DisconnectDuringEmitSkipsLaterSlot)
{
    using namespace MiniSignal;
    struct Sender
    {
        Signal<int> sig;
    };

    Sender sender;
    std::shared_ptr<Connection> c0;
    int count = 0;
    auto c1 = connect(&sender, &Sender::sig, [&c0](int x)
    {
        c0->DisConnect();
    });
    c0 = connect(&sender, &Sender::sig, [&count](int x)
    {
        count = x;
    });
    sender.sig.emit(5);
    EXPECT_EQ(count, 0);
}

/// 在执行槽函数过程中创建新的连接
TEST(MiniSignal, ConnectDuringEmitRunsNextEmit)
{
    using namespace MiniSignal;
    struct Sender
    {
        Signal<int> sig;
    };

    Sender sender;

    int count = 0;
    connect(&sender, &Sender::sig, [&sender, &count](int x)
    {
        count = x;
        connect(&sender, &Sender::sig, [&count](int x)
        {
            count = -x;
        });
    });
    sender.sig.emit(5);
    EXPECT_EQ(count, 5);
    sender.sig.emit(10);
    EXPECT_EQ(count, -10);
}
