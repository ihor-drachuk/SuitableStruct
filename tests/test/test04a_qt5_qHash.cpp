/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QCoreApplication>
#include <SuitableStruct/Internals/Qt5Replica.h>

// Test Qt5Replica hash functions vs Qt native qHash (only for Qt5)
#if QT_VERSION_MAJOR == 5

TEST(SuitableStruct, Qt5qHash_BasicTypes_Qt5Compatibility)
{
    // Test null
    QJsonValue nullValue;
    EXPECT_EQ(Qt5Replica::qHash(nullValue), qHash(nullValue));

    // Test bool values
    QJsonValue trueValue(true);
    QJsonValue falseValue(false);
    EXPECT_EQ(Qt5Replica::qHash(trueValue), qHash(trueValue));
    EXPECT_EQ(Qt5Replica::qHash(falseValue), qHash(falseValue));

    // Test integers
    QJsonValue intValue(42);
    QJsonValue negIntValue(-42);
    QJsonValue zeroValue(0);
    EXPECT_EQ(Qt5Replica::qHash(intValue), qHash(intValue));
    EXPECT_EQ(Qt5Replica::qHash(negIntValue), qHash(negIntValue));
    EXPECT_EQ(Qt5Replica::qHash(zeroValue), qHash(zeroValue));

    // Test doubles
    QJsonValue doubleValue(3.14);
    QJsonValue negDoubleValue(-3.14);
    QJsonValue zeroDoubleValue(0.0);
    EXPECT_EQ(Qt5Replica::qHash(doubleValue), qHash(doubleValue));
    EXPECT_EQ(Qt5Replica::qHash(negDoubleValue), qHash(negDoubleValue));
    EXPECT_EQ(Qt5Replica::qHash(zeroDoubleValue), qHash(zeroDoubleValue));

    // Test strings
    QJsonValue stringValue(QString("hello"));
    QJsonValue emptyStringValue(QString(""));
    QJsonValue unicodeValue(QString("🌟test🌟"));
    EXPECT_EQ(Qt5Replica::qHash(stringValue), qHash(stringValue));
    EXPECT_EQ(Qt5Replica::qHash(emptyStringValue), qHash(emptyStringValue));
    EXPECT_EQ(Qt5Replica::qHash(unicodeValue), qHash(unicodeValue));

    // Test undefined
    QJsonValue undefinedValue(QJsonValue::Undefined);
    EXPECT_EQ(Qt5Replica::qHash(undefinedValue), qHash(undefinedValue));
}

TEST(SuitableStruct, Qt5qHash_Arrays_Qt5Compatibility)
{
    // Test empty array
    QJsonArray emptyArray;
    QJsonValue emptyArrayValue(emptyArray);
    EXPECT_EQ(Qt5Replica::qHash(emptyArrayValue), qHash(emptyArrayValue));

    // Test simple array
    QJsonArray simpleArray;
    simpleArray.append(1);
    simpleArray.append(2);
    simpleArray.append(3);
    QJsonValue simpleArrayValue(simpleArray);
    EXPECT_EQ(Qt5Replica::qHash(simpleArrayValue), qHash(simpleArrayValue));

    // Test mixed array
    QJsonArray mixedArray;
    mixedArray.append(true);
    mixedArray.append(QString("test"));
    mixedArray.append(42);
    mixedArray.append(3.14);
    QJsonValue mixedArrayValue(mixedArray);
    EXPECT_EQ(Qt5Replica::qHash(mixedArrayValue), qHash(mixedArrayValue));

    // Test nested array
    QJsonArray nestedArray;
    nestedArray.append(simpleArray);
    nestedArray.append(emptyArray);
    QJsonValue nestedArrayValue(nestedArray);
    EXPECT_EQ(Qt5Replica::qHash(nestedArrayValue), qHash(nestedArrayValue));
}

TEST(SuitableStruct, Qt5qHash_Objects_Qt5Compatibility)
{
    // Test empty object
    QJsonObject emptyObject;
    QJsonValue emptyObjectValue(emptyObject);
    EXPECT_EQ(Qt5Replica::qHash(emptyObjectValue), qHash(emptyObjectValue));

    // Test simple object
    QJsonObject simpleObject;
    simpleObject["name"] = QString("John");
    simpleObject["age"] = 30;
    simpleObject["active"] = true;
    QJsonValue simpleObjectValue(simpleObject);
    EXPECT_EQ(Qt5Replica::qHash(simpleObjectValue), qHash(simpleObjectValue));

    // Test object with different types
    QJsonObject mixedObject;
    mixedObject["string"] = QString("test");
    mixedObject["number"] = 42;
    mixedObject["double"] = 3.14;
    mixedObject["bool"] = false;
    mixedObject["null"] = QJsonValue();
    QJsonValue mixedObjectValue(mixedObject);
    EXPECT_EQ(Qt5Replica::qHash(mixedObjectValue), qHash(mixedObjectValue));
}

