
//
// Created by ZQD on 26-7-28.
//

#pragma once
#include <any>
#include <array>
#include <cassert>
#include <charconv>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <typeindex>
#include <deque>

#include "CommonTraits.h"

namespace Version1
{
  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass) noexcept
      : m_ClassName(className), m_SuperClass(superClass)
    {
    }

  public:
    [[nodiscard]] std::string_view ClassName() const noexcept { return m_ClassName; }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept { return m_SuperClass; }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
  };

  class Object
  {
  public:
    virtual ~Object() = default;

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return &StaticMetaObject;
    }

  public:
    inline static MetaObject StaticMetaObject{"Object", nullptr};
  };

  class Widget : public Object
  {
    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return &StaticMetaObject;
    }

  public:
    inline static const MetaObject StaticMetaObject{
      "Widget",
      &Object::StaticMetaObject
    };
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return &StaticMetaObject;
    }

    inline static const MetaObject StaticMetaObject{
      "Button",
      &Widget::StaticMetaObject
    };
  };
}; // namespace Version1

namespace Version2
{
  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass) noexcept
      : m_ClassName(className), m_SuperClass(superClass)
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept { return m_ClassName; }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept { return m_SuperClass; }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
  };

  class Object
  {
  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{"Object", nullptr};
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject()
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject()
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  template <typename T>
  concept DrivedFrom = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    template <DrivedFrom T>
    void Register()
    {
      auto className = T::GetStaticMetaObject()->ClassName();
      auto [iter, inserted] = m_ObjectsCreateFun.try_emplace(
        std::string(className), [] { return std::make_unique<T>(); });
      if (!inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object> Create(const std::string& className) const
    {
      if (!m_ObjectsCreateFun.contains(className))
        return nullptr;
      return m_ObjectsCreateFun.at(className)();
    }

  private:
    std::unordered_map<std::string, std::function<std::unique_ptr<Object>()>>
    m_ObjectsCreateFun;
  };
}; // namespace Version2

namespace Version3
{
  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass) noexcept
      : m_ClassName(className), m_SuperClass(superClass)
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept { return m_ClassName; }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept { return m_SuperClass; }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{"Object", nullptr};
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject()
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject()
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };
} // namespace Version3

#define META_ROOT_OBJECT(Type)                                                 \
  static const MetaObject *GetStaticMetaObject() noexcept {                    \
    static const MetaObject StaticMetaObject{#Type, nullptr};                  \
    return &StaticMetaObject;                                                  \
  }                                                                            \
                                                                               \
  [[nodiscard]] virtual const MetaObject *GetMetaObject() const noexcept {     \
    return GetStaticMetaObject();                                              \
  }

#define META_OBJECT(Type, Base)                                                \
  static const MetaObject *GetStaticMetaObject() noexcept {                    \
    static const MetaObject StaticMetaObject{#Type,                            \
                                             Base::GetStaticMetaObject()};     \
    return &StaticMetaObject;                                                  \
  }                                                                            \
                                                                               \
  [[nodiscard]] const MetaObject *GetMetaObject() const noexcept override {    \
    return GetStaticMetaObject();                                              \
  }

namespace Version4
{
  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass) noexcept
      : m_ClassName(className), m_SuperClass(superClass)
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;
    META_ROOT_OBJECT(Object)
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    META_OBJECT(Widget, Object)
  };

  class Button : public Widget
  {
  public:
    META_OBJECT(Button, Widget)
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };
} // namespace Version4

namespace Version5
{
  class Object;
  using PropertyValue = std::variant<int, bool, double, std::string>;

  class MetaProperty
  {
  public:
    using Getter = std::function<PropertyValue(const Object*)>;
    using Setter = std::function<bool(Object*, const PropertyValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] PropertyValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const PropertyValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<PropertyValue, Value>);

    auto getter = [member](const Object* object)-> PropertyValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const PropertyValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> properties = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(properties))

    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    std::string m_Text = "OK";

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {MakeMemberProperty<Button, std::string>("text", &Button::m_Text)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };
} // namespace Version5

namespace Version6
{
  class Object;
  using PropertyValue = std::variant<int, bool, double, std::string>;

  class MetaProperty
  {
  public:
    using Getter = std::function<PropertyValue(const Object*)>;
    using Setter = std::function<bool(Object*, const PropertyValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] PropertyValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const PropertyValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<PropertyValue, Value>);

    auto getter = [member](const Object* object)-> PropertyValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const PropertyValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<PropertyValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> PropertyValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const PropertyValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, accessorGetter, accessorSetter);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const noexcept
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeAccessorProperty<Button, std::string>("textMethod", &Button::Text, &Button::SetText)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    std::string m_Text = "OK";
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using PropertyTable = std::unordered_map<std::string, PropertyValue>;

  inline bool ApplyProperty(Object* object, const PropertyTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
} // namespace Version6


namespace Version7
{
  class Object;
  using PropertyValue = std::variant<int, bool, double, std::string>;

  class MetaProperty
  {
  public:
    using Getter = std::function<PropertyValue(const Object*)>;
    using Setter = std::function<bool(Object*, const PropertyValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] PropertyValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const PropertyValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Method = std::function<void(Object* object)>;

    MetaMethod(std::string name, Method method): m_Name(std::move(name)), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    void Invoke(Object* object) const
    {
      if (!m_Method)
        return;
      m_Method(object);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

  private:
    std::string m_Name;
    Method m_Method;
  };

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<PropertyValue, Value>);

    auto getter = [member](const Object* object)-> PropertyValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const PropertyValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<PropertyValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> PropertyValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const PropertyValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, accessorGetter, accessorSetter);
  }

  template <typename Owner>
  MetaMethod MakeMetaMethod(const std::string& name, void (Owner::*method)())
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object)
    {
      if (auto* owner = dynamic_cast<Owner*>(object))
        std::invoke(method, owner);
    };

    return MetaMethod(name, metaMethod);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name);
      return nullptr;
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }

    void Click()
    {
      ++m_Clicked;
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeAccessorProperty<Button, std::string>("textMethod", &Button::Text, &Button::SetText)
        },
        {
          MakeMetaMethod<Button>("Click", &Button::Click)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  public:
    int m_Clicked = 0;

  private:
    std::string m_Text = "OK";
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using PropertyTable = std::unordered_map<std::string, PropertyValue>;

  inline bool ApplyProperty(Object* object, const PropertyTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}


namespace Version8
{
  class Object;
  using PropertyValue = std::variant<int, bool, double, std::string>;

  class MetaProperty
  {
  public:
    using Getter = std::function<PropertyValue(const Object*)>;
    using Setter = std::function<bool(Object*, const PropertyValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] PropertyValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const PropertyValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const PropertyValue>;
    using Method = std::function<bool(Object*, Arguments)>;

    MetaMethod(std::string name, Method method): m_Name(std::move(name)), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    bool Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return false;
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

  private:
    std::string m_Name;
    Method m_Method;
  };

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<PropertyValue, Value>);

    auto getter = [member](const Object* object)-> PropertyValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const PropertyValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<PropertyValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> PropertyValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const PropertyValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, accessorGetter, accessorSetter);
  }

  template <typename Owner, typename T>
  MetaMethod MakeMetaMethod(const std::string& name, void (Owner::*method)(T arg))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> bool
    {
      if (arguments.empty())
        return false;
      const auto* typedValue = std::get_if<T>(&arguments[0]);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
        return false;
      std::invoke(method, owner, *typedValue);
      return true;
    };

    return MetaMethod(name, metaMethod);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name);
      return nullptr;
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }

    void Click()
    {
      ++m_Clicked;
    }

    void AddClick(int num)
    {
      m_Clicked += num;
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty<Button, int>("M_Click", &Button::m_Clicked)
        },
        {
          MakeMetaMethod<Button, int>("F_AddClick", &Button::AddClick)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  public:
    int m_Clicked = 0;

  private:
    std::string m_Text = "OK";
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using PropertyTable = std::unordered_map<std::string, PropertyValue>;

  inline bool ApplyProperty(Object* object, const PropertyTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}

namespace Version9
{
  class Object;
  using MetaValue = std::variant<int, bool, double, std::string>;

  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<bool(Object*, Arguments)>;

    MetaMethod(std::string name, Method method): m_Name(std::move(name)), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    bool Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return false;
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

  private:
    std::string m_Name;
    Method m_Method;
  };

  template <typename T>
  using StoredArgument = std::remove_cvref_t<T>;

  template <typename Owner, typename... Args, std::size_t... Index>
  bool InvokeMethod(Owner* owner, void (Owner::*method)(Args... args), std::span<const MetaValue> arguments,
                    std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, StoredArgument<Args>> && ...));
    // 参数类型匹配
    if (!((std::get_if<StoredArgument<Args>>(&arguments[Index]) != nullptr) && ...))
    {
      return false;
    }
    std::invoke(method, owner, std::get<StoredArgument<Args>>(arguments[Index])...);
    return true;
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, accessorGetter, accessorSetter);
  }

  template <typename Owner, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, void (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> bool
    {
      if (arguments.size() != sizeof...(Args))
        return false;
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return false;
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    return MetaMethod(name, metaMethod);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name);
      return nullptr;
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }

    void GetLegnth()
    {
      std::cout << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
    }

    void SetSumLength(double A, double B)
    {
      m_Length = A + B;
    }

    void GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length)
        },
        {
          MakeMetaMethod("F_SetLength", &Button::SetLength),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}

namespace Version10
{
  class Object;
  using MetaValue = std::variant<int, bool, double, std::string>;

  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method): m_Name(std::move(name)), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

  private:
    std::string m_Name;
    Method m_Method;
  };

  template <typename T>
  using StoredArgument = std::remove_cvref_t<T>;

  template <typename Owner, typename R, typename... Args, std::size_t... Index>
  InvokeResult InvokeMethod(Owner* owner, R (Owner::*method)(Args... args), std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, StoredArgument<Args>> && ...));
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((std::get_if<StoredArgument<Args>>(&arguments[Index]) != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, std::get<StoredArgument<Args>>(arguments[Index])...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, std::get<StoredArgument<Args>>(arguments[Index])...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    return MetaMethod(name, metaMethod);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name);
      return nullptr;
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }

    void GetLegnth()
    {
      std::cout << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length)
        },
        {
          MakeMetaMethod("F_SetLength", &Button::SetLength),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}

namespace Version11
{
  class Object;
  using MetaValue = std::variant<int, bool, double, std::string>;

  template <typename T>
  using StoredArgument = std::remove_cvref_t<T>;

  enum class MetaTypeId
  {
    Void,
    Bool,
    Int,
    Double,
    String,
    UnKnow
  };


  template <typename T>
  constexpr MetaTypeId GetMetaTypeId()
  {
    using Type = StoredArgument<T>;
    if constexpr (std::is_same_v<Type, bool>)
      return MetaTypeId::Bool;
    if constexpr (std::is_same_v<Type, int>)
      return MetaTypeId::Int;
    if constexpr (std::is_same_v<Type, double>)
      return MetaTypeId::Double;
    if constexpr (std::is_same_v<Type, std::string>)
      return MetaTypeId::String;
    //static_assert(std::is_void_v<Type>, "Unsupported meta type");
    return MetaTypeId::UnKnow;
  }

