//
// Created by ZQD on 26-5-22.
//

#pragma once
#include <chrono>
#include <functional>
#include <gp_Pnt.hxx>
#include <memory>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <utility>
#include <variant>
#include <vector>

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


struct PropertyValue
{
    using Storage = std::variant<std::monostate, bool, int, double, gp_Pnt, std::string>;

    PropertyValue() = default;
    PropertyValue(int value);
    PropertyValue(double value);
    PropertyValue(bool value);
    PropertyValue(const std::string& value);
    PropertyValue(const gp_Pnt& value);
    ~PropertyValue() = default;

    [[nodiscard]] const Storage& Get() const
    {
        return m_Value;
    }

    template <class T>
    [[nodiscard]] const T& As() const
    {
        return std::get<T>(m_Value);
    }

    template <class T>
    [[nodiscard]] bool Is() const
    {
        return std::holds_alternative<T>(m_Value);
    }

    void Set(const Storage& val)
    {
        m_Value = val;
    }

    template <class T>
    void Set(const T& val)
    {
        m_Value = val;
    }

    Storage m_Value;
};


// 带有属性系统的ValueHandle
struct ValueHandle
{
    using PropertyChangedCallback = std::function<void(ValueHandle& valueHandle,
                                                       const std::string& name,
                                                       const PropertyValue& newVal,
                                                       const PropertyValue& oldVal)>;

    virtual ~ValueHandle() = default;


    template <class T>
    void AddProperty(const std::string& name, const T& initialValue)
    {
        properties.insert_or_assign(name, PropertyValue(initialValue));
    }

    void AddProperty(const std::string& name, const PropertyValue& initialValue)
    {
        properties.insert_or_assign(name, initialValue);
    }


    template <class T>
    [[nodiscard]] T GetProperty(const std::string& name) const
    {
        const auto it = properties.find(name);
        if (it == properties.end())
        {
            throw std::runtime_error("Property does not exist: " + name);
        }
        return it->second.As<T>();
    }

    [[nodiscard]] bool HasProperty(const std::string& name) const
    {
        return properties.find(name) != properties.end();
    }

    template <class T>
    void SetProperty(const std::string& name, const T& value)
    {
        const auto it = properties.find(name);
        if (it == properties.end())
        {
            throw std::runtime_error("Property does not exist: " + name);
        }

        const PropertyValue oldValue = it->second;
        it->second.Set(value);
        NotifyPropertyChanged(name, it->second, oldValue);
    }

    void SetProperty(const std::string& name, const PropertyValue& value);

    void SetOnSetProperty(PropertyChangedCallback fun)
    {
        m_OnSetFun = std::move(fun);
    }

    ValueId m_Id;
    std::unordered_map<std::string, PropertyValue> properties;
    PropertyChangedCallback m_OnSetFun;

private:
    void NotifyPropertyChanged(const std::string& name, const PropertyValue& newVal, const PropertyValue& oldVal);
};

struct DGContext
{
    DGContext();

    ComputerNode* GetComputerNode(const ComputerNodeId& id)
    {
        const auto iter = m_Nodes.find(id);
        if (iter != m_Nodes.end())
        {
            return iter->second.get();
        }
        return nullptr;
    }

    void AddComputerNode(std::unique_ptr<ComputerNode>&& node, const std::vector<std::shared_ptr<ValueHandle>>& inputs,
                         const std::vector<std::shared_ptr<ValueHandle>>& outputs);

    void AddValueHandle(const std::shared_ptr<ValueHandle>& valueHandle);

    template <class T>
    std::shared_ptr<ValueHandle> CreateValue(const std::string& propertyName, const T& initialValue);

    template <class T>
    void SetValueProperty(const std::shared_ptr<ValueHandle>& valueHandle, const std::string& propertyName,
                          const T& value);

    template <class T>
    void SetValueProperty(const ValueId& valueId, const std::string& propertyName, const T& value);

    void Evaluator();

