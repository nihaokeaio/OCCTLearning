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

#ifdef DG_ENABLE_TRACE
    m_Context.SetDebugName(j0, "j0.position");
    m_Context.SetDebugName(j1, "j1.position");
    m_Context.SetDebugName(j2, "j2.position");
    m_Context.SetDebugName(s0, "s0.length");
    m_Context.SetDebugName(s1, "s1.length");
    m_Context.SetDebugName(circleArea0, "circle.area");
    m_Context.SetDebugName(rectArea0, "rect.area");
#endif

    const auto distanceJ0J1 = m_Context.AddComputeNode({j0, j1}, {s0}, [](const ComputerView& view)
    {
        const auto p0 = view.Input<gp_Pnt>(0, "position");
        const auto p1 = view.Input<gp_Pnt>(1, "position");
        view.SetOutput(0, "length", p0.Distance(p1));
    });

    const auto distanceJ1J2 = m_Context.AddComputeNode({j1, j2}, {s1}, [](const ComputerView& view)
    {
        const auto p0 = view.Input<gp_Pnt>(0, "position");
        const auto p1 = view.Input<gp_Pnt>(1, "position");
        view.SetOutput(0, "length", p0.Distance(p1));
    });

    const auto rectArea = m_Context.AddComputeNode({s0, s1}, {rectArea0}, [](const ComputerView& view)
    {
        const auto l0 = view.Input<double>(0, "length");
        const auto l1 = view.Input<double>(1, "length");
        view.SetOutput(0, "area", l0 * l1);
    });

    const auto circleArea = m_Context.AddComputeNode({s0}, {circleArea0}, [](const ComputerView& view)
    {
        constexpr double pi = M_PI;
        const auto l0 = view.Input<double>(0, "length");
        view.SetOutput(0, "area", pi * l0 * l0);
    });

    const auto renderS0 = m_Context.AddComputeNode({s0}, {}, [this](const ComputerView& view)
    {
        m_Context.render->Update(view.InId(0));
    });

    const auto renderS1 = m_Context.AddComputeNode({s1}, {}, [this](const ComputerView& view)
    {
        m_Context.render->Update(view.InId(0));
    });

#ifdef DG_ENABLE_TRACE
    m_Context.SetDebugName(distanceJ0J1, "distance j0-j1");
    m_Context.SetDebugName(distanceJ1J2, "distance j1-j2");
    m_Context.SetDebugName(rectArea, "rect area");
    m_Context.SetDebugName(circleArea, "circle area");
    m_Context.SetDebugName(renderS0, "render s0");
    m_Context.SetDebugName(renderS1, "render s1");
#endif

    m_Context.SetValueProperty(j0, "position", gp_Pnt{10, 0, 0});
    m_Context.Evaluator();
}
