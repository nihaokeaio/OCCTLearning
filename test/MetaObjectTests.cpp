//
// Created by ZQD on 26-8-5.
//
#include <gtest/gtest.h>
#include "MetaObjectManager.h"


TEST(GoogleTestDemo, BasicAssertion)
{
    EXPECT_EQ(1 + 1, 2);
}

TEST(ObjectSerializationTest, RoundTripsButton)
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

    auto source = registry.CreateObject("Button");
    ASSERT_NE(source, nullptr);

    const MetaObject* metaObject = source->GetMetaObject();
    ASSERT_NE(metaObject, nullptr);
    const MetaProperty* visibleProperty = metaObject->FindProperty("M_Visible");
    const MetaProperty* lengthProperty = metaObject->FindProperty("M_Length");
    const MetaProperty* positionProperty = metaObject->FindProperty("M_Position");

    ASSERT_NE(visibleProperty, nullptr);
    ASSERT_NE(lengthProperty, nullptr);
    ASSERT_NE(positionProperty, nullptr);

    EXPECT_TRUE(visibleProperty->Write(source.get(), MetaValue{true}));

    EXPECT_TRUE(lengthProperty->Write(source.get(), MetaValue{3.0}));

    EXPECT_TRUE(positionProperty->Write(source.get(),MetaValue{Point{3, 4}}));

    auto snapshot = SerializeObject(source.get());
    ASSERT_TRUE(snapshot.has_value());
    EXPECT_EQ(snapshot->m_ClassName, "Button");
    EXPECT_EQ(snapshot->m_Properties.size(), 3u);
    auto restored = DeserializeObject(*snapshot, registry);
    ASSERT_NE(restored, nullptr);
    const MetaObject* restoredMeta = restored->GetMetaObject();
    ASSERT_NE(restoredMeta, nullptr);

    const MetaProperty* restoredVisibleProperty = restoredMeta->FindProperty("M_Visible");

    ASSERT_NE(restoredVisibleProperty, nullptr);

    // 必须保存 MetaValue，保证 GetIf 返回的指针有效
    MetaValue restoredVisibleValue = restoredVisibleProperty->Read(restored.get());

    const bool* restoredVisible = restoredVisibleValue.GetIf<bool>();
    ASSERT_NE(restoredVisible, nullptr);
    EXPECT_TRUE(*restoredVisible);
}

TEST(ObjectDeserializationTest, RejectsUnknownClass)
{
    using namespace Version17;
    RegisterObject registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();

    SerializedObject snapshot;
    snapshot.m_ClassName = "UnknownClass";
    auto restored = DeserializeObject(snapshot, registry);
    EXPECT_EQ(restored, nullptr);
}

TEST(ObjectDeserializationTest, RejectsMismatchedPropertyType)
{
    using namespace Version17;
    RegisterObject registry;
    registry.Register<Object>();
    registry.Register<Widget>();
    registry.Register<Button>();

    RegisterBuiltinMetaTypes<int>();
    RegisterBuiltinMetaTypes<bool>();
    RegisterBuiltinMetaTypes<double>();
    RegisterBuiltinMetaTypes<std::string>();

    SerializedObject snapshot;
    snapshot.m_ClassName = "Button";
    snapshot.m_Properties.emplace("M_Length", SerializedProperty{"bool", SerializedValue{true}});
    auto restored = DeserializeObject(snapshot, registry);
    EXPECT_EQ(restored, nullptr);
}

TEST(ObjectDeserializationTest, RejectsMalformedPoint)
{
    // Point 缺少 x/y 或字段类型错误
    using namespace Version17;
    // 缺醒字段
    SerializedValue::Object missingY{{"x", SerializedValue{3}}};
    assert(!MetaValue::Deserialize(GetMetaType<Point>(),SerializedValue{std::move(missingY)}));

    // 错误字段
    SerializedValue::Object wrongX{{"x", SerializedValue{std::string{"3"}}}, {"y", SerializedValue{4}}};
    assert(!MetaValue::Deserialize(GetMetaType<Point>(),SerializedValue{std::move(wrongX)}));
}
