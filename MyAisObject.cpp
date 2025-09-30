//
// Created by ZQD on 25-9-25.
//

#include "MyAisObject.h"

#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <TopoDS_Shape.hxx>
#include <StdPrs_ShadedShape.hxx>
#include <StdPrs_WFShape.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_BndBox.hxx>
#include "BRepBndLib.hxx"
#include <Prs3d_ToolSphere.hxx>

MyAisObject::MyAisObject()
{
    //设置自定义显示模式，设置非活动显示模式
    SetDisplayMode(Main);
    SetHilightMode(HighLight);
    ///设置修改内部的基本外观
    myDrawer->SetupOwnShadingAspect();
    myDrawer->ShadingAspect()->SetMaterial(Graphic3d_NameOfMaterial_Silver);
    ///设置线条外观
    myDrawer->SetWireAspect(new Prs3d_LineAspect(Quantity_NameOfColor::Quantity_NOC_GREEN,Aspect_TypeOfLine::Aspect_TOL_DOT,2.0));
}

void MyAisObject::Compute(const opencascade::handle<PrsMgr_PresentationManager>& thePrsMgr,
                          const opencascade::handle<Prs3d_Presentation>& thePrs, const Standard_Integer theMode)
{
    auto radius=100.;
    auto height=100.;
    TopoDS_Shape aShape=BRepPrimAPI_MakeCone(radius, 0.0, height);
    if (theMode==0)
    {
        ///呈现构建器
        StdPrs_ShadedShape::Add(thePrs,aShape,myDrawer);
        //StdPrs_WFShape::Add(thePrs,aShape,myDrawer);
        ///添加基本元素
        if(false)
        {
            Handle(Graphic3d_ArrayOfSegments) sSegs=new Graphic3d_ArrayOfSegments(4,4*2,Graphic3d_ArrayFlags_None);
            sSegs->AddVertex(gp_Pnt(0,0,0));
            sSegs->AddVertex(gp_Pnt(-radius,0,0));
            sSegs->AddVertex(gp_Pnt(0,0,radius));
            sSegs->AddVertex(gp_Pnt(radius,0,0));
            sSegs->AddEdges(1,2);
            sSegs->AddEdges(2,3);
            sSegs->AddEdges(3,4);
            sSegs->AddEdges(4,1);
            Handle(Graphic3d_Group) aGroupSegs=thePrs->NewGroup();
            aGroupSegs->SetGroupPrimitivesAspect(myDrawer->WireAspect()->Aspect());
            aGroupSegs->AddPrimitiveArray(sSegs);
        }
        ///二次构件
        if (true)
        {
            gp_Trsf t;
            t.SetTranslationPart(gp_Vec(100,100,100));
            Handle(Graphic3d_ArrayOfTriangles) atris=Prs3d_ToolSphere::Create(radius,25,25,t);
            Handle(Graphic3d_Group) aGroupSegs=thePrs->NewGroup();
            aGroupSegs->SetGroupPrimitivesAspect(myDrawer->ShadingAspect()->Aspect());
            aGroupSegs->AddPrimitiveArray(atris);
        }

    }
    else if (theMode==1)
    {
        Bnd_Box box;
        BRepBndLib::Add(aShape,box);
        Prs3d_BndBox::Add(thePrs,box,myDrawer);
        // TopoDS_Shape aShape = BRepPrimAPI_MakeCylinder(100.0, 20);
        // StdPrs_ShadedShape::Add (thePrs, aShape, myDrawer);
    }


}

void MyAisObject::ComputeSelection(const opencascade::handle<SelectMgr_Selection>& theSelection,
    const Standard_Integer theMode)
{
}

Standard_Boolean MyAisObject::AcceptDisplayMode(const Standard_Integer theMode) const
{
    return AIS_InteractiveObject::AcceptDisplayMode(theMode);
}
