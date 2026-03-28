/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Containers/QMap.h>
#include <SuitableStruct/Containers/QHash.h>

#include <QPointF>
#include <QSize>
#include <QSizeF>
#include <QRect>
#include <QRectF>
#include <QString>

using namespace SuitableStruct;

namespace {

struct GeometryStruct
{
    QPointF pointF;
    QSize size;
    QSizeF sizeF;
    QRect rect;
    QRectF rectF;

    auto ssTuple() const { return std::tie(pointF, size, sizeF, rect, rectF); }
    auto ssNamesTuple() const { return std::tie("pointF", "size", "sizeF", "rect", "rectF"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(GeometryStruct)
};

struct MapStruct
{
    QMap<QString, int> qmap;
    QHash<int, QString> qhash;

    auto ssTuple() const { return std::tie(qmap, qhash); }
    auto ssNamesTuple() const { return std::tie("qmap", "qhash"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(MapStruct)
};

} // namespace

TEST(SuitableStruct, NewTypes_Geometry_Binary)
{
    GeometryStruct a;
    a.pointF = QPointF(1.5, -2.7);
    a.size = QSize(800, 600);
    a.sizeF = QSizeF(19.2, 10.8);
    a.rect = QRect(10, 20, 300, 200);
    a.rectF = QRectF(1.1, 2.2, 3.3, 4.4);

    GeometryStruct b;
    ASSERT_NE(a, b);

    auto buf = ssSave(a);
    ssLoad(buf, b);
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, NewTypes_Geometry_Json)
{
    GeometryStruct a;
    a.pointF = QPointF(1.5, -2.7);
    a.size = QSize(800, 600);
    a.sizeF = QSizeF(19.2, 10.8);
    a.rect = QRect(10, 20, 300, 200);
    a.rectF = QRectF(1.1, 2.2, 3.3, 4.4);

    GeometryStruct b;
    ASSERT_NE(a, b);

    auto json = ssJsonSave(a);
    ssJsonLoad(json, b);
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, NewTypes_QMap_Binary)
{
    MapStruct a;
    a.qmap = {{"one", 1}, {"two", 2}, {"three", 3}};
    a.qhash = {{10, "ten"}, {20, "twenty"}};

    MapStruct b;
    ASSERT_NE(a, b);

    auto buf = ssSave(a);
    ssLoad(buf, b);
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, NewTypes_QMap_Json)
{
    MapStruct a;
    a.qmap = {{"one", 1}, {"two", 2}, {"three", 3}};
    a.qhash = {{10, "ten"}, {20, "twenty"}};

    MapStruct b;
    ASSERT_NE(a, b);

    auto json = ssJsonSave(a);
    ssJsonLoad(json, b);
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, NewTypes_Geometry_DefaultValues)
{
    GeometryStruct a, b;

    auto buf = ssSave(a);
    ssLoad(buf, b);
    ASSERT_EQ(a, b);

    auto json = ssJsonSave(a);
    ssJsonLoad(json, b);
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, NewTypes_QMap_Empty)
{
    MapStruct a, b;

    auto buf = ssSave(a);
    ssLoad(buf, b);
    ASSERT_EQ(a, b);

    auto json = ssJsonSave(a);
    ssJsonLoad(json, b);
    ASSERT_EQ(a, b);
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