  inline MetaTypeId GetMetaTypeId(const MetaValue& metaValue)
  {
    return std::visit([]<typename T>(const T&)-> MetaTypeId
    {
      return GetMetaTypeId<T>();
    }, metaValue);
  }

  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method, std::vector<MetaTypeId> metaTypeIds, MetaTypeId returnType,
               bool isConst = false): m_Name(std::move(name)),
                                      m_Method(std::move(method)),
                                      m_Parameters(std::move(metaTypeIds)),
                                      m_ReturnType(returnType), m_IsConst(isConst)
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

    [[nodiscard]] bool Matches(std::span<const MetaValue> parameters) const
    {
      if (parameters.size() != m_Parameters.size())
        return false;
      for (int i = 0; i < parameters.size(); i++)
      {
        if (m_Parameters[i] != GetMetaTypeId(parameters[i]))
          return false;
      }
      return true;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
      return m_IsConst;
    }

  private:
    std::string m_Name;
    std::vector<MetaTypeId> m_Parameters;
    bool m_IsConst = false;
    MetaTypeId m_ReturnType;
    Method m_Method;
  };

  template <typename Method, size_t Index>
  using MethodStoredArgument = StoredArgument<std::tuple_element_t<Index, typename FunctionTraits<Method>::ArgsTuple>>;

  template <typename Owner, typename Method, std::size_t... Index>
  InvokeResult InvokeMethod(Owner owner, Method method, std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, MethodStoredArgument<Method, Index>> && ...));
    using R = typename FunctionTraits<Method>::ReturnType;
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((std::get_if<MethodStoredArgument<Method, Index>>(&arguments[Index]) != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, std::get<MethodStoredArgument<Method, Index>>(arguments[Index])...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, std::get<MethodStoredArgument<Method, Index>>(arguments[Index])...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = std::get_if<Value>(&value);
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<MetaTypeId> metaTypeIds = {GetMetaTypeId<Args>()...};
    MetaTypeId returnType;
    if constexpr (std::is_void_v<R>)
    {
      returnType = MetaTypeId::Void;
    }
    else
    {
      returnType = GetMetaTypeId<R>();
    }
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, false);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args) const)
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](const Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<const Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<MetaTypeId> metaTypeIds = {GetMetaTypeId<Args>()...};
    MetaTypeId returnType;
    if constexpr (std::is_void_v<R>)
    {
      returnType = MetaTypeId::Void;
    }
    else
    {
      returnType = GetMetaTypeId<R>();
    }
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, true);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name,
                                               std::span<const MetaValue> parameters, bool isConst = false) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        if (!Matches(method, parameters))
          continue;
        if (isConst != method.IsConst())
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name, parameters, isConst);
      return nullptr;
    }

    [[nodiscard]] bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters) const
    {
      return method.Matches(parameters);
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }


    void GetLength()
    {
      std::cout << "[Nan Const] GetLength = " << m_Length << std::endl;
    }

    void GetLength() const
    {
      std::cout << "[Const] GetLength = " << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
      std::cout << "[Double] Length = " << m_Length << std::endl;
    }


    void SetLength(int value)
    {
      m_Length += value;
      std::cout << "[INT] Length = " << m_Length << std::endl;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length)
        },
        {
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(int)>(&Button::SetLength)),
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(double)>(&Button::SetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)()>(&Button::GetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)() const>(&Button::GetLength)),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}


namespace Version12
{
  class Object;
  //using MetaValue = std::variant<int, bool, double, std::string>;

  template <typename T>
  using StoredArgument = std::decay_t<T>;


  class MetaType
  {
  public:
    explicit MetaType(std::type_index type, std::string name): m_TypeId(type), m_Name(std::move(name))
    {
    }

    [[nodiscard]] std::type_index TypeId() const noexcept
    {
      return m_TypeId;
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

  private:
    std::type_index m_TypeId;
    std::string m_Name;
  };

  template <typename T>
  const MetaType* GetMetaTypeImpl()
  {
    static const MetaType metaType{std::type_index(typeid(T)), typeid(T).name()};
    return &metaType;
  }

  template <typename T>
  const MetaType* GetMetaType()
  {
    return GetMetaTypeImpl<StoredArgument<T>>();
  }

  class MetaValue
  {
  public:
    template <typename T>
      requires (!std::same_as<StoredArgument<T>, MetaValue>)
    MetaValue(T&& value)
    {
      m_Value = std::forward<T>(value);
      m_Type = GetMetaType<T>();
    }

    MetaValue(const MetaValue& other) = default;
    MetaValue(MetaValue&& other) noexcept = default;
    MetaValue& operator=(const MetaValue& other) = default;
    MetaValue& operator=(MetaValue&& other) noexcept = default;

    [[nodiscard]] const MetaType* Type() const
    {
      return m_Type;
    }

    template <typename T>
    [[nodiscard]] bool Is() const noexcept
    {
      return m_Type == GetMetaType<T>();
    }

    template <typename T>
    [[nodiscard]] const StoredArgument<T>* GetIf() const noexcept
    {
      return std::any_cast<StoredArgument<T>>(&m_Value);
    }

  private:
    std::any m_Value;
    const MetaType* m_Type;
  };


  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, Getter getter, Setter setter): m_Name(std::move(name)), m_Getter(std::move(getter)),
                                                                  m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method, std::vector<const MetaType*> metaTypes, const MetaType* returnType,
               bool isConst = false): m_Name(std::move(name)),
                                      m_Parameters(std::move(metaTypes)),
                                      m_IsConst(isConst),
                                      m_ReturnType(returnType), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

    [[nodiscard]] bool Matches(std::span<const MetaValue> parameters) const
    {
      if (parameters.size() != m_Parameters.size())
        return false;
      for (int i = 0; i < parameters.size(); i++)
      {
        if (m_Parameters[i] != parameters[i].Type())
          return false;
      }
      return true;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
      return m_IsConst;
    }

  private:
    std::string m_Name;
    std::vector<const MetaType*> m_Parameters;
    bool m_IsConst = false;
    const MetaType* m_ReturnType;
    Method m_Method;
  };

  template <typename Method, size_t Index>
  using MethodStoredArgument = StoredArgument<std::tuple_element_t<Index, typename FunctionTraits<Method>::ArgsTuple>>;

  template <typename Owner, typename Method, std::size_t... Index>
  InvokeResult InvokeMethod(Owner owner, Method method, std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, MethodStoredArgument<Method, Index>> && ...));
    using R = typename FunctionTraits<Method>::ReturnType;
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>() != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, false);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args) const)
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](const Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<const Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, true);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name,
                                               std::span<const MetaValue> parameters, bool isConst = false) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        if (!Matches(method, parameters))
          continue;
        if (isConst != method.IsConst())
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name, parameters, isConst);
      return nullptr;
    }

    [[nodiscard]] bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters) const
    {
      return method.Matches(parameters);
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  struct Point
  {
    int x;
    int y;
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }


    void GetLength()
    {
      std::cout << "[Nan Const] GetLength = " << m_Length << std::endl;
    }

    void GetLength() const
    {
      std::cout << "[Const] GetLength = " << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
      std::cout << "[Double] Length = " << m_Length << std::endl;
    }


    void SetLength(int value)
    {
      m_Length += value;
      std::cout << "[INT] Length = " << m_Length << std::endl;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    void SetPosition(const Point& point)
    {
      m_Position = point;
    }

    Point GetPosition() const
    {
      return m_Position;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length),
          MakeMemberProperty("M_Position", &Button::m_Position),
        },
        {
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(int)>(&Button::SetLength)),
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(double)>(&Button::SetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)()>(&Button::GetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)() const>(&Button::GetLength)),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo),
          MakeMetaMethod("F_GetPosition", &Button::GetPosition),
          MakeMetaMethod("F_SetPosition", &Button::SetPosition)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
    Point m_Position;
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}


namespace Version13
{
  class Object;
  //using MetaValue = std::variant<int, bool, double, std::string>;

  template <typename T>
  using StoredArgument = std::decay_t<T>;


  class MetaType
  {
  public:
    explicit MetaType(std::type_index type, std::string name): m_TypeId(type), m_Name(std::move(name))
    {
    }

    [[nodiscard]] std::type_index TypeId() const noexcept
    {
      return m_TypeId;
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

  private:
    std::type_index m_TypeId;
    std::string m_Name;
  };

  template <typename T>
  const MetaType* GetMetaTypeImpl()
  {
    static const MetaType metaType{std::type_index(typeid(T)), typeid(T).name()};
    return &metaType;
  }

  template <typename T>
  const MetaType* GetMetaType()
  {
    return GetMetaTypeImpl<StoredArgument<T>>();
  }

  class MetaValue
  {
  public:
    template <typename T>
      requires (!std::same_as<StoredArgument<T>, MetaValue>)
    MetaValue(T&& value)
    {
      m_Value = std::forward<T>(value);
      m_Type = GetMetaType<T>();
    }

    MetaValue(const MetaValue& other) = default;
    MetaValue(MetaValue&& other) noexcept = default;
    MetaValue& operator=(const MetaValue& other) = default;
    MetaValue& operator=(MetaValue&& other) noexcept = default;

    [[nodiscard]] const MetaType* Type() const
    {
      return m_Type;
    }

    template <typename T>
    [[nodiscard]] bool Is() const noexcept
    {
      return m_Type == GetMetaType<T>();
    }

    template <typename T>
    [[nodiscard]] const StoredArgument<T>* GetIf() const noexcept
    {
      return std::any_cast<StoredArgument<T>>(&m_Value);
    }

  private:
    std::any m_Value;
    const MetaType* m_Type;
  };


  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, const MetaType* type, Getter getter, Setter setter): m_Name(std::move(name)),
      m_Type(type), m_Getter(std::move(getter)),
      m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] const MetaType* Type() const noexcept
    {
      return m_Type;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    const MetaType* m_Type;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method, std::vector<const MetaType*> metaTypes, const MetaType* returnType,
               bool isConst = false): m_Name(std::move(name)),
                                      m_Parameters(std::move(metaTypes)),
                                      m_IsConst(isConst),
                                      m_ReturnType(returnType), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

    [[nodiscard]] bool Matches(std::span<const MetaValue> parameters) const
    {
      if (parameters.size() != m_Parameters.size())
        return false;
      for (int i = 0; i < parameters.size(); i++)
      {
        if (m_Parameters[i] != parameters[i].Type())
          return false;
      }
      return true;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
      return m_IsConst;
    }

    [[nodiscard]] const MetaType* ReturnType() const noexcept
    {
      return m_ReturnType;
    }

    [[nodiscard]] std::span<const MetaType* const> ParameterTypes() const noexcept
    {
      return m_Parameters;
    }

    [[nodiscard]] std::size_t GetParameterCount() const noexcept
    {
      return m_Parameters.size();
    }

    [[nodiscard]] const MetaType* ParameterType(const std::size_t index) const noexcept
    {
      return m_Parameters[index];
    }

    [[nodiscard]] std::string Signature() const
    {
      auto retName = std::string(m_ReturnType->Name());
      std::string params;
      for (std::size_t i = 0; i < m_Parameters.size(); i++)
      {
        auto s = i == GetParameterCount() - 1 ? "" : ", ";
        params += std::string(m_Parameters[i]->Name()) + s;
      }
      std::string isConst = m_IsConst ? "const" : "";
      std::string signature = retName + " " + m_Name + "(" + params + ") " + isConst;
      return signature;
    }

  private:
    std::string m_Name;
    std::vector<const MetaType*> m_Parameters;
    bool m_IsConst = false;
    const MetaType* m_ReturnType;
    Method m_Method;
  };

  template <typename Method, size_t Index>
  using MethodStoredArgument = StoredArgument<std::tuple_element_t<Index, typename FunctionTraits<Method>::ArgsTuple>>;

  template <typename Owner, typename Method, std::size_t... Index>
  InvokeResult InvokeMethod(Owner owner, Method method, std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, MethodStoredArgument<Method, Index>> && ...));
    using R = typename FunctionTraits<Method>::ReturnType;
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>() != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, GetMetaType<Value>(), getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, GetMetaType<Value>(), accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, false);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args) const)
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](const Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<const Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, true);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name,
                                               std::span<const MetaValue> parameters, bool isConst = false) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        if (!Matches(method, parameters))
          continue;
        if (isConst != method.IsConst())
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name, parameters, isConst);
      return nullptr;
    }

    [[nodiscard]] bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters) const
    {
      return method.Matches(parameters);
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

    [[nodiscard]] std::span<const MetaMethod> OwnMethods() const noexcept
    {
      return m_Methods;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  inline std::string DumpMetaObject(const MetaObject* metaObject)
  {
    std::stringstream ss;
    const auto cls = metaObject->SuperClass()
                       ? std::string(" : ") + std::string(metaObject->SuperClass()->ClassName())
                       : "";
    ss << metaObject->ClassName() << cls << "\n";
    ss << "properties:\n";
    for (const auto& prop : metaObject->OwnProperties())
    {
      ss << "\t" << prop.Type()->TypeId().name() << prop.Name() << "\n";
    }
    ss << "methods:\n";
    for (const auto& method : metaObject->OwnMethods())
    {
      ss << "\t" << method.Signature() << "\n";
    }
    return ss.str();
  }

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  struct Point
  {
    int x;
    int y;
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }


    void GetLength()
    {
      std::cout << "[Nan Const] GetLength = " << m_Length << std::endl;
    }

    void GetLength() const
    {
      std::cout << "[Const] GetLength = " << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
      std::cout << "[Double] Length = " << m_Length << std::endl;
    }


    void SetLength(int value)
    {
      m_Length += value;
      std::cout << "[INT] Length = " << m_Length << std::endl;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    void SetPosition(const Point& point)
    {
      m_Position = point;
    }

    Point GetPosition() const
    {
      return m_Position;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length),
          MakeMemberProperty("M_Position", &Button::m_Position),
        },
        {
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(int)>(&Button::SetLength)),
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(double)>(&Button::SetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)()>(&Button::GetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)() const>(&Button::GetLength)),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo),
          MakeMetaMethod("F_GetPosition", &Button::GetPosition),
          MakeMetaMethod("F_SetPosition", &Button::SetPosition)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
    Point m_Position;
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class ObjectRegister
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };

  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}

