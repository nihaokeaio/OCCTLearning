//
// Created by ZQD on 25-10-9.
//
#pragma once
#include <SelectMgr_EntityOwner.hxx>


class MyAisObject;

class MyAisOwner : public SelectMgr_EntityOwner
{
    DEFINE_STANDARD_RTTI_INLINE(MyAisOwner, SelectMgr_EntityOwner)

public:
    explicit MyAisOwner(const Handle(MyAisObject)& theObj, int thePriority = 0);

    void HilightWithColor(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                          const Handle(Prs3d_Drawer)& theStyle,
                          const Standard_Integer theMode) override;

    void Unhilight(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                   const Standard_Integer theMode) override;

    Standard_Boolean IsForcedHilight() const override;

protected:
    Handle(Prs3d_Presentation) myPrs;
};




