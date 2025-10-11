//
// Created by ZQD on 25-9-25.
//

#include "MyAisObject.h"
#include "MyAisOwner.h"
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <TopoDS_Shape.hxx>
#include <StdPrs_ShadedShape.hxx>
#include <StdPrs_WFShape.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_BndBox.hxx>
#include "BRepBndLib.hxx"
#include <Prs3d_ToolSphere.hxx>
#include <Prs3d_ToolCylinder.hxx>
#include <Prs3d_ToolDisk.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <Select3D_SensitivePrimitiveArray.hxx>
#include <StdPrs_ToolTriangulatedShape.hxx>
#include <StdSelect_BRepSelectionTool.hxx>

MyAisObject::MyAisObject()
{
    //设置自定义显示模式，设置非活动显示模式
    SetDisplayMode(Main);
    //SetHilightMode(HighLight);
    ///设置修改内部的基本外观
    myDrawer->SetupOwnShadingAspect();
    myDrawer->ShadingAspect()->SetMaterial(Graphic3d_NameOfMaterial_Gold);
    ///设置线条外观
    myDrawer->SetWireAspect(new Prs3d_LineAspect(Quantity_NameOfColor::Quantity_NOC_GREEN,Aspect_TypeOfLine::Aspect_TOL_DOT,2.0));
}

void MyAisObject::Compute(const opencascade::handle<PrsMgr_PresentationManager>& thePrsMgr,
                          const opencascade::handle<Prs3d_Presentation>& thePrs, const Standard_Integer theMode)
{
    auto radius=100.;
    auto height=100.;
    const int nbSlices = 25, nbStacks = 25;
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
        ///二次构件（Quadric builders）
        if (true)
        {
            gp_Trsf tSphere;
            tSphere.SetTranslationPart(gp_Vec(100, 100, 100));
            Handle(Graphic3d_ArrayOfTriangles) atris = Prs3d_ToolSphere::Create(radius, nbSlices, nbStacks, tSphere);

            ///构件圆柱
            gp_Trsf tCylinder;
            tCylinder.SetTranslationPart(gp_Vec(-200, -200, 0));
            //Handle(Graphic3d_ArrayOfTriangles) atris2=Prs3d_ToolCylinder::Create(radius, radius,height, 25, 25, tCylinder);
            Prs3d_ToolCylinder aCy1(radius, radius, height, nbSlices, nbStacks);
            ///使用Prs3d_ToolDisk封顶
            Prs3d_ToolDisk aDisk(0.0, radius, nbSlices, 1);
            Handle(Graphic3d_ArrayOfTriangles) aTrisDisk = new Graphic3d_ArrayOfTriangles(
                aCy1.VerticesNb() + aDisk.VerticesNb(), 3 * (aCy1.TrianglesNb() + aDisk.TrianglesNb()),
                Graphic3d_ArrayFlags_VertexNormal);
            ///数据填入aTrisDisk中
            aCy1.FillArray(aTrisDisk, tCylinder);
            aDisk.FillArray(aTrisDisk, tCylinder);

            ///手动添加,需要手动变换坐标位置
            Handle(Graphic3d_ArrayOfTriangles) aTris3 = new Graphic3d_ArrayOfTriangles(
                nbSlices + 1, nbSlices * 3,
                Graphic3d_ArrayFlags_VertexNormal);
            aTris3->AddVertex(gp_Pnt(0, 0, height).Transformed(tCylinder), gp::DZ());
            for (int aSlice = 0; aSlice < nbSlices; ++aSlice)
            {
                double angle = M_PI * 2.0 * static_cast<double>(aSlice) / static_cast<double>(nbSlices);
                aTris3->AddVertex(gp_Pnt(Cos(angle) * radius, Sin(angle) * radius, height).Transformed(tCylinder),
                                  gp::DZ());
            }
            for (int aSlice = 0; aSlice < nbSlices; ++aSlice)
            {
                aTris3->AddEdges(1, aSlice + 2, aSlice + 1 < nbSlices ? (aSlice + 3) : 2);
            }


            Handle(Graphic3d_Group) aGroupSegs = thePrs->NewGroup();
            aGroupSegs->SetGroupPrimitivesAspect(myDrawer->ShadingAspect()->Aspect());
            aGroupSegs->AddPrimitiveArray(atris);
            aGroupSegs->AddPrimitiveArray(aTrisDisk);
            aGroupSegs->AddPrimitiveArray(aTris3);
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
    if (theMode != 0)
        return;
    const double radius = 100., height = 100.;
    TopoDS_Shape aShape = BRepPrimAPI_MakeCylinder(radius, height);
    ///将包围盒添加到选择
    if (false)
    {
        Bnd_Box aBox;
        BRepBndLib::Add(aShape, aBox);
        Handle(SelectMgr_EntityOwner) anOwner = new SelectMgr_EntityOwner(this);
        Handle(Select3D_SensitiveBox) aSensBox = new Select3D_SensitiveBox(anOwner, aBox);
        theSelection->Add(aSensBox);
    }
    if (false)
    {
        Standard_Real aDef1 = StdPrs_ToolTriangulatedShape::GetDeflection(aShape, myDrawer);
        StdSelect_BRepSelectionTool::Load(theSelection, aShape, TopAbs_SHAPE, aDef1, myDrawer->DeviationAngle(),
                                          myDrawer->IsAutoTriangulation());
    }
    Handle(MyAisOwner) anOwner = new MyAisOwner(this);
    Handle(Graphic3d_ArrayOfTriangles) aTris = Prs3d_ToolCylinder::Create(radius, radius, height, 25, 25, gp_Trsf());
    Handle(Select3D_SensitivePrimitiveArray) aSensTris = new Select3D_SensitivePrimitiveArray(anOwner);
    gp_Trsf tCylinder;
    tCylinder.SetTranslationPart(gp_Vec(-200, -200, 0));
    aSensTris->InitTriangulation(aTris->Attributes(), aTris->Indices(), TopLoc_Location(tCylinder));
    theSelection->Add(aSensTris);
}

Standard_Boolean MyAisObject::AcceptDisplayMode(const Standard_Integer theMode) const
{
    return AIS_InteractiveObject::AcceptDisplayMode(theMode);
}