namespace Version14
{
  class Object;
  //using MetaValue = std::variant<int, bool, double, std::string>;

  template <typename T>
  using StoredArgument = std::decay_t<T>;

  class MetaTypeRegister;

  class MetaType
  {
  public:
    explicit MetaType(std::type_index type, std::string name)
      :
      m_TypeId(type), m_Name(std::move(name))
    {
    }

    ~MetaType() = default;

    [[nodiscard]] std::type_index TypeId() const noexcept
    {
      return m_TypeId;
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    void SetName(const std::string_view name)
    {
      m_Name = name;
    }

  private:
    std::type_index m_TypeId;
    std::string m_Name;
  };

  class MetaTypeRegister
  {
  public:
    using MetaTypeMap = std::unordered_map<std::type_index, std::unique_ptr<MetaType>>;
    using MetaNameMap = std::unordered_map<std::string, const MetaType*>;


    template <typename T>
    static bool Register(const std::string_view metaName)
    {
      MetaType* metaType = GetOrCreate<T>();
      if (!metaType)
        return false;
      // 查询是否注册过
      if (const MetaType* namedMetaType = FindMetaType(metaName))
      {
        if (metaType == namedMetaType)
          return true;
        return false;
      }
      //若新的名字没有注册过，再看看旧名字是否被注册过了,若被注册了，则不能重复注册，返回false
      if (FindMetaType(metaType->Name()))
      {
        return false;
      }
      metaType->SetName(metaName);
      auto& metaNameType = GetMetaNameMap();
      auto [iter,insert] = metaNameType.emplace(std::string(metaName), metaType);
      return insert;
    }


    template <typename T>
    static const MetaType* GetMetaType()
    {
      return GetOrCreate<T>();
    }

    static const MetaType* FindMetaType(const std::string_view name)
    {
      const auto it = GetMetaNameMap().find(std::string(name));
      if (it == GetMetaNameMap().end())
        return nullptr;
      return it->second;
    }

  private:
    static MetaTypeMap& GetMetaTypeMap()
    {
      static MetaTypeMap metaTypeMap;
      return metaTypeMap;
    }

    static MetaNameMap& GetMetaNameMap()
    {
      static MetaNameMap metaNameMap;
      return metaNameMap;
    }

    static MetaType* FindMetaType(const std::type_index& typeId)
    {
      auto& metaTypeMap = GetMetaTypeMap();
      if (const auto iter = metaTypeMap.find(typeId); iter != metaTypeMap.end())
      {
        return iter->second.get();
      }

      return nullptr;
    }

    template <typename T>
    static MetaType* GetOrCreate()
    {
      using Type = StoredArgument<T>;
      std::type_index typeId = typeid(StoredArgument<T>);
      if (auto metaType = FindMetaType(typeId))
        return metaType;
      auto& metaTypeMap = GetMetaTypeMap();
      auto metaType = std::make_unique<MetaType>(typeId, typeid(Type).name());
      auto [iter, inserted] = metaTypeMap.emplace(typeId, std::move(metaType));
      return iter->second.get();
    }
  };

  template <typename T>
  const MetaType* GetMetaType()
  {
    return MetaTypeRegister::GetMetaType<T>();
  }

  template <typename T>
  bool RegisterMetaType(const std::string_view name)
  {
    using Type = StoredArgument<T>;
    return MetaTypeRegister::Register<Type>(name);
  }


  class MetaValue
  {
  public:
    template <typename T>
      requires (!std::same_as<StoredArgument<T>, MetaValue>)
    MetaValue(T&& value)
    {
      m_Value = std::forward<T>(value);
      m_Type = GetMetaType<T>();
    }

    MetaValue(const MetaValue& other) = default;
    MetaValue(MetaValue&& other) noexcept = default;
    MetaValue& operator=(const MetaValue& other) = default;
    MetaValue& operator=(MetaValue&& other) noexcept = default;

    [[nodiscard]] const MetaType* Type() const
    {
      return m_Type;
    }

    template <typename T>
    [[nodiscard]] bool Is() const noexcept
    {
      return m_Type == GetMetaType<T>();
    }

    template <typename T>
    [[nodiscard]] const StoredArgument<T>* GetIf() const noexcept
    {
      return std::any_cast<StoredArgument<T>>(&m_Value);
    }

  private:
    std::any m_Value;
    const MetaType* m_Type;
  };


  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, const MetaType* type, Getter getter, Setter setter): m_Name(std::move(name)),
      m_Type(type), m_Getter(std::move(getter)),
      m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] const MetaType* Type() const noexcept
    {
      return m_Type;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    const MetaType* m_Type;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method, std::vector<const MetaType*> metaTypes, const MetaType* returnType,
               bool isConst = false): m_Name(std::move(name)),
                                      m_Parameters(std::move(metaTypes)),
                                      m_IsConst(isConst),
                                      m_ReturnType(returnType), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

    [[nodiscard]] bool Matches(std::span<const MetaValue> parameters) const
    {
      if (parameters.size() != m_Parameters.size())
        return false;
      for (int i = 0; i < parameters.size(); i++)
      {
        if (m_Parameters[i] != parameters[i].Type())
          return false;
      }
      return true;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
      return m_IsConst;
    }

    [[nodiscard]] const MetaType* ReturnType() const noexcept
    {
      return m_ReturnType;
    }

    [[nodiscard]] std::span<const MetaType* const> ParameterTypes() const noexcept
    {
      return m_Parameters;
    }

    [[nodiscard]] std::size_t GetParameterCount() const noexcept
    {
      return m_Parameters.size();
    }

    [[nodiscard]] const MetaType* ParameterType(const std::size_t index) const noexcept
    {
      return m_Parameters[index];
    }

    [[nodiscard]] std::string Signature() const
    {
      std::ostringstream out;
      out << m_ReturnType->Name() << ' ' << m_Name << '(';
      for (std::size_t i = 0; i < m_Parameters.size(); ++i)
      {
        if (i != 0)
          out << ", ";
        out << m_Parameters[i]->Name();
      }
      out << ')';
      if (m_IsConst)
        out << " const";
      return out.str();
    }

  private:
    std::string m_Name;
    std::vector<const MetaType*> m_Parameters;
    bool m_IsConst = false;
    const MetaType* m_ReturnType;
    Method m_Method;
  };

  template <typename Method, size_t Index>
  using MethodStoredArgument = StoredArgument<std::tuple_element_t<Index, typename FunctionTraits<Method>::ArgsTuple>>;

  template <typename Owner, typename Method, std::size_t... Index>
  InvokeResult InvokeMethod(Owner owner, Method method, std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, MethodStoredArgument<Method, Index>> && ...));
    using R = typename FunctionTraits<Method>::ReturnType;
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>() != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, GetMetaType<Value>(), getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, GetMetaType<Value>(), accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, false);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args) const)
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](const Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<const Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, true);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name,
                                               std::span<const MetaValue> parameters, bool isConst = false) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        if (!Matches(method, parameters))
          continue;
        if (isConst != method.IsConst())
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name, parameters, isConst);
      return nullptr;
    }

    [[nodiscard]] bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters) const
    {
      return method.Matches(parameters);
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

    [[nodiscard]] std::span<const MetaMethod> OwnMethods() const noexcept
    {
      return m_Methods;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  inline std::string DumpMetaObject(const MetaObject* metaObject)
  {
    std::stringstream ss;
    const auto cls = metaObject->SuperClass()
                       ? std::string(" : ") + std::string(metaObject->SuperClass()->ClassName())
                       : "";
    ss << metaObject->ClassName() << cls << "\n";
    ss << "properties:\n";
    for (const auto& prop : metaObject->OwnProperties())
    {
      ss << "\t" << prop.Type()->Name() << " " << prop.Name() << "\n";
    }
    ss << "methods:\n";
    for (const auto& method : metaObject->OwnMethods())
    {
      ss << "\t" << method.Signature() << "\n";
    }
    return ss.str();
  }

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  struct Point
  {
    int x;
    int y;
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }


    void GetLength()
    {
      std::cout << "[Nan Const] GetLength = " << m_Length << std::endl;
    }

    void GetLength() const
    {
      std::cout << "[Const] GetLength = " << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
      std::cout << "[Double] Length = " << m_Length << std::endl;
    }


    void SetLength(int value)
    {
      m_Length += value;
      std::cout << "[INT] Length = " << m_Length << std::endl;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    void SetPosition(const Point& point)
    {
      m_Position = point;
    }

    Point GetPosition() const
    {
      return m_Position;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length),
          MakeMemberProperty("M_Position", &Button::m_Position),
        },
        {
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(int)>(&Button::SetLength)),
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(double)>(&Button::SetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)()>(&Button::GetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)() const>(&Button::GetLength)),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo),
          MakeMetaMethod("F_GetPosition", &Button::GetPosition),
          MakeMetaMethod("F_SetPosition", &Button::SetPosition)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
    Point m_Position;
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class RegisterObject
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };


  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }
}


namespace Version15
{
  class Object;
  //using MetaValue = std::variant<int, bool, double, std::string>;

  template <typename T>
  using StoredArgument = std::decay_t<T>;

  class MetaTypeRegister;

  class MetaType
  {
  public:
    using Serializer = std::function<std::optional<std::string>(const std::any&)>;
    using Deserializer = std::function<std::optional<std::any>(std::string_view)>;


    explicit

    MetaType(std::type_index type, std::string name, Serializer serializer = nullptr,
             Deserializer deserializer = nullptr)
      : m_TypeId(type), m_Name(std::move(name)), m_Serializer(std::move(serializer)),
        m_Deserializer(std::move(deserializer))
    {
    }

    ~MetaType() = default;

    [[nodiscard]] std::type_index TypeId() const noexcept
    {
      return m_TypeId;
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    void SetName(const std::string_view name)
    {
      m_Name = name;
    }

    void SetSerializeAndDeserialize(Serializer serializer, Deserializer deserializer)
    {
      m_Serializer = std::move(serializer);
      m_Deserializer = std::move(deserializer);
    }

    std::optional<std::string> Serialize(const std::any& value) const
    {
      if (!m_Serializer)
        return std::nullopt;
      return m_Serializer(value);
    }

    std::optional<std::any> Deserialize(const std::string_view stringView) const
    {
      if (!m_Deserializer)
        return std::nullopt;
      return m_Deserializer(stringView);
    }

    bool IsSerializable() const noexcept
    {
      return m_Serializer && m_Deserializer;
    }

  private:
    std::type_index m_TypeId;
    std::string m_Name;
    Serializer m_Serializer;
    Deserializer m_Deserializer;
  };

  class MetaTypeRegister
  {
  public:
    using MetaTypeMap = std::unordered_map<std::type_index, std::unique_ptr<MetaType>>;
    using MetaNameMap = std::unordered_map<std::string, const MetaType*>;


    template <typename T>
    static bool Register(const std::string_view metaName, const MetaType::Serializer serializer,
                         const MetaType::Deserializer deserializer)
    {
      MetaType* metaType = GetOrCreate<T>();
      if (!metaType)
        return false;
      // 查询是否注册过
      if (const MetaType* namedMetaType = FindMetaType(metaName))
      {
        if (metaType == namedMetaType)
          return true;
        return false;
      }
      //若新的名字没有注册过，再看看旧名字是否被注册过了,若被注册了，则不能重复注册，返回false
      if (FindMetaType(metaType->Name()))
      {
        return false;
      }
      metaType->SetName(metaName);
      metaType->SetSerializeAndDeserialize(serializer, deserializer);
      auto& metaNameType = GetMetaNameMap();
      metaNameType.emplace(std::string(metaName), metaType);
      return true;
    }


    template <typename T>
    static const MetaType* GetMetaType()
    {
      return GetOrCreate<T>();
    }

    static const MetaType* FindMetaType(const std::string_view name)
    {
      const auto it = GetMetaNameMap().find(std::string(name));
      if (it == GetMetaNameMap().end())
        return nullptr;
      return it->second;
    }

  private:
    static MetaTypeMap& GetMetaTypeMap()
    {
      static MetaTypeMap metaTypeMap;
      return metaTypeMap;
    }

    static MetaNameMap& GetMetaNameMap()
    {
      static MetaNameMap metaNameMap;
      return metaNameMap;
    }

    static MetaType* FindMetaType(const std::type_index& typeId)
    {
      auto& metaTypeMap = GetMetaTypeMap();
      if (const auto iter = metaTypeMap.find(typeId); iter != metaTypeMap.end())
      {
        return iter->second.get();
      }

      return nullptr;
    }

