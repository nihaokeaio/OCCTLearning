//
// Created by ZQD on 26-8-7.
//

#pragma once
#include <any>
#include <array>
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
#include <sstream>

#include "CommonTraits.h"


namespace MiniMetaObject
{
  class Object;

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


    MetaValue() = default;

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
      const auto value = type->Deserialize(serializedValue);
      if (!value)
        return std::nullopt;
      if (std::type_index(value->type()) != type->TypeId())
        return std::nullopt;

      return MetaValue{std::move(*value), type};
    }

    template <typename T>
    bool Equal(MetaValue& value)
    {
      if (!Is<T>() || !value.Is<T>())
        return false;
      return *std::any_cast<T>(&m_Value) == *std::any_cast<T>(value.m_Value);
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


    [[nodiscard]] std::span<const MetaProperty> OwnProperties() const noexcept
    {
      return m_Properties;
    }

    [[nodiscard]] std::span<const MetaMethod> OwnMethods() const noexcept
    {
      return m_Methods;
    }

  private:
    [[nodiscard]] static bool Matches(const MetaMethod& method, std::span<const MetaValue> parameters)
    {
      return method.Matches(parameters);
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

  struct Point
  {
    int x;
    int y;
  };

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



