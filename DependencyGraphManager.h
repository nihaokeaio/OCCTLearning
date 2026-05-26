//
// Created by ZQD on 26-5-22.
//

#pragma once
#include <chrono>
#include <gp_Pnt.hxx>
#include <queue>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

struct ComputerNode;
struct Render;
struct GraphExecutor;

class UniqueRandomGenerator
{
private:
    std::mt19937 rng;
    std::unordered_set<int> generated;
    int minValue;
    int maxValue;

public:
    UniqueRandomGenerator(int min, int max)
        : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
          minValue(min), maxValue(max)
    {
        if (max - min + 1 < 1)
        {
            throw std::runtime_error("范围太小，无法生成唯一值");
        }
    }

    // 生成下一个不重复的随机数
    int next()
    {
        if (generated.size() >= (maxValue - minValue + 1))
        {
            throw std::runtime_error("已生成所有可能的值，没有更多不重复的值");
        }

        std::uniform_int_distribution<int> dist(minValue, maxValue);
        int value;
        do
        {
            value = dist(rng);
        }
        while (generated.find(value) != generated.end());

        generated.insert(value);
        return value;
    }

    // 重置生成器
    void reset()
    {
        generated.clear();
    }

    // 获取已生成的数量
    size_t generatedCount() const
    {
        return generated.size();
    }

    // 检查是否还有未生成的值
    bool hasNext() const
    {
        return generated.size() < (maxValue - minValue + 1);
    }
};

static UniqueRandomGenerator generator(1, 100);


struct ComputerNodeId
{
    ComputerNodeId()
    {
        m_Id = generator.next();
    }

    bool operator==(const ComputerNodeId& other) const { return m_Id == other.m_Id; }

    int m_Id = 0;
};

struct ValueId
{
    ValueId()
    {
        m_Id = generator.next();
    }

    bool operator==(const ValueId& other) const { return m_Id == other.m_Id; }
    int m_Id;
};

struct ValueHandle
{
    virtual ~ValueHandle() = default;
};


// 需要特化 hash
namespace std
{
    template <>
    struct hash<ComputerNodeId>
    {
        size_t operator()(const ComputerNodeId& n) const noexcept
        {
            return hash<int>()(n.m_Id);
        }
    };

    template <>
    struct hash<ValueId>
    {
        size_t operator()(const ValueId& n) const noexcept
        {
            return hash<int>()(n.m_Id);
        }
    };
}

struct DGContext
{
    ComputerNode* GetComputerNode(const ComputerNodeId& id)
    {
        const auto iter = m_Nodes.find(id);
        if (iter != m_Nodes.end())
        {
            return iter->second.get();
        }
        return nullptr;
    }

    //std::unordered_map<NodeId, Element> m_Element;
    std::unordered_map<ValueId, std::shared_ptr<ValueHandle>> m_Position;
    std::unordered_map<ValueId, std::shared_ptr<ValueHandle>> m_Length;
    std::unordered_map<ComputerNodeId, std::unique_ptr<ComputerNode>> m_Nodes;
    std::unique_ptr<Render> render;
};

enum class DependencyType
{
    PositionToLength,
    LengthToRender
};

struct ComputerNode
{
    virtual ~ComputerNode() = default;
    virtual void Evaluator(DGContext& context) = 0;
    ComputerNodeId m_Id;
};

struct Dependency
{
    explicit Dependency(ComputerNodeId f, ComputerNodeId t): from(f), to(t)
    {
    }

    ComputerNodeId from;
    ComputerNodeId to;
};

enum DirtyFlags
{
    Dirty_None = 0,
    Dirty_Position = 1 << 1,
    Dirty_Length = 1 << 2,
    Dirty_Render = 1 << 3
};

struct DirtyNode
{
    ComputerNodeId id;
    DirtyFlags flags;
};


struct JointNode : ValueHandle
{
    gp_Pnt m_Position;
    ValueId m_Id;
};

struct SegmentNode : ValueHandle
{
    double m_Length;
    ValueId m_Id;
};

