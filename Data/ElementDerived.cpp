//
// Created by ZQD on 2026/8/8.
//

#include "ElementDerived.h"

#include <gp_pnt.hxx>

PointElement::PointElement() {
    m_Properties.Set("Position", gp_Pnt());
}

const MiniMetaObject::MetaObject *PointElement::GetStaticMetaObject() noexcept {
    using namespace MiniMetaObject;
    static const MetaObject StaticMetaObject{
        "PointElement", Element::GetStaticMetaObject(),
        {
            PropertySet::MakeMetaProperty<PointElement, gp_Pnt>("Position", "Position")
        }
    };
    return &StaticMetaObject;
}

const MiniMetaObject::MetaObject *PointElement::GetMetaObject() const noexcept {
    return GetStaticMetaObject();
}

SegmentElement::SegmentElement() {
    m_Properties.Set("Length", 0.0);
}

const MiniMetaObject::MetaObject *SegmentElement::GetStaticMetaObject() noexcept {
    using namespace MiniMetaObject;
    static const MetaObject StaticMetaObject{
        "SegmentElement", Element::GetStaticMetaObject(),
        {
            PropertySet::MakeMetaProperty<SegmentElement, double>("Length", "Length")
        }
    };
    return &StaticMetaObject;
}

const MiniMetaObject::MetaObject *SegmentElement::GetMetaObject() const noexcept {
    return GetStaticMetaObject();
}

CircleElement::CircleElement() {
    m_Properties.Set("Area", 0.0);
}

const MiniMetaObject::MetaObject *CircleElement::GetStaticMetaObject() noexcept {
    using namespace MiniMetaObject;
    static const MetaObject StaticMetaObject{
        "CircleElement", Element::GetStaticMetaObject(),
        {
            PropertySet::MakeMetaProperty<CircleElement, double>("Area", "Area")
        }
    };
    return &StaticMetaObject;
}

const MiniMetaObject::MetaObject *CircleElement::GetMetaObject() const noexcept {
    return GetStaticMetaObject();
}

MetricsElement::MetricsElement() {
    m_Properties.Set("TotalLength", 0.0);
}

const MiniMetaObject::MetaObject *MetricsElement::GetStaticMetaObject() noexcept {
    using namespace MiniMetaObject;
    static const MetaObject StaticMetaObject{
        "MetricsElement", Element::GetStaticMetaObject(),
        {
            PropertySet::MakeMetaProperty<MetricsElement, double>("TotalLength", "TotalLength")
        }
    };
    return &StaticMetaObject;
}

const MiniMetaObject::MetaObject *MetricsElement::GetMetaObject() const noexcept {
    return GetStaticMetaObject();
}