    template <typename T>
    static MetaType* GetOrCreate()
    {
      using Type = StoredArgument<T>;
      std::type_index typeId = typeid(StoredArgument<T>);
      if (auto metaType = FindMetaType(typeId))
        return metaType;
      auto& metaTypeMap = GetMetaTypeMap();
      auto metaType = std::make_unique<MetaType>(typeId, typeid(Type).name());
      auto [iter, inserted] = metaTypeMap.emplace(typeId, std::move(metaType));
      return iter->second.get();
    }
  };

  template <typename T>
  const MetaType* GetMetaType()
  {
    return MetaTypeRegister::GetMetaType<T>();
  }

  template <typename T>
  bool RegisterMetaType(const std::string_view name, MetaType::Serializer serializer = nullptr,
                        MetaType::Deserializer deserializer = nullptr)
  {
    using Type = StoredArgument<T>;
    return MetaTypeRegister::Register<Type>(name, serializer, deserializer);
  }


  class MetaValue
  {
  public:
    template <typename T>
      requires (!std::same_as<StoredArgument<T>, MetaValue>)
    MetaValue(T&& value)
    {
      m_Value = std::forward<T>(value);
      m_Type = GetMetaType<T>();
    }


    MetaValue(const MetaValue& other) = default;
    MetaValue(MetaValue&& other) noexcept = default;
    MetaValue& operator=(const MetaValue& other) = default;
    MetaValue& operator=(MetaValue&& other) noexcept = default;

    [[nodiscard]] const MetaType* Type() const
    {
      return m_Type;
    }

    template <typename T>
    [[nodiscard]] bool Is() const noexcept
    {
      return m_Type == GetMetaType<T>();
    }

    template <typename T>
    [[nodiscard]] const StoredArgument<T>* GetIf() const noexcept
    {
      return std::any_cast<StoredArgument<T>>(&m_Value);
    }

    std::optional<std::string> Serialize() const
    {
      return m_Type->Serialize(m_Value);
    }

    std::optional<MetaValue> static Deserialize(const MetaType* type, const std::string_view string)
    {
      if (!type)
        return std::nullopt;
      auto value = type->Deserialize(string);
      if (!value)
        return std::nullopt;
      if (std::type_index(value->type()) != type->TypeId())
        return std::nullopt;

      return MetaValue{std::move(*value), type};
    }

  private:
    MetaValue(std::any value, const MetaType* type): m_Value(std::move(value)), m_Type(type)
    {
    }

  private:
    std::any m_Value;
    const MetaType* m_Type;
  };


  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, const MetaType* type, Getter getter, Setter setter): m_Name(std::move(name)),
      m_Type(type), m_Getter(std::move(getter)),
      m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] const MetaType* Type() const noexcept
    {
      return m_Type;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    const MetaType* m_Type;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method, std::vector<const MetaType*> metaTypes, const MetaType* returnType,
               bool isConst = false): m_Name(std::move(name)),
                                      m_Parameters(std::move(metaTypes)),
                                      m_IsConst(isConst),
                                      m_ReturnType(returnType), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

    [[nodiscard]] bool Matches(std::span<const MetaValue> parameters) const
    {
      if (parameters.size() != m_Parameters.size())
        return false;
      for (int i = 0; i < parameters.size(); i++)
      {
        if (m_Parameters[i] != parameters[i].Type())
          return false;
      }
      return true;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
      return m_IsConst;
    }

    [[nodiscard]] const MetaType* ReturnType() const noexcept
    {
      return m_ReturnType;
    }

    [[nodiscard]] std::span<const MetaType* const> ParameterTypes() const noexcept
    {
      return m_Parameters;
    }

    [[nodiscard]] std::size_t GetParameterCount() const noexcept
    {
      return m_Parameters.size();
    }

    [[nodiscard]] const MetaType* ParameterType(const std::size_t index) const noexcept
    {
      return m_Parameters[index];
    }

    [[nodiscard]] std::string Signature() const
    {
      std::ostringstream out;
      out << m_ReturnType->Name() << ' ' << m_Name << '(';
      for (std::size_t i = 0; i < m_Parameters.size(); ++i)
      {
        if (i != 0)
          out << ", ";
        out << m_Parameters[i]->Name();
      }
      out << ')';
      if (m_IsConst)
        out << " const";
      return out.str();
    }

  private:
    std::string m_Name;
    std::vector<const MetaType*> m_Parameters;
    bool m_IsConst = false;
    const MetaType* m_ReturnType;
    Method m_Method;
  };

  template <typename Method, size_t Index>
  using MethodStoredArgument = StoredArgument<std::tuple_element_t<Index, typename FunctionTraits<Method>::ArgsTuple>>;

  template <typename Owner, typename Method, std::size_t... Index>
  InvokeResult InvokeMethod(Owner owner, Method method, std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, MethodStoredArgument<Method, Index>> && ...));
    using R = typename FunctionTraits<Method>::ReturnType;
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>() != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, GetMetaType<Value>(), getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, GetMetaType<Value>(), accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, false);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args) const)
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](const Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<const Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, true);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name,
                                               std::span<const MetaValue> parameters, bool isConst = false) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        if (!Matches(method, parameters))
          continue;
        if (isConst != method.IsConst())
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name, parameters, isConst);
      return nullptr;
    }

    [[nodiscard]] bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters) const
    {
      return method.Matches(parameters);
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

    [[nodiscard]] std::span<const MetaMethod> OwnMethods() const noexcept
    {
      return m_Methods;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  inline std::string DumpMetaObject(const MetaObject* metaObject)
  {
    std::stringstream ss;
    const auto cls = metaObject->SuperClass()
                       ? std::string(" : ") + std::string(metaObject->SuperClass()->ClassName())
                       : "";
    ss << metaObject->ClassName() << cls << "\n";
    ss << "properties:\n";
    for (const auto& prop : metaObject->OwnProperties())
    {
      ss << "\t" << prop.Type()->Name() << " " << prop.Name() << "\n";
    }
    ss << "methods:\n";
    for (const auto& method : metaObject->OwnMethods())
    {
      ss << "\t" << method.Signature() << "\n";
    }
    return ss.str();
  }

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  struct Point
  {
    int x;
    int y;
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }


    void GetLength()
    {
      std::cout << "[Nan Const] GetLength = " << m_Length << std::endl;
    }

    void GetLength() const
    {
      std::cout << "[Const] GetLength = " << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
      std::cout << "[Double] Length = " << m_Length << std::endl;
    }


    void SetLength(int value)
    {
      m_Length += value;
      std::cout << "[INT] Length = " << m_Length << std::endl;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    void SetPosition(const Point& point)
    {
      m_Position = point;
    }

    Point GetPosition() const
    {
      return m_Position;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length),
          MakeMemberProperty("M_Position", &Button::m_Position),
        },
        {
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(int)>(&Button::SetLength)),
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(double)>(&Button::SetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)()>(&Button::GetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)() const>(&Button::GetLength)),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo),
          MakeMetaMethod("F_GetPosition", &Button::GetPosition),
          MakeMetaMethod("F_SetPosition", &Button::SetPosition)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
    Point m_Position;
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class RegisterObject
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };


  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }

  template <typename T>
  std::optional<std::pair<T, T>> ParsePair(
    std::string_view sv,
    char left_bracket = '[',
    char right_bracket = ']',
    char separator = ','
  )
  {
    const char* first = sv.data();
    const char* last = first + sv.size();

    if (first == last || *first != left_bracket)
      return std::nullopt;
    ++first;

    // 解析左值
    T a{};
    auto res = std::from_chars(first, last, a);
    if (res.ec != std::errc{})
      return std::nullopt;
    first = res.ptr;
    // 跳过空白和分隔符
    while (first != last && (*first == ' ' || *first == separator)) ++first;
    // 解析右值
    T b{};
    res = std::from_chars(first, last, b);
    if (res.ec != std::errc{})
      return std::nullopt;
    first = res.ptr;
    while (first != last && *first == ' ') ++first;
    if (first == last || *first != right_bracket)
      return std::nullopt;
    return std::pair<T, T>{a, b};
  }

  inline void RegisterBuiltinMetaTypes()
  {
    RegisterMetaType<int>("int", [](const std::any& value)-> std::optional<std::string>
                          {
                            if (const auto intPointer = std::any_cast<int>(&value))
                              return std::to_string(*intPointer);
                            return std::nullopt;
                          }, [](const std::string_view text)-> std::optional<std::any>
                          {
                            int value{};
                            const char* first = text.data();
                            const char* last = first + text.size();
                            auto [ptr, error] = std::from_chars(first, last, value);
                            if (error != std::errc{} || ptr != last)
                              return std::nullopt;
                            return std::any{value};
                          });
    RegisterMetaType<Point>("Point", [](const std::any& value)-> std::optional<std::string>
                            {
                              if (const auto pointPointer = std::any_cast<Point>(&value))
                              {
                                MetaValue v0(pointPointer->x);
                                MetaValue v1(pointPointer->y);
                                std::stringstream ss;
                                ss << "[";
                                ss << v0.Serialize().value();
                                ss << " , ";
                                ss << v1.Serialize().value();
                                ss << "]";
                                return ss.str();
                              }
                              return std::nullopt;
                            }, [](const std::string_view string)-> std::optional<std::any>
                            {
                              if (string.empty())
                                return std::nullopt;
                              int value = 0;
                              if (auto point = ParsePair<int>(string); point.has_value())
                              {
                                Point p;
                                auto [x,y] = point.value();
                                p.x = x;
                                p.y = y;
                                return p;
                              }
                              return std::nullopt;
                            });
  }
}

namespace Version16
{
  class Object;
  //using MetaValue = std::variant<int, bool, double, std::string>;

  template <typename T>
  using StoredArgument = std::decay_t<T>;

  class MetaTypeRegister;

  class SerializedValue
  {
  public:
    using Array = std::vector<SerializedValue>;
    using Object = std::unordered_map<std::string, SerializedValue>;
    using Storage = std::variant<std::monostate, bool, int, double, std::string, Array, Object>;
    SerializedValue() = default;

    explicit SerializedValue(bool value): m_Value(value)
    {
    }

    explicit SerializedValue(int value): m_Value(value)
    {
    }

    explicit SerializedValue(double value): m_Value(value)
    {
    }

    explicit SerializedValue(std::string value): m_Value(std::move(value))
    {
    }

    explicit SerializedValue(Array value): m_Value(std::move(value))
    {
    }

    explicit SerializedValue(Object value): m_Value(std::move(value))
    {
    }

    template <typename T>
    const T* GetIf() const noexcept
    {
      return std::get_if<T>(&m_Value);
    }

    template <typename T>
    bool Is() const
    {
      return std::holds_alternative<T>(m_Value);
    }

  private:
    Storage m_Value;
  };

  class MetaType
  {
  public:
    using Serializer = std::function<std::optional<SerializedValue>(const std::any&)>;
    using Deserializer = std::function<std::optional<std::any>(const SerializedValue&)>;


    explicit

    MetaType(std::type_index type, std::string name, Serializer serializer = nullptr,
             Deserializer deserializer = nullptr)
      : m_TypeId(type), m_Name(std::move(name)), m_Serializer(std::move(serializer)),
        m_Deserializer(std::move(deserializer))
    {
    }

    ~MetaType() = default;

    [[nodiscard]] std::type_index TypeId() const noexcept
    {
      return m_TypeId;
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    void SetName(const std::string_view name)
    {
      m_Name = name;
    }

    void SetSerializeAndDeserialize(Serializer serializer, Deserializer deserializer)
    {
      m_Serializer = std::move(serializer);
      m_Deserializer = std::move(deserializer);
    }

    std::optional<SerializedValue> Serialize(const std::any& value) const
    {
      if (!m_Serializer)
        return std::nullopt;
      return m_Serializer(value);
    }

    std::optional<std::any> Deserialize(const SerializedValue& value) const
    {
      if (!m_Deserializer)
        return std::nullopt;
      return m_Deserializer(value);
    }

    bool IsSerializable() const noexcept
    {
      return m_Serializer && m_Deserializer;
    }

  private:
    std::type_index m_TypeId;
    std::string m_Name;
    Serializer m_Serializer;
    Deserializer m_Deserializer;
  };

  class MetaTypeRegister
  {
  public:
    using MetaTypeMap = std::unordered_map<std::type_index, std::unique_ptr<MetaType>>;
    using MetaNameMap = std::unordered_map<std::string, const MetaType*>;