TEST(SuitableStruct, Qt5qHash_NestedStructures_Qt5Compatibility)
{
    // Create some base structures
    QJsonArray simpleArray;
    simpleArray.append(1);
    simpleArray.append(2);
    simpleArray.append(3);

    QJsonObject simpleObject;
    simpleObject["name"] = QString("John");
    simpleObject["age"] = 30;
    simpleObject["active"] = true;

    // Test nested object with user and scores
    QJsonObject nestedObject;
    nestedObject["user"] = simpleObject;
    nestedObject["scores"] = simpleArray;
    QJsonValue nestedObjectValue(nestedObject);
    EXPECT_EQ(Qt5Replica::qHash(nestedObjectValue), qHash(nestedObjectValue));

    // Test array of objects
    QJsonArray arrayOfObjects;
    arrayOfObjects.append(simpleObject);
    arrayOfObjects.append(QJsonObject());
    arrayOfObjects.append(nestedObject);
    QJsonValue arrayOfObjectsValue(arrayOfObjects);
    EXPECT_EQ(Qt5Replica::qHash(arrayOfObjectsValue), qHash(arrayOfObjectsValue));
}

TEST(SuitableStruct, Qt5qHash_SeedsConsistency_Qt5Compatibility)
{
    QJsonValue testValue(QString("test"));

    // Test with different seeds
    EXPECT_EQ(Qt5Replica::qHash(testValue, 0), qHash(testValue, 0));
    EXPECT_EQ(Qt5Replica::qHash(testValue, 42), qHash(testValue, 42));
    EXPECT_EQ(Qt5Replica::qHash(testValue, 12345), qHash(testValue, 12345));
    EXPECT_EQ(Qt5Replica::qHash(testValue, UINT_MAX), qHash(testValue, UINT_MAX));

    // Test that different seeds produce different results
    EXPECT_NE(Qt5Replica::qHash(testValue, 0), Qt5Replica::qHash(testValue, 42));
    EXPECT_NE(qHash(testValue, 0), qHash(testValue, 42));
}

TEST(SuitableStruct, Qt5qHash_Consistency_Qt5Compatibility)
{
    // Test that same values produce same hashes
    QJsonValue val1(QString("consistency"));
    QJsonValue val2(QString("consiste") + "ncy");

    EXPECT_EQ(Qt5Replica::qHash(val1), Qt5Replica::qHash(val2));
    EXPECT_EQ(qHash(val1), qHash(val2));
    EXPECT_EQ(Qt5Replica::qHash(val1), qHash(val1));
    EXPECT_EQ(Qt5Replica::qHash(val2), qHash(val2));
}

TEST(SuitableStruct, Qt5qHash_DifferentValues_Qt5Compatibility)
{
    // Test that different values usually produce different hashes
    QJsonValue val1(QString("test1"));
    QJsonValue val2(QString("test2"));
    QJsonValue val3(123);
    QJsonValue val4(123.0);

    EXPECT_EQ(Qt5Replica::qHash(val1), qHash(val1));
    EXPECT_EQ(Qt5Replica::qHash(val2), qHash(val2));
    EXPECT_EQ(Qt5Replica::qHash(val3), qHash(val3));
    EXPECT_EQ(Qt5Replica::qHash(val4), qHash(val4));
}

#endif // QT_VERSION_MAJOR == 5

// Test exact hash values for cross-Qt-version compatibility.
// These tests run for any Qt version but use Qt5Replica for deterministic results.

TEST(SuitableStruct, Qt5qHash_ExactHashValues_CrossVersionCompatibility)
{
    // These exact values ensure our hash implementation is deterministic
    // and produces the same results regardless of Qt version.

    // Basic types
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue()), 0x00000000); // null
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(true)), 0x00000001);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(false)), 0x00000000);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(QJsonValue::Undefined)), 0x00000000);

    // Numbers
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(0)), 0x00000000);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(1)), 0x00001D4F);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(-1)), 0x00001DCF);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(0.0)), 0x00000000);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(1.0)), 0x00001D4F);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(-1.0)), 0x00001DCF);

    // Strings
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(QString(""))), 0x00000000);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(QString("test"))), 0x00364492);

    // Empty containers should have predictable hashes
    QJsonArray emptyArray;
    QJsonObject emptyObject;
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(emptyArray)), 0x00000000);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(emptyObject)), 0x00000000);

    // Simple containers
    QJsonArray simpleArray;
    simpleArray.append(1);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(simpleArray)), 0x9E379708);

    QJsonObject simpleObject;
    simpleObject["key"] = QString("value");
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(simpleObject)), 0x62B8C8E1);
}

