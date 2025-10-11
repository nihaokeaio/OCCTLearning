//
// Created by ZQD on 25-9-25.
//
#pragma once
#include <AIS_InteractiveObject.hxx>

class MyAisObject : public AIS_InteractiveObject
{
    DEFINE_STANDARD_RTTI_INLINE(MyAisObject, AIS_InteractiveObject)
public:
    enum MyDisplayMode{Main=0,HighLight=1};
    //! Default constructor.
    MyAisObject();

    //! Destructor.
    ~MyAisObject() override =default;

    //! Returns the3D View.
public:
    void Compute(const opencascade::handle<PrsMgr_PresentationManager>& thePrsMgr,
                 const opencascade::handle<Prs3d_Presentation>& thePrs, const Standard_Integer theMode) override;

public:
    void ComputeSelection(const opencascade::handle<SelectMgr_Selection>& theSelection,
        const Standard_Integer theMode) override;

    Standard_Boolean AcceptDisplayMode(const Standard_Integer theMode) const override;
};