    template <typename T>
    static bool Register(const std::string_view metaName, MetaType::Serializer serializer,
                         MetaType::Deserializer deserializer)
    {
      MetaType* metaType = GetOrCreate<T>();
      if (!metaType)
        return false;
      // 同或运算
      if ((serializer == nullptr) != (deserializer == nullptr))
        return false;
      // 查询是否注册过
      if (const MetaType* namedMetaType = FindMetaType(metaName))
      {
        if (metaType == namedMetaType)
          return true;
        return false;
      }
      //若新的名字没有注册过，再看看旧名字是否被注册过了,若被注册了，则不能重复注册，返回false
      if (FindMetaType(metaType->Name()))
      {
        return false;
      }
      metaType->SetName(metaName);
      if (serializer && deserializer)
        metaType->SetSerializeAndDeserialize(std::move(serializer), std::move(deserializer));
      auto& metaNameType = GetMetaNameMap();
      metaNameType.emplace(std::string(metaName), metaType);
      return true;
    }


    template <typename T>
    static const MetaType* GetMetaType()
    {
      return GetOrCreate<T>();
    }

    static const MetaType* FindMetaType(const std::string_view name)
    {
      const auto it = GetMetaNameMap().find(std::string(name));
      if (it == GetMetaNameMap().end())
        return nullptr;
      return it->second;
    }

  private:
    static MetaTypeMap& GetMetaTypeMap()
    {
      static MetaTypeMap metaTypeMap;
      return metaTypeMap;
    }

    static MetaNameMap& GetMetaNameMap()
    {
      static MetaNameMap metaNameMap;
      return metaNameMap;
    }

    static MetaType* FindMetaType(const std::type_index& typeId)
    {
      auto& metaTypeMap = GetMetaTypeMap();
      if (const auto iter = metaTypeMap.find(typeId); iter != metaTypeMap.end())
      {
        return iter->second.get();
      }

      return nullptr;
    }

    template <typename T>
    static MetaType* GetOrCreate()
    {
      using Type = StoredArgument<T>;
      std::type_index typeId = typeid(StoredArgument<T>);
      if (auto metaType = FindMetaType(typeId))
        return metaType;
      auto& metaTypeMap = GetMetaTypeMap();
      auto metaType = std::make_unique<MetaType>(typeId, typeid(Type).name());
      auto [iter, inserted] = metaTypeMap.emplace(typeId, std::move(metaType));
      return iter->second.get();
    }
  };

  template <typename T>
  const MetaType* GetMetaType()
  {
    return MetaTypeRegister::GetMetaType<T>();
  }

  template <typename T>
  bool RegisterMetaType(const std::string_view name, MetaType::Serializer serializer = nullptr,
                        MetaType::Deserializer deserializer = nullptr)
  {
    using Type = StoredArgument<T>;
    return MetaTypeRegister::Register<Type>(name, std::move(serializer), std::move(deserializer));
  }


  class MetaValue
  {
  public:
    template <typename T>
      requires (!std::same_as<StoredArgument<T>, MetaValue>)
    MetaValue(T&& value)
    {
      m_Value = std::forward<T>(value);
      m_Type = GetMetaType<T>();
    }


    MetaValue(const MetaValue& other) = default;
    MetaValue(MetaValue&& other) noexcept = default;
    MetaValue& operator=(const MetaValue& other) = default;
    MetaValue& operator=(MetaValue&& other) noexcept = default;

    [[nodiscard]] const MetaType* Type() const
    {
      return m_Type;
    }

    template <typename T>
    [[nodiscard]] bool Is() const noexcept
    {
      return m_Type == GetMetaType<T>();
    }

    template <typename T>
    [[nodiscard]] const StoredArgument<T>* GetIf() const noexcept
    {
      return std::any_cast<StoredArgument<T>>(&m_Value);
    }

    std::optional<SerializedValue> Serialize() const
    {
      return m_Type->Serialize(m_Value);
    }

    std::optional<MetaValue> static Deserialize(const MetaType* type, const SerializedValue& serializedValue)
    {
      if (!type)
        return std::nullopt;
      auto value = type->Deserialize(serializedValue);
      if (!value)
        return std::nullopt;
      if (std::type_index(value->type()) != type->TypeId())
        return std::nullopt;

      return MetaValue{std::move(*value), type};
    }

  private:
    MetaValue(std::any value, const MetaType* type): m_Value(std::move(value)), m_Type(type)
    {
    }

  private:
    std::any m_Value;
    const MetaType* m_Type;
  };


  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, const MetaType* type, Getter getter, Setter setter): m_Name(std::move(name)),
      m_Type(type), m_Getter(std::move(getter)),
      m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] const MetaType* Type() const noexcept
    {
      return m_Type;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    const MetaType* m_Type;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method, std::vector<const MetaType*> metaTypes, const MetaType* returnType,
               bool isConst = false): m_Name(std::move(name)),
                                      m_Parameters(std::move(metaTypes)),
                                      m_IsConst(isConst),
                                      m_ReturnType(returnType), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

    [[nodiscard]] bool Matches(std::span<const MetaValue> parameters) const
    {
      if (parameters.size() != m_Parameters.size())
        return false;
      for (int i = 0; i < parameters.size(); i++)
      {
        if (m_Parameters[i] != parameters[i].Type())
          return false;
      }
      return true;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
      return m_IsConst;
    }

    [[nodiscard]] const MetaType* ReturnType() const noexcept
    {
      return m_ReturnType;
    }

    [[nodiscard]] std::span<const MetaType* const> ParameterTypes() const noexcept
    {
      return m_Parameters;
    }

    [[nodiscard]] std::size_t GetParameterCount() const noexcept
    {
      return m_Parameters.size();
    }

    [[nodiscard]] const MetaType* ParameterType(const std::size_t index) const noexcept
    {
      return m_Parameters[index];
    }

    [[nodiscard]] std::string Signature() const
    {
      std::ostringstream out;
      out << m_ReturnType->Name() << ' ' << m_Name << '(';
      for (std::size_t i = 0; i < m_Parameters.size(); ++i)
      {
        if (i != 0)
          out << ", ";
        out << m_Parameters[i]->Name();
      }
      out << ')';
      if (m_IsConst)
        out << " const";
      return out.str();
    }

  private:
    std::string m_Name;
    std::vector<const MetaType*> m_Parameters;
    bool m_IsConst = false;
    const MetaType* m_ReturnType;
    Method m_Method;
  };

  template <typename Method, size_t Index>
  using MethodStoredArgument = StoredArgument<std::tuple_element_t<Index, typename FunctionTraits<Method>::ArgsTuple>>;

  template <typename Owner, typename Method, std::size_t... Index>
  InvokeResult InvokeMethod(Owner owner, Method method, std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, MethodStoredArgument<Method, Index>> && ...));
    using R = typename FunctionTraits<Method>::ReturnType;
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>() != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, GetMetaType<Value>(), getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, GetMetaType<Value>(), accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, false);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args) const)
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](const Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<const Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, true);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name,
                                               std::span<const MetaValue> parameters, bool isConst = false) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        if (!Matches(method, parameters))
          continue;
        if (isConst != method.IsConst())
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name, parameters, isConst);
      return nullptr;
    }

    [[nodiscard]] bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters) const
    {
      return method.Matches(parameters);
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

    [[nodiscard]] std::span<const MetaMethod> OwnMethods() const noexcept
    {
      return m_Methods;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  inline std::string DumpMetaObject(const MetaObject* metaObject)
  {
    std::stringstream ss;
    const auto cls = metaObject->SuperClass()
                       ? std::string(" : ") + std::string(metaObject->SuperClass()->ClassName())
                       : "";
    ss << metaObject->ClassName() << cls << "\n";
    ss << "properties:\n";
    for (const auto& prop : metaObject->OwnProperties())
    {
      ss << "\t" << prop.Type()->Name() << " " << prop.Name() << "\n";
    }
    ss << "methods:\n";
    for (const auto& method : metaObject->OwnMethods())
    {
      ss << "\t" << method.Signature() << "\n";
    }
    return ss.str();
  }

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  struct Point
  {
    int x;
    int y;
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }


    void GetLength()
    {
      std::cout << "[Nan Const] GetLength = " << m_Length << std::endl;
    }

    void GetLength() const
    {
      std::cout << "[Const] GetLength = " << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
      std::cout << "[Double] Length = " << m_Length << std::endl;
    }


    void SetLength(int value)
    {
      m_Length += value;
      std::cout << "[INT] Length = " << m_Length << std::endl;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    void SetPosition(const Point& point)
    {
      m_Position = point;
    }

    Point GetPosition() const
    {
      return m_Position;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length),
          MakeMemberProperty("M_Position", &Button::m_Position),
        },
        {
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(int)>(&Button::SetLength)),
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(double)>(&Button::SetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)()>(&Button::GetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)() const>(&Button::GetLength)),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo),
          MakeMetaMethod("F_GetPosition", &Button::GetPosition),
          MakeMetaMethod("F_SetPosition", &Button::SetPosition)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
    Point m_Position;
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class RegisterObject
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };


  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }

  inline void RegisterBuiltinMetaTypes()
  {
    RegisterMetaType<int>("int", [](const std::any& value)-> std::optional<SerializedValue>
                          {
                            if (const auto intPointer = std::any_cast<int>(&value))
                              return SerializedValue(*intPointer);
                            return std::nullopt;
                          }, [](const SerializedValue& serializedValue)-> std::optional<std::any>
                          {
                            if (const auto value = serializedValue.GetIf<int>())
                              return std::any{*value};
                            return std::nullopt;
                          });
    RegisterMetaType<Point>("Point", [](const std::any& value)-> std::optional<SerializedValue>
                            {
                              if (const auto pointPointer = std::any_cast<Point>(&value))
                              {
                                SerializedValue::Object object;
                                object.emplace("x", pointPointer->x);
                                object.emplace("y", pointPointer->y);
                                return SerializedValue(std::move(object));
                              }
                              return std::nullopt;
                            }, [](const SerializedValue& serializedValue)-> std::optional<std::any>
                            {
                              const auto* object = serializedValue.GetIf<SerializedValue::Object>();
                              if (!object)
                                return std::nullopt;
                              const auto xIter = object->find("x");
                              const auto yIter = object->find("y");
                              if (xIter == object->end() || yIter == object->end())
                              {
                                return std::nullopt;
                              }
                              const int* x = xIter->second.GetIf<int>();
                              const int* y = yIter->second.GetIf<int>();
                              if (!x || !y)
                                return std::nullopt;
                              return std::any{Point{*x, *y}};
                            });
  }
}


namespace Version17
{
  class Object;
  //using MetaValue = std::variant<int, bool, double, std::string>;

  template <typename T>
  using StoredArgument = std::decay_t<T>;

  class MetaTypeRegister;

  class SerializedValue
  {
  public:
    using Array = std::vector<SerializedValue>;
    using Object = std::unordered_map<std::string, SerializedValue>;
    using Storage = std::variant<std::monostate, bool, int, double, std::string, Array, Object>;
    SerializedValue() = default;

    explicit SerializedValue(bool value): m_Value(value)
    {
    }

    explicit SerializedValue(int value): m_Value(value)
    {
    }

    explicit SerializedValue(double value): m_Value(value)
    {
    }

    explicit SerializedValue(std::string value): m_Value(std::move(value))
    {
    }

    explicit SerializedValue(Array value): m_Value(std::move(value))
    {
    }

    explicit SerializedValue(Object value): m_Value(std::move(value))
    {
    }

    template <typename T>
    const T* GetIf() const noexcept
    {
      return std::get_if<T>(&m_Value);
    }

    template <typename T>
    bool Is() const
    {
      return std::holds_alternative<T>(m_Value);
    }

  private:
    Storage m_Value;
  };

  struct SerializedProperty
  {
    std::string m_TypeName;
    SerializedValue m_Value;
  };

  struct SerializedObject
  {
    std::string m_ClassName;
    std::unordered_map<std::string, SerializedProperty> m_Properties;
  };


  class MetaType
  {
  public:
    using Serializer = std::function<std::optional<SerializedValue>(const std::any&)>;
    using Deserializer = std::function<std::optional<std::any>(const SerializedValue&)>;


    explicit

    MetaType(std::type_index type, std::string name, Serializer serializer = nullptr,
             Deserializer deserializer = nullptr)
      : m_TypeId(type), m_Name(std::move(name)), m_Serializer(std::move(serializer)),
        m_Deserializer(std::move(deserializer))
    {
    }

    ~MetaType() = default;

    [[nodiscard]] std::type_index TypeId() const noexcept
    {
      return m_TypeId;
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    void SetName(const std::string_view name)
    {
      m_Name = name;
    }

    void SetSerializeAndDeserialize(Serializer serializer, Deserializer deserializer)
    {
      m_Serializer = std::move(serializer);
      m_Deserializer = std::move(deserializer);
    }

    std::optional<SerializedValue> Serialize(const std::any& value) const
    {
      if (!m_Serializer)
        return std::nullopt;
      return m_Serializer(value);
    }

    std::optional<std::any> Deserialize(const SerializedValue& value) const
    {
      if (!m_Deserializer)
        return std::nullopt;
      return m_Deserializer(value);
    }

    bool IsSerializable() const noexcept
    {
      return m_Serializer && m_Deserializer;
    }

  private:
    std::type_index m_TypeId;
    std::string m_Name;
    Serializer m_Serializer;
    Deserializer m_Deserializer;
  };

