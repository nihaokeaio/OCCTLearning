//
// Created by ZQD on 25-10-9.
//

#include "MyAisOwner.h"
#include "MyAisObject.h"

#include <AIS_InteractiveContext.hxx>
#include <Graphic3d_ArrayOfTriangles.hxx>

#include <Prs3d_PresentationShadow.hxx>
#include <SelectMgr_SortCriterion.hxx>
#include <StdSelect_ViewerSelector3d.hxx>
#include <Prs3d_ArrowAspect.hxx>
#include <Prs3d_Arrow.hxx>

MyAisOwner::MyAisOwner(const opencascade::handle<MyAisObject>& theObj, int thePriority): SelectMgr_EntityOwner(
    theObj, thePriority)
{
}

void MyAisOwner::HilightWithColor(const opencascade::handle<PrsMgr_PresentationManager>& thePrsMgr,
                                  const opencascade::handle<Prs3d_Drawer>& theStyle, const Standard_Integer theMode)
{
    myPrs = new Prs3d_Presentation(thePrsMgr->StructureManager());
    MyAisObject* anObj = dynamic_cast<MyAisObject*>(mySelectable);
    if (myPrs.IsNull())
    {
        anObj->Compute(thePrsMgr, myPrs, MyAisObject::MyDisplayMode::HighLight);
    }
    if (thePrsMgr->IsImmediateModeOn())
    {
        if (false)
        {
            Handle(Prs3d_PresentationShadow) aShadow =
                new Prs3d_PresentationShadow(thePrsMgr->StructureManager(), myPrs);
            aShadow->SetZLayer(Graphic3d_ZLayerId_Top);
            aShadow->Highlight(theStyle);
            thePrsMgr->AddToImmediateList(aShadow);
        }

        Handle(StdSelect_ViewerSelector3d) aSelector =
            anObj->InteractiveContext()->MainSelector();
        SelectMgr_SortCriterion aPickPnt;
        for (int aPickIter = 1; aPickIter <= aSelector->NbPicked(); ++aPickIter)
        {
            if (aSelector->Picked(aPickIter) == this)
            {
                aPickPnt = aSelector->PickedData(aPickIter);
                break;
            }
        }
        Handle(Prs3d_Presentation) aPrs = mySelectable->GetHilightPresentation(thePrsMgr);
        aPrs->SetZLayer(Graphic3d_ZLayerId_Top);
        aPrs->Clear();
        Handle(Graphic3d_Group) aGroup = aPrs->NewGroup();
        aGroup->SetGroupPrimitivesAspect(theStyle->ArrowAspect()->Aspect());
        gp_Trsf aTrsfInv = mySelectable->LocalTransformation().Inverted();
        gp_Dir aNorm(aPickPnt.Normal.x(), aPickPnt.Normal.y(), aPickPnt.Normal.z());
        Handle(Graphic3d_ArrayOfTriangles) aTris =
            Prs3d_Arrow::DrawShaded(gp_Ax1(aPickPnt.Point, aNorm).Transformed(aTrsfInv),
                                    1.0, 15.0,
                                    3.0, 4.0, 10);
        aGroup->AddPrimitiveArray(aTris);
        thePrsMgr->AddToImmediateList(aPrs);
    }
    else
    {
        myPrs->Display();
    }
}

void MyAisOwner::Unhilight(const opencascade::handle<PrsMgr_PresentationManager>& thePrsMgr,
                           const Standard_Integer theMode)
{
    if (!myPrs.IsNull())
        myPrs->Erase();
}

Standard_Boolean MyAisOwner::IsForcedHilight() const
{
    return true;
}