struct Render
{
    void Update(ValueId node)
    {
        std::cout << node.m_Id << "update !" << std::endl;
    }
};


struct GraphExecutor
{
    void MarkDirty(const ValueId id)
    {
        dirtyQueue.push(id);
    }

    bool Evaluate(DGContext& context)
    {
        while (!dirtyQueue.empty())
        {
            auto dirtyValueId = dirtyQueue.front();
            dirtyQueue.pop();
            const auto& iter = dependNodeLists.find(dirtyValueId);
            {
                for (auto& computerNodeId : iter->second)
                {
                    const auto node = context.m_Nodes[computerNodeId].get();
                    node->Evaluator(context);

                    // 图执行器负责传播：找到该节点的所有输出，标记为脏
                    auto outputIter = nodeOutputs.find(computerNodeId);
                    if (outputIter != nodeOutputs.end())
                    {
                        for (const auto& outputValueId : outputIter->second)
                        {
                            MarkDirty(outputValueId); // 自动级联传播
                        }
                    }
                }
            }
        }
        return false;
    }

    std::queue<ValueId> dirtyQueue;
    std::unordered_map<ValueId, std::vector<ComputerNodeId>> dependNodeLists;
    std::unordered_map<ComputerNodeId, std::vector<ValueId>> nodeOutputs;
};

struct PositionToLengthNode : ComputerNode
{
    PositionToLengthNode(const ValueId& inputA, const ValueId& inputB, const ValueId& output):
        m_InputA(inputA),
        m_InputB(inputB), m_Output(output)
    {
    }

    void Evaluator(DGContext& context) override
    {
        const auto& p0 = dynamic_cast<JointNode*>(context.m_Position[m_InputA].get());
        const auto& p1 = dynamic_cast<JointNode*>(context.m_Position[m_InputB].get());
        auto segmentNode = dynamic_cast<SegmentNode*>(context.m_Length[m_Output].get());
        segmentNode->m_Length = p0->m_Position.Distance(p1->m_Position);
    }

    ValueId m_InputA;
    ValueId m_InputB;
    ValueId m_Output;
};

struct LengthToRenderNode : ComputerNode
{
    LengthToRenderNode(const ValueId& input): m_Input(input)
    {
    }

    void Evaluator(DGContext& context) override
    {
        context.render->Update(m_Input);
    }

    ValueId m_Input;
};


class DependencyGraphManager
{
public:
    void Test()
    {
        ///数据层
        auto j0 = std::make_shared<JointNode>();
        auto j1 = std::make_shared<JointNode>();
        auto s0 = std::make_shared<SegmentNode>();
        m_Context.m_Position.insert({j0->m_Id, j0});
        m_Context.m_Position.insert({j1->m_Id, j1});
        m_Context.m_Length.insert({s0->m_Id, s0});
        m_Context.render = std::make_unique<Render>();


        ///数据流层
        auto pNode = std::make_unique<PositionToLengthNode>(j0->m_Id, j1->m_Id, s0->m_Id);
        auto lNode = std::make_unique<LengthToRenderNode>(s0->m_Id);

        ///数据元到计算节点的依赖图
        m_GraphExecutor.dependNodeLists.insert({j0->m_Id, {pNode->m_Id}});
        m_GraphExecutor.dependNodeLists.insert({j1->m_Id, {pNode->m_Id}});
        m_GraphExecutor.dependNodeLists.insert({s0->m_Id, {lNode->m_Id}});

        ///图到数据元影响图
        m_GraphExecutor.nodeOutputs.insert({pNode->m_Id, {s0->m_Id}});

        auto j0Ptr = j0.get();
        m_Context.m_Nodes.insert({pNode->m_Id, std::move(pNode)});
        m_Context.m_Nodes.insert({lNode->m_Id, std::move(lNode)});


        ///变更
        j0Ptr->m_Position = {100, 100, 100};
        m_GraphExecutor.MarkDirty(j0Ptr->m_Id);


        m_GraphExecutor.Evaluate(m_Context);
    }

public:
    GraphExecutor m_GraphExecutor;
    DGContext m_Context;
};



