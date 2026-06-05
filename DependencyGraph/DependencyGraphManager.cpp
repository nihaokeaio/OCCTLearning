#include "DependencyGraphManager.h"

#include "ComputeNodes.h"

#include <gp_Pnt.hxx>

#include <memory>

void DependencyGraphManager::Test()
{
    auto j0 = m_Context.CreateValue("position", gp_Pnt(0, 0, 0));
    auto j1 = m_Context.CreateValue("position", gp_Pnt(100, 0, 0));
    auto j2 = m_Context.CreateValue("position", gp_Pnt(100, 100, 0));

    auto s0 = m_Context.CreateValue("length", 0.0);
    auto s1 = m_Context.CreateValue("length", 0.0);

    auto circleArea0 = m_Context.CreateValue("area", 0.0);
    auto rectArea0 = m_Context.CreateValue("area", 0.0);
    m_Context.render = std::make_unique<Render>();

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

    m_Context.SetValueProperty(j0, "position", gp_Pnt{10, 0, 0});
    m_Context.Evaluator();
}
