//
// Created by ZQD on 26-8-13.
//

#pragma once
#include <memory>

#include "Data/Property/PropertyAddress.h"
#include "Data/Property/PropertySet.h"
#include "DependencyGraph/DGContext.h"


class SketchModel;
class DependencyBinding;
class PropertyResolver;
class Document;

class SketchRuntime
{
public:
    SketchRuntime();
    ~SketchRuntime();

    bool SetProperty(const PropertyAddress& address, const PropertyValue& value, ChangeSource source) const;

    DGContext::EvaluationResult Flush() const;

    Document* GetDocument() const;
    DGContext* GetDGContext() const;
    SketchModel* GetSketchModel() const;
    PropertyResolver* GetPropertyResolver() const;

private:
    void InitConnect();

private:
    std::unique_ptr<Document> m_Document;
    std::unique_ptr<DGContext> m_DGContext;
    std::unique_ptr<PropertyResolver> m_PropertyResolver;
    std::unique_ptr<DependencyBinding> m_DependencyBinding;
    std::unique_ptr<SketchModel> m_SketchModel;
};



