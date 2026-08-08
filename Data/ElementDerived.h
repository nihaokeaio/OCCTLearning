//
// Created by ZQD on 2026/8/8.
//

#pragma once
#include "Element.h"

#define META_OBJECT_DEFINE\
static const MiniMetaObject::MetaObject *GetStaticMetaObject() noexcept;\
[[nodiscard]] const MiniMetaObject::MetaObject *GetMetaObject() const noexcept override;


class PointElement : public Element {
public:
    PointElement();

    static const MiniMetaObject::MetaObject *GetStaticMetaObject() noexcept;

    [[nodiscard]] const MiniMetaObject::MetaObject *GetMetaObject() const noexcept override;
};

class SegmentElement : public Element {
public:
    SegmentElement();

    static const MiniMetaObject::MetaObject *GetStaticMetaObject() noexcept;

    [[nodiscard]] const MiniMetaObject::MetaObject *GetMetaObject() const noexcept override;
};

class CircleElement : public Element {
public:
    CircleElement();

    static const MiniMetaObject::MetaObject *GetStaticMetaObject() noexcept;

    [[nodiscard]] const MiniMetaObject::MetaObject *GetMetaObject() const noexcept override;
};