TEST(SuitableStruct, Qt5qHash_StringHashValues_CrossVersionCompatibility)
{
    // Test specific string hashes for consistency across versions
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(QString(""))), 0x00000000);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(QString("a"))), 0x00000061);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(QString("hello"))), 0x05E918D2);

    // Different strings should (usually) have different hashes
    EXPECT_NE(Qt5Replica::qHash(QJsonValue(QString("hello"))), Qt5Replica::qHash(QJsonValue(QString("world"))));
}

TEST(SuitableStruct, Qt5qHash_NumberHashValues_CrossVersionCompatibility)
{
    // Test specific number hashes for consistency across versions
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(0)), 0x00000000);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(1)), 0x00001D4F);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(42)), 0x0000089B);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(3.14159)), 0x8EFEDB2C);

    // Same values should always produce same hashes (consistency test)
    QJsonValue value_0(0);
    QJsonValue value_1(1);
    QJsonValue value_42(42);
    QJsonValue value_pi(3.14159);

    EXPECT_EQ(Qt5Replica::qHash(value_0), Qt5Replica::qHash(QJsonValue(0)));
    EXPECT_EQ(Qt5Replica::qHash(value_1), Qt5Replica::qHash(QJsonValue(1)));
    EXPECT_EQ(Qt5Replica::qHash(value_42), Qt5Replica::qHash(QJsonValue(42)));
    EXPECT_EQ(Qt5Replica::qHash(value_pi), Qt5Replica::qHash(QJsonValue(3.14159)));

    // Different values should (usually) have different hashes
    EXPECT_NE(Qt5Replica::qHash(value_0), Qt5Replica::qHash(value_1));
    EXPECT_NE(Qt5Replica::qHash(value_1), Qt5Replica::qHash(value_42));
    EXPECT_NE(Qt5Replica::qHash(value_42), Qt5Replica::qHash(value_pi));
}

TEST(SuitableStruct, Qt5qHash_ComplexStructureHashes_CrossVersionCompatibility)
{
    // Test that complex structures have consistent hashes
    QJsonObject obj;
    obj["key1"] = QString("value1");
    obj["key2"] = 42;
    obj["key3"] = true;

    QJsonArray arr;
    arr.append(1);
    arr.append(QString("test"));
    arr.append(obj);

    QJsonValue complexValue(arr);

    // Hash should be consistent across runs
    const auto hash1 = Qt5Replica::qHash(complexValue);
    const auto hash2 = Qt5Replica::qHash(complexValue);
    EXPECT_EQ(hash1, hash2);

    // Same structure built again should have same hash
    QJsonObject obj2;
    obj2["key1"] = QString("value1");
    obj2["key2"] = 42;
    obj2["key3"] = true;

    QJsonArray arr2;
    arr2.append(1);
    arr2.append(QString("test"));
    arr2.append(obj2);

    QJsonValue complexValue2(arr2);
    const auto hash3 = Qt5Replica::qHash(complexValue2);
    EXPECT_EQ(hash1, hash3);
    EXPECT_EQ(hash1, 0x5DA5CF2B);
}

TEST(SuitableStruct, Qt5qHash_SeedInfluence_CrossVersionCompatibility)
{
    QJsonValue testValue(QString("seedtest"));

    // Test specific hash values with different seeds
    EXPECT_EQ(Qt5Replica::qHash(testValue, 0), 0x3B0D7283);
    EXPECT_EQ(Qt5Replica::qHash(testValue, 1), 0xAB011D9F);
    EXPECT_EQ(Qt5Replica::qHash(testValue, 42), 0x1D500335);
    EXPECT_EQ(Qt5Replica::qHash(testValue, 12345), 0xB5056123);

    // Different seeds should produce different hashes
    const auto hash0 = Qt5Replica::qHash(testValue, 0);
    const auto hash1 = Qt5Replica::qHash(testValue, 1);
    const auto hash42 = Qt5Replica::qHash(testValue, 42);

    EXPECT_NE(hash0, hash1);
    EXPECT_NE(hash0, hash42);
    EXPECT_NE(hash1, hash42);

    // Same seed should always produce same hash (consistency)
    EXPECT_EQ(hash0, Qt5Replica::qHash(testValue, 0));
    EXPECT_EQ(hash1, Qt5Replica::qHash(testValue, 1));
    EXPECT_EQ(hash42, Qt5Replica::qHash(testValue, 42));
}

TEST(SuitableStruct, Qt5qHash_ComplexStructureExactHashes_CrossVersionCompatibility)
{
    // Test exact hash values for complex structures
    QJsonObject obj;
    obj["key1"] = QString("value1");
    obj["key2"] = 42;
    obj["key3"] = true;
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(obj)), 0x4909C6F4);

    QJsonArray arr;
    arr.append(1);
    arr.append(QString("test"));
    arr.append(obj);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(arr)), 0x5DA5CF2B);

    // Test with seeds
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(obj), 42), 0x49FF55BE);
    EXPECT_EQ(Qt5Replica::qHash(QJsonValue(arr), 42), 0x4270DB2C);
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
