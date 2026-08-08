#include "SketchModel.h"

#include "DependencyGraph/ComputerView.h"
#include "Data/Element.h"
#include <numbers>

SketchModel::SketchModel(DGContext *context, Document *document) : m_Context(context), m_Document(document) {
}

Element *SketchModel::CreateElement(std::string_view elementName) const {
    auto object = m_Document->GetMetaRegister()->CreateObject(std::string(elementName));
    auto element = std::unique_ptr<Element>(static_cast<Element *>(object.release()));
    const auto eleRawPtr = element.get();
    m_Document->RegisterElement(std::move(element));
    return eleRawPtr;
}

void SketchModel::AddPropertyAddress(PropertyAddress address) const {
    m_Context->AddValueAddress(std::move(address));
}

void SketchModel::AddSegmentComputerNode(std::vector<PropertyAddress> in, std::vector<PropertyAddress> out) const {
    m_Context->AddComputeNode(std::move(in), std::move(out), [](const ComputerView &view) {
        const auto p0 = view.Input<gp_Pnt>(0);
        const auto p1 = view.Input<gp_Pnt>(1);
        view.SetOutput(0, p0.Distance(p1));
    });
}

void SketchModel::AddCircleAreaComputerNode(std::vector<PropertyAddress> in, std::vector<PropertyAddress> out) const {
    m_Context->AddComputeNode(std::move(in), std::move(out), [](const ComputerView &view) {
        const auto radiusLength = view.Input<double>(0);
        view.SetOutput(0, std::numbers::pi * radiusLength * radiusLength);
    });
}

DGContext::EvaluationResult SketchModel::Evaluate() const {
    return m_Context->Evaluate();
}

void SketchModel::MarkDirty(PropertyAddress node) const {
    m_Context->MarkDirty(std::move(node));
}