    std::unordered_map<ValueId, std::shared_ptr<ValueHandle>> m_Values;
    std::unordered_map<ComputerNodeId, std::unique_ptr<ComputerNode>> m_Nodes;
    std::unique_ptr<Render> render;
    std::unique_ptr<GraphExecutor> m_GraphExecutor;
};


struct ComputerNode
{
    virtual ~ComputerNode() = default;
    virtual void Evaluator(DGContext& context) = 0;
    ComputerNodeId m_Id;
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

    bool Evaluate(DGContext* context)
    {
        while (!dirtyQueue.empty())
        {
            auto dirtyValueId = dirtyQueue.front();
            dirtyQueue.pop();
            const auto& iter = dependNodeLists.find(dirtyValueId);
            if (iter == dependNodeLists.end())
            {
                continue;
            }
            for (auto& computerNodeId : iter->second)
            {
                const auto node = context->m_Nodes[computerNodeId].get();
                node->Evaluator(*context);

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
        return false;
    }

    std::queue<ValueId> dirtyQueue;
    std::unordered_map<ValueId, std::vector<ComputerNodeId>> dependNodeLists;
    std::unordered_map<ComputerNodeId, std::vector<ValueId>> nodeOutputs;
};

template <class T>
std::shared_ptr<ValueHandle> DGContext::CreateValue(const std::string& propertyName, const T& initialValue)
{
    auto valueHandle = std::make_shared<ValueHandle>();
    valueHandle->AddProperty(propertyName, initialValue);
    AddValueHandle(valueHandle);
    return valueHandle;
}

template <class T>
void DGContext::SetValueProperty(const std::shared_ptr<ValueHandle>& valueHandle, const std::string& propertyName,
                                 const T& value)
{
    valueHandle->SetProperty(propertyName, value);
    m_GraphExecutor->MarkDirty(valueHandle->m_Id);
}

template <class T>
void DGContext::SetValueProperty(const ValueId& valueId, const std::string& propertyName, const T& value)
{
    const auto iter = m_Values.find(valueId);
    if (iter == m_Values.end())
    {
        throw std::runtime_error("ValueHandle does not exist");
    }
    SetValueProperty(iter->second, propertyName, value);
}

struct PositionToLengthNode : ComputerNode
{
    using SmartPtr = std::shared_ptr<ValueHandle>;

    PositionToLengthNode(const ValueId& inputA, const ValueId& inputB, const ValueId& output):
        m_InputA(inputA),
        m_InputB(inputB), m_Output(output)
    {
    }

    void Evaluator(DGContext& context) override
    {
        const auto& v0 = context.m_Values[m_InputA];
        const auto& v1 = context.m_Values[m_InputB];
        const auto& v2 = context.m_Values[m_Output];
        computeFunc(v0, v1, v2);
    }

    ValueId m_InputA;
    ValueId m_InputB;
    ValueId m_Output;
    std::function<void(const SmartPtr& v0, const SmartPtr& v1, const SmartPtr& v2)> computeFunc;
};

struct LengthToRectArea : ComputerNode
{
    using SmartPtr = std::shared_ptr<ValueHandle>;

    LengthToRectArea(const ValueId& inputA, const ValueId& inputB, const ValueId& output):
        m_InputA(inputA),
        m_InputB(inputB), m_Output(output)
    {
    }

    void Evaluator(DGContext& context) override
    {
        const auto& v0 = context.m_Values[m_InputA];
        const auto& v1 = context.m_Values[m_InputB];
        const auto& v2 = context.m_Values[m_Output];
        computeFunc(v0, v1, v2);
    }

    ValueId m_InputA;
    ValueId m_InputB;
    ValueId m_Output;
    std::function<void(const SmartPtr& v0, const SmartPtr& v1, const SmartPtr& v2)> computeFunc;
};

struct LengthToCircleArea : ComputerNode
{
    using SmartPtr = std::shared_ptr<ValueHandle>;

    LengthToCircleArea(const ValueId& inputA, const ValueId& output):
        m_InputA(inputA), m_Output(output)
    {
    }

    void Evaluator(DGContext& context) override
    {
        const auto& v0 = context.m_Values[m_InputA];
        const auto& v1 = context.m_Values[m_Output];
        computeFunc(v0, v1);
    }

    ValueId m_InputA;
    ValueId m_Output;
    std::function<void(const SmartPtr& v0, const SmartPtr& v1)> computeFunc;
};

struct LengthToRenderNode : ComputerNode
{
    explicit LengthToRenderNode(const ValueId& input): m_Input(input)
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
        auto j0 = m_Context.CreateValue("position", gp_Pnt(0, 0, 0));
        auto j1 = m_Context.CreateValue("position", gp_Pnt(100, 0, 0));

        ///数据层
        auto j2 = m_Context.CreateValue("position", gp_Pnt(100, 100, 0));

        auto s0 = m_Context.CreateValue("length", 0.0);
        auto s1 = m_Context.CreateValue("length", 0.0);

        auto circleArea0 = m_Context.CreateValue("area", 0.0);

        auto rectArea0 = m_Context.CreateValue("area", 0.0);
        m_Context.render = std::make_unique<Render>();


        ///数据流层
        auto pLNode0 = std::make_unique<PositionToLengthNode>(j0->m_Id, j1->m_Id, s0->m_Id);
        pLNode0->computeFunc = [](const std::shared_ptr<ValueHandle>& in0, const std::shared_ptr<ValueHandle>& in1,
                                  const std::shared_ptr<ValueHandle>& out)
        {
            auto v0 = in0->GetProperty<gp_Pnt>("position");
            auto v1 = in1->GetProperty<gp_Pnt>("position");
            out->SetProperty("length", v0.Distance(v1));
        };

        auto pLNode1 = std::make_unique<PositionToLengthNode>(j1->m_Id, j2->m_Id, s1->m_Id);
        pLNode1->computeFunc = [](const std::shared_ptr<ValueHandle>& in0, const std::shared_ptr<ValueHandle>& in1,
                                  const std::shared_ptr<ValueHandle>& out)
        {
            const auto v0 = in0->GetProperty<gp_Pnt>("position");
            const auto v1 = in1->GetProperty<gp_Pnt>("position");
            out->SetProperty("length", v0.Distance(v1));
        };

        auto rectAreaNode0 = std::make_unique<LengthToRectArea>(s0->m_Id, s1->m_Id, rectArea0->m_Id);
        rectAreaNode0->computeFunc = [](const std::shared_ptr<ValueHandle>& in0,
                                        const std::shared_ptr<ValueHandle>& in1,
                                        const std::shared_ptr<ValueHandle>& out)
        {
            const auto v0 = in0->GetProperty<double>("length");
            const auto v1 = in1->GetProperty<double>("length");
            out->SetProperty("area", v0 * v1);
        };

        auto circleAreaNode0 = std::make_unique<LengthToCircleArea>(s0->m_Id, circleArea0->m_Id);
        circleAreaNode0->computeFunc = [](const std::shared_ptr<ValueHandle>& in0,
                                          const std::shared_ptr<ValueHandle>& out)
        {
            constexpr double pi = 3.14159265358979323846;
            const auto v0 = in0->GetProperty<double>("length");
            out->SetProperty("area", pi * v0 * v0);
        };


        auto lengthToRenderNode0 = std::make_unique<LengthToRenderNode>(s0->m_Id);
        auto lengthToRenderNode1 = std::make_unique<LengthToRenderNode>(s1->m_Id);

        m_Context.AddComputerNode(std::move(pLNode0), {j0, j1}, {s0});
        m_Context.AddComputerNode(std::move(pLNode1), {j1, j2}, {s1});

        m_Context.AddComputerNode(std::move(rectAreaNode0), {s0, s1}, {rectArea0});
        m_Context.AddComputerNode(std::move(circleAreaNode0), {s0}, {circleArea0});

        m_Context.AddComputerNode(std::move(lengthToRenderNode0), {s0}, {});
        m_Context.AddComputerNode(std::move(lengthToRenderNode1), {s1}, {});

        ///变更
        m_Context.SetValueProperty(j0, "position", gp_Pnt{10, 0, 0});
        m_Context.Evaluator();
    }

public:
    DGContext m_Context;
};