  class MetaTypeRegister
  {
  public:
    using MetaTypeMap = std::unordered_map<std::type_index, std::unique_ptr<MetaType>>;
    using MetaNameMap = std::unordered_map<std::string, const MetaType*>;


    template <typename T>
    static bool Register(const std::string_view metaName, MetaType::Serializer serializer,
                         MetaType::Deserializer deserializer)
    {
      MetaType* metaType = GetOrCreate<T>();
      if (!metaType)
        return false;
      // 同或运算
      if ((serializer == nullptr) != (deserializer == nullptr))
        return false;
      // 查询是否注册过
      if (const MetaType* namedMetaType = FindMetaType(metaName))
      {
        if (metaType == namedMetaType)
          return true;
        return false;
      }
      //若新的名字没有注册过，再看看旧名字是否被注册过了,若被注册了，则不能重复注册，返回false
      if (FindMetaType(metaType->Name()))
      {
        return false;
      }
      metaType->SetName(metaName);
      if (serializer && deserializer)
        metaType->SetSerializeAndDeserialize(std::move(serializer), std::move(deserializer));
      auto& metaNameType = GetMetaNameMap();
      metaNameType.emplace(std::string(metaName), metaType);
      return true;
    }


    template <typename T>
    static const MetaType* GetMetaType()
    {
      return GetOrCreate<T>();
    }

    static const MetaType* FindMetaType(const std::string_view name)
    {
      const auto it = GetMetaNameMap().find(std::string(name));
      if (it == GetMetaNameMap().end())
        return nullptr;
      return it->second;
    }

  private:
    static MetaTypeMap& GetMetaTypeMap()
    {
      static MetaTypeMap metaTypeMap;
      return metaTypeMap;
    }

    static MetaNameMap& GetMetaNameMap()
    {
      static MetaNameMap metaNameMap;
      return metaNameMap;
    }

    static MetaType* FindMetaType(const std::type_index& typeId)
    {
      auto& metaTypeMap = GetMetaTypeMap();
      if (const auto iter = metaTypeMap.find(typeId); iter != metaTypeMap.end())
      {
        return iter->second.get();
      }

      return nullptr;
    }

    template <typename T>
    static MetaType* GetOrCreate()
    {
      using Type = StoredArgument<T>;
      std::type_index typeId = typeid(StoredArgument<T>);
      if (auto metaType = FindMetaType(typeId))
        return metaType;
      auto& metaTypeMap = GetMetaTypeMap();
      auto metaType = std::make_unique<MetaType>(typeId, typeid(Type).name());
      auto [iter, inserted] = metaTypeMap.emplace(typeId, std::move(metaType));
      return iter->second.get();
    }
  };

  template <typename T>
  const MetaType* GetMetaType()
  {
    return MetaTypeRegister::GetMetaType<T>();
  }

  template <typename T>
  bool RegisterMetaType(const std::string_view name, MetaType::Serializer serializer = nullptr,
                        MetaType::Deserializer deserializer = nullptr)
  {
    using Type = StoredArgument<T>;
    return MetaTypeRegister::Register<Type>(name, std::move(serializer), std::move(deserializer));
  }


  class MetaValue
  {
  public:
    template <typename T>
      requires (!std::same_as<StoredArgument<T>, MetaValue>)
    MetaValue(T&& value)
    {
      m_Value = std::forward<T>(value);
      m_Type = GetMetaType<T>();
    }


    MetaValue(const MetaValue& other) = default;
    MetaValue(MetaValue&& other) noexcept = default;
    MetaValue& operator=(const MetaValue& other) = default;
    MetaValue& operator=(MetaValue&& other) noexcept = default;

    [[nodiscard]] const MetaType* Type() const
    {
      return m_Type;
    }

    template <typename T>
    [[nodiscard]] bool Is() const noexcept
    {
      return m_Type == GetMetaType<T>();
    }

    template <typename T>
    [[nodiscard]] const StoredArgument<T>* GetIf() const noexcept
    {
      return std::any_cast<StoredArgument<T>>(&m_Value);
    }

    std::optional<SerializedValue> Serialize() const
    {
      return m_Type->Serialize(m_Value);
    }

    std::optional<MetaValue> static Deserialize(const MetaType* type, const SerializedValue& serializedValue)
    {
      if (!type)
        return std::nullopt;
      auto value = type->Deserialize(serializedValue);
      if (!value)
        return std::nullopt;
      if (std::type_index(value->type()) != type->TypeId())
        return std::nullopt;

      return MetaValue{std::move(*value), type};
    }

  private:
    MetaValue(std::any value, const MetaType* type): m_Value(std::move(value)), m_Type(type)
    {
    }

  private:
    std::any m_Value;
    const MetaType* m_Type;
  };


  struct InvokeResult
  {
    bool invokeSuccess{};
    std::optional<MetaValue> invokeResult;
  };


  class MetaProperty
  {
  public:
    using Getter = std::function<MetaValue(const Object*)>;
    using Setter = std::function<bool(Object*, const MetaValue&)>;

    MetaProperty(std::string name, const MetaType* type, Getter getter, Setter setter): m_Name(std::move(name)),
      m_Type(type), m_Getter(std::move(getter)),
      m_Setter(std::move(setter))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }

    [[nodiscard]] const MetaType* Type() const noexcept
    {
      return m_Type;
    }

    [[nodiscard]] MetaValue Read(const Object* object) const
    {
      return m_Getter(object);
    }

    bool Write(Object* object, const MetaValue& value) const
    {
      if (!m_Setter)
        return false;
      return m_Setter(object, value);;
    }

    [[nodiscard]] bool IsWritable() const noexcept
    {
      return static_cast<bool>(m_Setter);
    }

  private:
    std::string m_Name;
    const MetaType* m_Type;
    Getter m_Getter;
    Setter m_Setter;
  };

  class MetaMethod
  {
  public:
    using Arguments = std::span<const MetaValue>;
    using Method = std::function<InvokeResult(Object*, Arguments)>;

    MetaMethod(std::string name, Method method, std::vector<const MetaType*> metaTypes, const MetaType* returnType,
               bool isConst = false): m_Name(std::move(name)),
                                      m_Parameters(std::move(metaTypes)),
                                      m_IsConst(isConst),
                                      m_ReturnType(returnType), m_Method(std::move(method))
    {
    }

    [[nodiscard]] std::string_view Name() const noexcept
    {
      return m_Name;
    }


    InvokeResult Invoke(Object* object, Arguments args) const
    {
      if (!m_Method)
        return {false, std::nullopt};
      return m_Method(object, args);
    }

    [[nodiscard]] bool IsInvoke() const noexcept
    {
      return static_cast<bool>(m_Method);
    }

    [[nodiscard]] bool Matches(std::span<const MetaValue> parameters) const
    {
      if (parameters.size() != m_Parameters.size())
        return false;
      for (int i = 0; i < parameters.size(); i++)
      {
        if (m_Parameters[i] != parameters[i].Type())
          return false;
      }
      return true;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
      return m_IsConst;
    }

    [[nodiscard]] const MetaType* ReturnType() const noexcept
    {
      return m_ReturnType;
    }

    [[nodiscard]] std::span<const MetaType* const> ParameterTypes() const noexcept
    {
      return m_Parameters;
    }

    [[nodiscard]] std::size_t GetParameterCount() const noexcept
    {
      return m_Parameters.size();
    }

    [[nodiscard]] const MetaType* ParameterType(const std::size_t index) const noexcept
    {
      return m_Parameters[index];
    }

    [[nodiscard]] std::string Signature() const
    {
      std::ostringstream out;
      out << m_ReturnType->Name() << ' ' << m_Name << '(';
      for (std::size_t i = 0; i < m_Parameters.size(); ++i)
      {
        if (i != 0)
          out << ", ";
        out << m_Parameters[i]->Name();
      }
      out << ')';
      if (m_IsConst)
        out << " const";
      return out.str();
    }

  private:
    std::string m_Name;
    std::vector<const MetaType*> m_Parameters;
    bool m_IsConst = false;
    const MetaType* m_ReturnType;
    Method m_Method;
  };

  template <typename Method, size_t Index>
  using MethodStoredArgument = StoredArgument<std::tuple_element_t<Index, typename FunctionTraits<Method>::ArgsTuple>>;

