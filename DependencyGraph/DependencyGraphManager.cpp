#include "DependencyGraphManager.h"

#include <gp_Pnt.hxx>

#include <memory>

void DependencyGraphManager::Test()
{
    const auto j0 = m_Context.CreateValue("position", gp_Pnt(0, 0, 0));
    const auto j1 = m_Context.CreateValue("position", gp_Pnt(100, 0, 0));
    const auto j2 = m_Context.CreateValue("position", gp_Pnt(100, 100, 0));

    const auto s0 = m_Context.CreateValue("length", 0.0);
    const auto s1 = m_Context.CreateValue("length", 0.0);

    const auto circleArea0 = m_Context.CreateValue("area", 0.0);
    const auto rectArea0 = m_Context.CreateValue("area", 0.0);
    m_Context.render = std::make_unique<Render>();

    m_Context.AddComputeNode({j0, j1}, {s0}, [](const ComputerView& view)
    {
        auto v0 = view.In(0).GetProperty<gp_Pnt>("position");
        auto v1 = view.In(1).GetProperty<gp_Pnt>("position");
        view.Out(0).SetProperty("length", v0.Distance(v1));
    });

    m_Context.AddComputeNode({j1, j2}, {s1}, [](const ComputerView& view)
    {
        const auto v0 = view.In(0).GetProperty<gp_Pnt>("position");
        const auto v1 = view.In(1).GetProperty<gp_Pnt>("position");
        view.Out(0).SetProperty("length", v0.Distance(v1));
    });

    m_Context.AddComputeNode({s0, s1}, {rectArea0}, [](const ComputerView& view)
    {
        const auto v0 = view.In(0).GetProperty<double>("length");
        const auto v1 = view.In(1).GetProperty<double>("length");
        view.Out(0).SetProperty("area", v0 * v1);
    });

    m_Context.AddComputeNode({s0}, {circleArea0}, [](const ComputerView& view)
    {
        constexpr double pi = 3.14159265358979323846;
        const auto v0 = view.In(0).GetProperty<double>("length");
        view.Out(0).SetProperty("area", pi * v0 * v0);
    });

    m_Context.AddComputeNode({s0}, {}, [this](const ComputerView& view)
    {
        m_Context.render->Update(view.InId(0));
    });

    m_Context.AddComputeNode({s1}, {}, [this](const ComputerView& view)
    {
        m_Context.render->Update(view.InId(0));
    });

    m_Context.SetValueProperty(j0, "position", gp_Pnt{10, 0, 0});
    m_Context.Evaluator();
}
