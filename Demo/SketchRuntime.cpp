//
// Created by ZQD on 26-8-13.
//

#include "SketchRuntime.h"
#include "SketchModel.h"
#include "Data/Document.h"
#include "Data/Property/PropertyResolver.h"
#include "DependencyGraph/DependencyBinding.h"

SketchRuntime::SketchRuntime()
{
    m_Document = std::make_unique<Document>();
    m_PropertyResolver = std::make_unique<PropertyResolver>(m_Document.get());
    m_DGContext = std::make_unique<DGContext>(m_PropertyResolver.get());
    m_DependencyBinding = std::make_unique<DependencyBinding>(m_DGContext.get());
    m_SketchModel = std::make_unique<SketchModel>(m_DGContext.get(), m_Document.get());
    InitConnect();
}

SketchRuntime::~SketchRuntime() = default;

bool SketchRuntime::SetProperty(const PropertyAddress& address, const PropertyValue& value, ChangeSource source) const
{
    return m_PropertyResolver->Write(address, value, source);
}

DGContext::EvaluationResult SketchRuntime::Flush() const
{
    return m_DGContext->Evaluate();
}

Document* SketchRuntime::GetDocument() const
{
    return m_Document.get();
}

DGContext* SketchRuntime::GetDGContext() const
{
    return m_DGContext.get();
}

SketchModel* SketchRuntime::GetSketchModel() const
{
    return m_SketchModel.get();
}

PropertyResolver* SketchRuntime::GetPropertyResolver() const
{
    return m_PropertyResolver.get();
}

void SketchRuntime::InitConnect()
{
    MiniSignal::connect(m_Document.get(), &Document::m_ElementPropertyChangedSignal,
                        [this](const MessageInfo::PropertyChangePayload& message)
                        {
                            m_DependencyBinding->OnPropertyChanged({message.source, message.address});
                        });
}