  template <typename Owner, typename Method, std::size_t... Index>
  InvokeResult InvokeMethod(Owner owner, Method method, std::span<const MetaValue> arguments,
                            std::index_sequence<Index...>)
  {
    // 能否构造MetaValue
    static_assert((std::is_constructible_v<MetaValue, MethodStoredArgument<Method, Index>> && ...));
    using R = typename FunctionTraits<Method>::ReturnType;
    if constexpr (!std::is_void_v<R>)
    {
      static_assert(std::is_constructible_v<MetaValue, StoredArgument<R>>);
    }
    // 参数类型匹配
    if (!((arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>() != nullptr) && ...))
    {
      return {false, std::nullopt};
    }
    if constexpr (std::is_void_v<R>)
    {
      std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::nullopt};
    }
    else
    {
      R ret = std::invoke(method, owner, *arguments[Index].template GetIf<MethodStoredArgument<Method, Index>>()...);
      return {true, std::move(ret)};
    }
  }

  template <typename Owner, typename Value>
  MetaProperty MakeMemberProperty(const std::string& name, Value Owner::* member)
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto getter = [member](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return owner->*member;
      throw std::bad_cast{};
    };
    auto setter = [member](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      owner->*member = *typedValue;
      return true;
    };
    return MetaProperty(name, GetMetaType<Value>(), getter, setter);
  }

  template <typename Owner, typename Value>
  MetaProperty MakeAccessorProperty(const std::string& name, Value (Owner::*getter)() const,
                                    bool (Owner::*setter)(Value))
  {
    static_assert(std::derived_from<Owner, Object>);

    static_assert(std::is_constructible_v<MetaValue, Value>);

    auto accessorGetter = [getter](const Object* object)-> MetaValue
    {
      if (const auto* owner = dynamic_cast<const Owner*>(object))
        return std::invoke(getter, owner);
      throw std::bad_cast{};
    };
    auto accessorSetter = [setter](Object* object, const MetaValue& value)-> bool
    {
      const auto* typedValue = value.GetIf<StoredArgument<Value>>();
      auto* owner = dynamic_cast<Owner*>(object);
      if (!typedValue || !owner)
      {
        return false;
      }
      return std::invoke(setter, owner, *typedValue);
    };
    return MetaProperty(name, GetMetaType<Value>(), accessorGetter, accessorSetter);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args))
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, false);
  }

  template <typename Owner, typename R, typename... Args>
  MetaMethod MakeMetaMethod(const std::string& name, R (Owner::*method)(Args... args) const)
  {
    static_assert(std::derived_from<Owner, Object>);

    auto metaMethod = [method](const Object* object, MetaMethod::Arguments arguments)-> InvokeResult
    {
      if (arguments.size() != sizeof...(Args))
        return {false, std::nullopt};
      auto* owner = dynamic_cast<const Owner*>(object);
      if (!owner)
        return {false, std::nullopt};
      return InvokeMethod(owner, method, arguments, std::index_sequence_for<Args...>{});
    };
    std::vector<const MetaType*> metaTypeIds = {GetMetaType<Args>()...};
    const MetaType* returnType = GetMetaType<R>();
    return MetaMethod(name, metaMethod, metaTypeIds, returnType, true);
  }

  class MetaObject
  {
  public:
    MetaObject(const std::string_view className,
               const MetaObject* superClass, std::vector<MetaProperty> memberProperties = {},
               std::vector<MetaMethod> methods = {}) noexcept
      : m_ClassName(className), m_SuperClass(superClass), m_Properties(std::move(memberProperties)),
        m_Methods(std::move(methods))
    {
    }

    [[nodiscard]] std::string_view ClassName() const noexcept
    {
      return m_ClassName;
    }

    [[nodiscard]] const MetaObject* SuperClass() const noexcept
    {
      return m_SuperClass;
    }

    bool Inherits(const MetaObject* type) const
    {
      if (!type)
        return false;
      for (auto cur = this; cur; cur = cur->m_SuperClass)
      {
        if (cur == type)
          return true;
      }
      return false;
    }

    [[nodiscard]] const MetaProperty* FindProperty(const std::string_view name) const
    {
      for (const auto& metaProperty : m_Properties)
      {
        if (metaProperty.Name() != name)
          continue;
        return &metaProperty;
      }
      if (m_SuperClass)
        return m_SuperClass->FindProperty(name);
      return nullptr;
    }

    [[nodiscard]] const MetaMethod* FindMethod(const std::string_view name,
                                               std::span<const MetaValue> parameters, bool isConst = false) const
    {
      for (const auto& method : m_Methods)
      {
        if (method.Name() != name)
          continue;
        if (!Matches(method, parameters))
          continue;
        if (isConst != method.IsConst())
          continue;
        return &method;
      }
      if (m_SuperClass)
        return m_SuperClass->FindMethod(name, parameters, isConst);
      return nullptr;
    }

    [[nodiscard]] bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters) const
    {
      return method.Matches(parameters);
    }

    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

    [[nodiscard]] std::span<const MetaMethod> OwnMethods() const noexcept
    {
      return m_Methods;
    }

  private:
    std::string_view m_ClassName;
    const MetaObject* m_SuperClass;
    std::vector<MetaProperty> m_Properties;
    std::vector<MetaMethod> m_Methods;
  };

  inline std::string DumpMetaObject(const MetaObject* metaObject)
  {
    std::stringstream ss;
    const auto cls = metaObject->SuperClass()
                       ? std::string(" : ") + std::string(metaObject->SuperClass()->ClassName())
                       : "";
    ss << metaObject->ClassName() << cls << "\n";
    ss << "properties:\n";
    for (const auto& prop : metaObject->OwnProperties())
    {
      ss << "\t" << prop.Type()->Name() << " " << prop.Name() << "\n";
    }
    ss << "methods:\n";
    for (const auto& method : metaObject->OwnMethods())
    {
      ss << "\t" << method.Signature() << "\n";
    }
    return ss.str();
  }

  class Object
  {
  public:
    virtual void foo() = 0;

  public:
    virtual ~Object() = default;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Object",
        nullptr
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] virtual const MetaObject* GetMetaObject() const noexcept
    {
      return GetStaticMetaObject();
    }
  };

  class Widget : public Object
  {
  public:
    void foo() override
    {
    }

    bool m_Visible = true;

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Widget",
        Object::GetStaticMetaObject(), {MakeMemberProperty<Widget, bool>("M_Visible", &Widget::m_Visible)}
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }
  };

  struct Point
  {
    int x;
    int y;
  };

  class Button : public Widget
  {
  public:
    [[nodiscard]] std::string Text() const
    {
      return m_Text;
    }

    bool SetText(std::string text)
    {
      if (text.empty())
        return false;

      m_Text = std::move(text);
      return true;
    }


    void GetLength()
    {
      std::cout << "[Nan Const] GetLength = " << m_Length << std::endl;
    }

    void GetLength() const
    {
      std::cout << "[Const] GetLength = " << m_Length << std::endl;
    }

    void SetLength(double length)
    {
      m_Length += length;
      std::cout << "[Double] Length = " << m_Length << std::endl;
    }


    void SetLength(int value)
    {
      m_Length += value;
      std::cout << "[INT] Length = " << m_Length << std::endl;
    }

    double SetSumLength(double A, double B)
    {
      m_Length = A + B;
      return m_Length;
    }

    void SetPosition(const Point& point)
    {
      m_Position = point;
    }

    Point GetPosition() const
    {
      return m_Position;
    }

    std::string GetInfo(std::string text, int count)
    {
      std::cout << "Text = " << text << " Count = " << count << std::endl;
      return text + std::to_string(count);
    }

    static const MetaObject* GetStaticMetaObject() noexcept
    {
      static const MetaObject StaticMetaObject{
        "Button",
        Widget::GetStaticMetaObject(), {
          MakeMemberProperty("M_Length", &Button::m_Length),
          MakeMemberProperty("M_Position", &Button::m_Position),
        },
        {
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(int)>(&Button::SetLength)),
          MakeMetaMethod("F_SetLength", static_cast<void(Button::*)(double)>(&Button::SetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)()>(&Button::GetLength)),
          MakeMetaMethod("F_GetLength", static_cast<void(Button::*)() const>(&Button::GetLength)),
          MakeMetaMethod("F_SetSumLength", &Button::SetSumLength),
          MakeMetaMethod("F_GetInfo", &Button::GetInfo),
          MakeMetaMethod("F_GetPosition", &Button::GetPosition),
          MakeMetaMethod("F_SetPosition", &Button::SetPosition)
        }
      };
      return &StaticMetaObject;
    }

    [[nodiscard]] const MetaObject* GetMetaObject() const noexcept override
    {
      return GetStaticMetaObject();
    }

  private:
    double m_Length = 0;
    std::string m_Text = "OK";
    Point m_Position = {};
  };

  template <typename T>
  concept MetaObjectType = std::derived_from<T, Object>;

  class RegisterObject
  {
  public:
    struct TypeEntry
    {
      const MetaObject* m_Type = nullptr;
      std::function<std::unique_ptr<Object>()> m_CreateFun;
    };

    template <MetaObjectType T>
    void Register()
    {
      const MetaObject* metaObject = T::GetStaticMetaObject();
      const auto className = metaObject->ClassName();
      TypeEntry entry;
      entry.m_Type = metaObject;
      if constexpr (!std::is_abstract_v<T> &&
        std::is_default_constructible_v<T>)
      {
        entry.m_CreateFun = [] { return std::make_unique<T>(); };
      }
      if (auto [iter, inserted] =
          m_MetaObjectMaps.try_emplace(std::string(className), entry);
        !inserted)
        throw std::logic_error("Class name already registered");
    }

    [[nodiscard]] std::unique_ptr<Object>
    CreateObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      if (const auto& [metaObject, createFun] = iter->second; createFun)
      {
        return createFun();
      }
      return nullptr;
    }

    [[nodiscard]] const MetaObject*
    GetMetaObject(const std::string& className) const
    {
      const auto iter = m_MetaObjectMaps.find(className);
      if (iter == m_MetaObjectMaps.end())
        return nullptr;
      const auto& [metaObject, createFun] = iter->second;
      return metaObject;
    }

  private:
    std::unordered_map<std::string, TypeEntry> m_MetaObjectMaps;
  };


  using MetaValueTable = std::unordered_map<std::string, MetaValue>;

  inline bool ApplyProperty(Object* object, const MetaValueTable& properties)
  {
    if (!object)
      return false;
    for (const auto& [key, value] : properties)
    {
      const auto metaProperty = object->GetMetaObject()->FindProperty(key);
      if (!metaProperty || !metaProperty->Write(object, value))
        return false;
    }
    return true;
  }

  template <typename T>
  constexpr std::string_view GetMetaTypeId()
  {
    using Type = StoredArgument<T>;
    if constexpr (std::is_same_v<Type, bool>)
      return "bool";
    if constexpr (std::is_same_v<Type, int>)
      return "int";
    if constexpr (std::is_same_v<Type, double>)
      return "double";
    if constexpr (std::is_same_v<Type, std::string>)
      return "std::string";
    return "";
  }

  template <typename T>
  concept BuildInTypeConcept = std::same_as<std::decay_t<T>, int> ||
    std::same_as<std::decay_t<T>, bool> ||
    std::same_as<std::decay_t<T>, double> ||
    std::same_as<std::decay_t<T>, std::string>;

  template <BuildInTypeConcept T>
  void RegisterBuiltinMetaTypes()
  {
    std::string_view metaTypeId = GetMetaTypeId<T>();
    if (metaTypeId.empty())
      return;
    RegisterMetaType<T>(metaTypeId, [](const std::any& value)-> std::optional<SerializedValue>
                        {
                          if (const auto intPointer = std::any_cast<T>(&value))
                            return SerializedValue(*intPointer);
                          return std::nullopt;
                        }, [](const SerializedValue& serializedValue)-> std::optional<std::any>
                        {
                          if (const auto value = serializedValue.GetIf<T>())
                            return std::any{*value};
                          return std::nullopt;
                        });
  }

  inline void RegisterCustomMetaTypes()
  {
    RegisterMetaType<Point>("Point", [](const std::any& value)-> std::optional<SerializedValue>
                            {
                              if (const auto pointPointer = std::any_cast<Point>(&value))
                              {
                                SerializedValue::Object object;
                                object.emplace("x", pointPointer->x);
                                object.emplace("y", pointPointer->y);
                                return SerializedValue(std::move(object));
                              }
                              return std::nullopt;
                            }, [](const SerializedValue& serializedValue)-> std::optional<std::any>
                            {
                              const auto* object = serializedValue.GetIf<SerializedValue::Object>();
                              if (!object)
                                return std::nullopt;
                              const auto xIter = object->find("x");
                              const auto yIter = object->find("y");
                              if (xIter == object->end() || yIter == object->end())
                              {
                                return std::nullopt;
                              }
                              const int* x = xIter->second.GetIf<int>();
                              const int* y = yIter->second.GetIf<int>();
                              if (!x || !y)
                                return std::nullopt;
                              return std::any{Point{*x, *y}};
                            });
  }

  inline std::optional<SerializedObject> SerializeObject(const Object* object)
  {
    if (!object)
      return std::nullopt;
    auto metaObject = object->GetMetaObject();
    if (!metaObject)
      return std::nullopt;
    std::deque<const MetaObject*> metaObjects;
    for (auto m = metaObject; m; m = m->SuperClass())
      metaObjects.emplace_back(m);
    SerializedObject serializedObject;
    serializedObject.m_ClassName = metaObject->ClassName();
    while (!metaObjects.empty())
    {
      auto m = metaObjects.back();
      metaObjects.pop_back();
      for (const auto& p : m->OwnProperties())
      {
        auto value = p.Read(object).Serialize();
        if (!value.has_value())
          return std::nullopt;
        SerializedProperty serializedProperty{std::string(p.Type()->Name()), value.value()};
        serializedObject.m_Properties.insert_or_assign(std::string(p.Name()), serializedProperty);
      }
    }
    return serializedObject;
  }

  inline std::unique_ptr<Object> DeserializeObject(const SerializedObject& serializedObject,
                                                   const RegisterObject& registry)
  {
    auto className = serializedObject.m_ClassName;
    auto object = registry.CreateObject(className);
    if (!object)
      return nullptr;
    auto metaObject = object->GetMetaObject();
    if (!metaObject)
      return nullptr;
    for (const auto& [propertyName,serializedProperty] : serializedObject.m_Properties)
    {
      auto metaProperty = metaObject->FindProperty(propertyName);
      if (!metaProperty)
        continue;
      auto& [type,value] = serializedProperty;
      auto metaType = MetaTypeRegister::FindMetaType(type);
      if (!metaType || metaType != metaProperty->Type())
        return nullptr;
      auto metaValue = MetaValue::Deserialize(metaType, value);
      if (!metaValue)
        return nullptr;
      if (!metaProperty->Write(object.get(), metaValue.value()))
        return nullptr;
    }
    return object;
  }
}

#undef META_OBJECT
#undef META_ROOT_OBJECT

class MetaObjectManager
{
public:
  MetaObjectManager()
  {
    test17();
  }

  void test1()
  {
    using namespace Version1;
    Button button;
    Object* object = &button;

    const MetaObject* meta = object->GetMetaObject();

    assert(meta->ClassName() == "Button");

    assert(meta->Inherits(&Button::StaticMetaObject));
    assert(meta->Inherits(&Widget::StaticMetaObject));
    assert(meta->Inherits(&Object::StaticMetaObject));

    assert(!Widget::StaticMetaObject.Inherits(&Button::StaticMetaObject));

    assert(Object::StaticMetaObject.SuperClass() == nullptr);
  }

  void test2()
  {
    using namespace Version2;
    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();

    const std::unique_ptr<Object> object = registry.Create("Button");
    const auto button = registry.Create("Button");

    assert(button);
    assert(button->GetMetaObject() == Button::GetStaticMetaObject());

    assert(object);
    assert(object->GetMetaObject()->ClassName() == "Button");
    assert(object->GetMetaObject()->Inherits(Widget::GetStaticMetaObject()));
    assert(registry.Create("Unknown") == nullptr);
  }

  void test3()
  {
    using namespace Version3;
    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();

    auto buttonMeta = registry.GetMetaObject("Button");
    assert(buttonMeta);
    assert(buttonMeta == Button::GetStaticMetaObject());

    auto widgetObject = registry.CreateObject("Widget");
    assert(widgetObject);

    auto object = registry.CreateObject("Object");
    assert(!object);
    auto objectMeta = registry.GetMetaObject("Object");
    assert(objectMeta);
    assert(objectMeta == Object::GetStaticMetaObject());

    assert(registry.CreateObject("Unknown") == nullptr);
  }

  void test5()
  {
    using namespace Version5;
    Button button;
    Object* object = &button;
    const MetaObject* meta = object->GetMetaObject();
    auto text = meta->FindProperty("text");
    auto visible = meta->FindProperty("visible");
    assert(text);
    assert(visible);
    assert(std::get<std::string>(text->Read(object)) == "OK");
    assert(text->Write(object, std::string("Cancel")));
    assert(visible->Write(object,false));

    assert(button.m_Text == "Cancel");
    assert(button.m_Visible == false);

    assert(!text->Write(object, true));
  }

  void test6()
  {
    using namespace Version6;
    Button button;
    Object* object = &button;
    const MetaObject* meta = object->GetMetaObject();
    auto text = meta->FindProperty("textMethod");
    assert(text);
    assert(std::get<std::string>(text->Read(object)) == "OK");
    assert(text->Write(object, std::string("Cancel")));

    assert(button.Text() == "Cancel");
    assert(!text->Write(object, std::string(""))); // 空字符串不会被写入
    assert(button.Text() == "Cancel");
    /// 未知类型测试
    {
      ObjectRegister registry;
      registry.Register<Object>();
      registry.Register<Widget>();
      registry.Register<Button>();
      // 运行时创建对象
      auto tempObject = registry.CreateObject("Button");
      PropertyTable config{
        {"textMethod", std::string{"Confirm"}},
        {"visible", false}
      };
      // 修改未知具体类型对象
      assert(ApplyProperty(tempObject.get(), config));
    }
  }

  void test7()
  {
    using namespace Version7;
    Button button;
    Object* object = &button;
    const MetaObject* meta = object->GetMetaObject();
    auto clickMethod = meta->FindMethod("Click");
    assert(clickMethod);
    clickMethod->Invoke(object);
    assert(button.m_Clicked==1);
  }

  void test8()
  {
    using namespace Version8;
    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();
    std::vector<PropertyValue> param = {3};

    auto object = registry.CreateObject("Button");
    const MetaObject* meta = object->GetMetaObject();
    auto clickMethod = meta->FindMethod("F_AddClick");
    auto clickMember = meta->FindProperty("M_Click");
    assert(clickMethod);
    assert(clickMethod->Invoke(object.get(), param));
    assert(std::get<int>( clickMember->Read(object.get()))==3);

    // 参数数量错误
    assert(!clickMethod->Invoke(object.get(), {}));

    // 参数类型错误
    std::array<PropertyValue, 1> badArgs{std::string{"three"}};
    assert(!clickMethod->Invoke(object.get(), badArgs));
  }

  void test9()
  {
    using namespace Version9;
    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();
    std::vector<MetaValue> param0 = {3.0};

    auto object = registry.CreateObject("Button");
    const MetaObject* meta = object->GetMetaObject();
    auto setLengthMethod = meta->FindMethod("F_SetLength");
    auto lengthMember = meta->FindProperty("M_Length");
    assert(lengthMember);
    assert(setLengthMethod->Invoke(object.get(), param0));
    assert(std::get<double>( lengthMember->Read(object.get()))==3.0);

    // 多参数调用
    std::vector<MetaValue> param1 = {3.0, 6.8};
    std::vector<MetaValue> param2 = {std::string("Hello"), 5};
    auto setSumLengthMethod = meta->FindMethod("F_SetSumLength");
    auto getInfoMethod = meta->FindMethod("F_GetInfo");
    assert(setLengthMethod);
    assert(setSumLengthMethod);
    assert(getInfoMethod);

    assert(setSumLengthMethod->Invoke( object.get(),param1));
    assert(getInfoMethod->Invoke(object.get(),param2));
  }

  void test10()
  {
    using namespace Version10;
    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();
    std::vector<MetaValue> param0 = {3.0};

    auto object = registry.CreateObject("Button");
    const MetaObject* meta = object->GetMetaObject();
    auto setLengthMethod = meta->FindMethod("F_SetLength");
    auto lengthMember = meta->FindProperty("M_Length");
    assert(lengthMember);
    assert(setLengthMethod->Invoke(object.get(), param0).invokeSuccess);
    assert(std::get<double>( lengthMember->Read(object.get()))==3.0);

    // 多参数调用
    std::vector<MetaValue> param1 = {3.0, 6.8};
    std::vector<MetaValue> param2 = {std::string("Hello"), 5};
    std::vector<MetaValue> param3 = {std::string("Hello"), "world"};
    auto setSumLengthMethod = meta->FindMethod("F_SetSumLength");
    auto getInfoMethod = meta->FindMethod("F_GetInfo");
    assert(setLengthMethod);
    assert(setSumLengthMethod);
    assert(getInfoMethod);

    auto result1 = setSumLengthMethod->Invoke(object.get(), param1);
    auto result2 = getInfoMethod->Invoke(object.get(), param2);
    auto result3 = getInfoMethod->Invoke(object.get(), param3);
    assert(result1.invokeSuccess);
    assert(result2.invokeSuccess);
    assert(!result3.invokeSuccess);
  }

  void test11()
  {
    using namespace Version11;
    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();
    std::vector<MetaValue> param0 = {3.0};

    auto object = registry.CreateObject("Button");
    const MetaObject* meta = object->GetMetaObject();

    // 函数重载调用
    std::vector<MetaValue> param1 = {3.0};
    std::vector<MetaValue> param2 = {5};
    std::vector<MetaValue> param3 = {std::string("Hello")};
    auto setLengthMethodD = meta->FindMethod("F_SetLength", param1);
    auto setLengthMethodI = meta->FindMethod("F_SetLength", param2);
    auto setLengthMethodS = meta->FindMethod("F_SetLength", param3);
    auto getLengthMethodNC = meta->FindMethod("F_GetLength", {});
    auto getLengthMethodC = meta->FindMethod("F_GetLength", {}, true);
    assert(setLengthMethodD);
    assert(setLengthMethodI);
    assert(!setLengthMethodS);

    auto result1 = setLengthMethodD->Invoke(object.get(), param1);
    auto result2 = setLengthMethodI->Invoke(object.get(), param2);
    auto result3 = getLengthMethodNC->Invoke(object.get(), {});
    auto result4 = getLengthMethodC->Invoke(object.get(), {});
    assert(result1.invokeSuccess);
    assert(result2.invokeSuccess);
  }

  void test12()
  {
    using namespace Version12;
    assert(GetMetaType<int>() ==GetMetaType<const int&>());

    Point point{1, 2};
    MetaValue value1{point};
    MetaValue value2{Point{3, 4}};
    MetaValue value3 = &value1;
    assert(value1.Type()==value2.Type());
    assert(value1.Is<Point>());
    assert(value1.Is<const Point&>());
    //assert(value3.Is<const Point&>());
    const Point* pPoint = value1.GetIf<Point>();
    assert(pPoint->x == 1);
    assert(pPoint->y == 2);
    assert(value1.GetIf<int>() == nullptr);

    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();
    auto object = registry.CreateObject("Button");
    const MetaObject* meta = object->GetMetaObject();
    std::vector<MetaValue> param0 = {value2};
    auto getPositionMethod = meta->FindMethod("F_GetPosition", {}, true);
    auto setPositionMethod = meta->FindMethod("F_SetPosition", param0);
    auto result0 = setPositionMethod->Invoke(object.get(), param0);
    auto result1 = getPositionMethod->Invoke(object.get(), {});
    assert(result1.invokeSuccess);
    assert(result1.invokeResult.has_value());
    auto position = result1.invokeResult.value().GetIf<Point>();
    assert(position);
    assert(position->x == 3);
    assert(position->y == 4);
  }

  void test13()
  {
    using namespace Version13;
    ObjectRegister registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();
    auto object = registry.CreateObject("Button");
    const MetaObject* meta = object->GetMetaObject();
    auto getPositionMethod = meta->FindMethod("F_GetPosition", {}, true);
    auto s = getPositionMethod->Signature();
    std::cout << s << std::endl;
    auto metaInfo = DumpMetaObject(meta);
    std::cout << metaInfo << std::endl;
  }

  void test14()
  {
    using namespace Version14;
    RegisterObject registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();

    RegisterMetaType<int>("int");
    RegisterMetaType<double>("double");
    RegisterMetaType<std::string>("string");

    assert(RegisterMetaType<int>("int")); // true
    assert(RegisterMetaType<int>("int")); // true，幂等
    assert(!RegisterMetaType<int>("integer")); // false，不允许改名
    assert(!RegisterMetaType<Point>("int")); // false，名字已被占用

    const MetaType* pointBefore = GetMetaType<const Point&>();
    assert(pointBefore);
    assert(RegisterMetaType<Point>("Point"));
    const MetaType* pointAfter = GetMetaType<Point>();
    assert(pointBefore == pointAfter);
    assert(pointAfter->Name() == "Point");
    const MetaType* boolType = GetMetaType<bool>();
    assert(boolType);
    assert(MetaTypeRegister::FindMetaType("Point")==pointAfter);


    auto object = registry.CreateObject("Button");
    const MetaObject* meta = object->GetMetaObject();
    auto getPositionMethod = meta->FindMethod("F_GetPosition", {}, true);
    auto s = getPositionMethod->Signature();
    std::cout << s << std::endl;
    auto metaInfo = DumpMetaObject(meta);
    std::cout << metaInfo << std::endl;
  }

  void test15()
  {
    using namespace Version15;
    RegisterBuiltinMetaTypes();
    RegisterObject registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();


    const auto metaIntType = GetMetaType<int>();
    MetaValue value{1};
    auto text = value.Serialize();
    auto tempValue = MetaValue::Deserialize(metaIntType, "5");

    RegisterMetaType<double>("double");
    RegisterMetaType<std::string>("string");
    RegisterMetaType<Point>("Point");

    auto pointMetaType = GetMetaType<Point>();
    assert(pointMetaType);
    auto pointValue = MetaValue::Deserialize(pointMetaType, "[3,4]");
    assert(pointValue.has_value());
    auto pointValuePtr = pointValue.value().GetIf<Point>();
    assert(pointValuePtr);
    assert(pointValuePtr->x == 3);
    auto pointText = pointValue.value().Serialize();
    assert(pointText.has_value());
  }

  void test16()
  {
    using namespace Version16;
    RegisterBuiltinMetaTypes();
    MetaValue source{Point{3, 4}};
    auto serialized = source.Serialize();

    assert(serialized);
    assert(serialized->Is<SerializedValue::Object>());

    auto restored = MetaValue::Deserialize(GetMetaType<Point>(), *serialized);
    assert(restored);

    const Point* point = restored->GetIf<Point>();
    assert(point);
    assert(point->x == 3);
    assert(point->y == 4);

    // 缺醒字段
    SerializedValue::Object missingY{{"x", SerializedValue{3}}};
    assert(!MetaValue::Deserialize(GetMetaType<Point>(),SerializedValue{std::move(missingY)}));

    // 错误字段
    SerializedValue::Object wrongX{{"x", SerializedValue{std::string{"3"}}}, {"y", SerializedValue{4}}};
    assert(!MetaValue::Deserialize(GetMetaType<Point>(),SerializedValue{std::move(wrongX)}));
  }

  void test17()
  {
    using namespace Version17;
    RegisterObject registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();

    RegisterBuiltinMetaTypes<int>();
    RegisterBuiltinMetaTypes<bool>();
    RegisterBuiltinMetaTypes<std::string>();
    RegisterBuiltinMetaTypes<double>();
    RegisterCustomMetaTypes();
    MetaValue position{Point{3, 4}};
    MetaValue visible{true};
    MetaValue length{3.0};
    auto source = registry.CreateObject("Button");
    assert(source);
    auto metaObject = source->GetMetaObject();
    assert(metaObject);
    auto m_VisibleProp = metaObject->FindProperty("M_Visible");
    auto m_LengthProp = metaObject->FindProperty("M_Length");
    auto m_PositionProp = metaObject->FindProperty("M_Position");
    assert(m_VisibleProp && m_VisibleProp->Write(source.get(), visible));
    assert(m_LengthProp && m_LengthProp->Write(source.get(), length));
    assert(m_PositionProp && m_PositionProp->Write(source.get(), position));

    auto snapshot = SerializeObject(source.get());
    assert(snapshot);
    assert(snapshot->m_ClassName == "Button");
    assert(snapshot->m_Properties.size() == 3);
    auto restored = DeserializeObject(*snapshot, registry);
    assert(restored);
    const MetaObject* restoredMeta = restored->GetMetaObject();
    MetaValue restoredVisibleValue = restoredMeta->FindProperty("M_Visible")->Read(restored.get());
    const bool* restoredVisible = restoredVisibleValue.GetIf<bool>();
    assert(restoredVisible && *restoredVisible);
  }
};

//static MetaObjectManager meta_object_manager;
