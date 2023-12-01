/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Internals/DefaultTypes.h>

#include <SuitableStruct/SerializerJson.h>
#include <variant>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QByteArray>
#include <QString>
#include <QPoint>
#include <QJsonObject>
#include <QColor>
#include <QDataStream>
#include <QDateTime>
#include <QTimeZone>

namespace {
void setupDataStream(QDataStream& ds) {
    ds.setByteOrder(QDataStream::BigEndian);
    ds.setFloatingPointPrecision(QDataStream::DoublePrecision);
    ds.setVersion(QDataStream::Qt_5_15);
}
} // namespace
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

namespace SuitableStruct {

Buffer ssSaveImpl(const std::string& value)
{
    Buffer buffer;
    buffer.write((uint64_t)value.size());
    buffer.writeRaw(value.data(), value.size());
    return buffer;
}

void ssLoadImpl(BufferReader& buffer, std::string& value)
{
    // Load & swap is not needed here because it's implemented in ssLoad

    uint64_t sz;
    buffer.read(sz);

    value.resize(sz);
    buffer.readRaw(value.data(), sz);
}


#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

Buffer ssSaveImpl(const QByteArray& value)
{
    Buffer result;
    result.write((uint64_t)value.size());
    result.writeRaw(value.constData(), value.size());
    return result;
}

void ssLoadImpl(BufferReader& buffer, QByteArray& value)
{
    uint64_t sz;
    buffer.read(sz);
    value.resize(sz);
    buffer.readRaw(value.data(), sz);
}

Buffer ssSaveImpl(const QString& value)
{
    return ssSaveImpl(value.toUtf8());
}

void ssLoadImpl(BufferReader& buffer, QString& value)
{
    value = QString::fromUtf8(ssLoadImplRet<QByteArray>(buffer));
}

Buffer ssSaveImpl(const QPoint& value)
{
    Buffer result;
    result.write(value.x());
    result.write(value.y());
    return result;
}

void ssLoadImpl(BufferReader& buffer, QPoint& value)
{
    ssLoadImpl(buffer, value.rx());
    ssLoadImpl(buffer, value.ry());
}

Buffer ssSaveImpl(const QColor& value)
{
    Buffer result;
    result.writeRaw(&value, sizeof(value));
    return result;
}

void ssLoadImpl(BufferReader& buffer, QColor& value)
{
    buffer.readRaw(&value, sizeof(value));
}

Buffer ssSaveImpl(const QJsonValue& value)
{
    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);
    setupDataStream(ds);
    ds << value;
    return ssSaveImpl(data);
}

void ssLoadImpl(BufferReader& buffer, QJsonValue& value)
{
    QByteArray data;
    ssLoadImpl(buffer, data);
    QDataStream ds (&data, QIODevice::ReadOnly);
    setupDataStream(ds);
    ds >> value;
}

Buffer ssSaveImpl(const QTimeZone& value)
{
    return ssSaveImpl(value.id());
}

void ssLoadImpl(BufferReader& buffer, QTimeZone& value)
{
    decltype(value.id()) temp;
    ssLoadImpl(buffer, temp);
    value = temp.isEmpty() ? QTimeZone() : QTimeZone(temp);
}

Buffer ssSaveImpl(const QDate& value)
{
    return ssSaveImpl(value.toJulianDay());
}

void ssLoadImpl(BufferReader& buffer, QDate& value)
{
    decltype(value.toJulianDay()) temp;
    ssLoadImpl(buffer, temp);
    value = QDate::fromJulianDay(temp);
}

Buffer ssSaveImpl(const QTime& value)
{
    return ssSaveImpl(value.msecsSinceStartOfDay());
}

void ssLoadImpl(BufferReader& buffer, QTime& value)
{
    decltype(value.msecsSinceStartOfDay()) temp;
    ssLoadImpl(buffer, temp);
    value = QTime::fromMSecsSinceStartOfDay(temp);
}

Buffer ssSaveImpl(const QDateTime& value)
{
    Buffer result;
    result += ssSaveImpl(value.timeSpec());

    switch (value.timeSpec()) {
        case Qt::LocalTime: break;
        case Qt::UTC:       break;

        case Qt::OffsetFromUTC:
            result += ssSaveImpl(value.offsetFromUtc());
            break;

        case Qt::TimeZone:
            result += ssSaveImpl(value.timeZone());
            break;

        default:
            assert(false && "Unhandled timeSpec!");
    }

    result += ssSaveImpl(value.date());
    result += ssSaveImpl(value.time());

    return result;
}

void ssLoadImpl(BufferReader& buffer, QDateTime& value)
{
    using OffsetFromUtc = decltype(value.offsetFromUtc());

    const auto timeSpec = ssLoadImplRet<Qt::TimeSpec>(buffer);
    std::variant<std::monostate, OffsetFromUtc, QTimeZone> timeDetails;

    switch (timeSpec) {
        case Qt::LocalTime: break;
        case Qt::UTC:       break;

        case Qt::OffsetFromUTC: {
            timeDetails = ssLoadImplRet<OffsetFromUtc>(buffer);
            break;
        }

        case Qt::TimeZone:
            timeDetails = ssLoadImplRet<QTimeZone>(buffer);
            break;

        default:
            assert(false && "Unhandled timeSpec!");
    }

    const auto date = ssLoadImplRet<QDate>(buffer);
    const auto time = ssLoadImplRet<QTime>(buffer);

    switch (timeSpec) {
        case Qt::LocalTime:
        case Qt::UTC:
            value = QDateTime(date, time, timeSpec);
            break;

        case Qt::OffsetFromUTC:
            value = QDateTime(date, time, timeSpec, std::get<OffsetFromUtc>(timeDetails));
            break;

        case Qt::TimeZone:
            value = QDateTime(date, time, std::get<QTimeZone>(timeDetails));
            break;

        default:
            assert(false && "Unhandled timeSpec!");
    }
}

// ---- ssJsonSave/Load ----

QJsonValue ssJsonSaveImpl(bool value)
{
    return {value};
}

void ssJsonLoadImpl(const QJsonValue& src, bool& dst)
{
    if (src.isBool()) {
        dst = src.toBool();

    } else if (src.isDouble()) {
        const auto dstDouble = src.toDouble();
        const auto dstInt = src.toInt();

        if (!qFuzzyCompare(dstDouble, static_cast<double>(dstInt)))
            Internal::throwIntegrity();

        switch (dstInt) {
            case 0: dst = false; break;
            case 1: dst = true;  break;
            default: Internal::throwIntegrity();
        }

    } else {
        Internal::throwIntegrity();
    }
}

QJsonValue ssJsonSaveImpl(QChar value)
{
    return QString(value);
}

void ssJsonLoadImpl(const QJsonValue& src, QChar& dst)
{
    assert(src.toString().length() == 1);
    dst = src.toString().at(0);
}

QJsonValue ssJsonSaveImpl(const QString& value)
{
    return value;
}

void ssJsonLoadImpl(const QJsonValue& src, QString& dst)
{
    dst = src.toString();
}

QJsonValue ssJsonSaveImpl(const QByteArray& value)
{
    const auto result = QString::fromLatin1(value.toHex());
    return result.isEmpty() ? result : ("0x" + result);
}

void ssJsonLoadImpl(const QJsonValue& src, QByteArray& dst)
{
    assert(src.isString());
    const auto str = src.toString();

    if (str.isEmpty()) {
        dst.clear();
        return;
    }

    assert(str.mid(0, 2) == "0x");
    dst = QByteArray::fromHex(str.mid(2).toLatin1());
}

QJsonValue ssJsonSaveImpl(const std::string& value)
{
    return QString::fromStdString(value);
}

void ssJsonLoadImpl(const QJsonValue& src, std::string& dst)
{
    assert(src.isString());
    dst = src.toString().toStdString();
}

QJsonValue ssJsonSaveImpl(const QPoint& value)
{
    QJsonObject obj;
    obj["x"] = ssJsonSaveImpl(value.x());
    obj["y"] = ssJsonSaveImpl(value.y());
    return obj;
}

void ssJsonLoadImpl(const QJsonValue& src, QPoint& dst)
{
    assert(src.isObject());
    const auto obj = src.toObject();
    ssJsonLoadImpl(obj["x"], dst.rx());
    ssJsonLoadImpl(obj["y"], dst.ry());
}

QJsonValue ssJsonSaveImpl(const QColor& value)
{
    QJsonObject result;
    result["spec"] = ssJsonSaveImpl(value.spec());

    switch (value.spec()) {
        case QColor::Spec::Invalid:
            break;

        case QColor::Spec::Rgb:
            result["r"] = ssJsonSaveImpl(value.red());
            result["g"] = ssJsonSaveImpl(value.green());
            result["b"] = ssJsonSaveImpl(value.blue());
            result["a"] = ssJsonSaveImpl(value.alpha());
            break;

        case QColor::Spec::Hsv:
            result["h"] = ssJsonSaveImpl(value.hsvHue());
            result["s"] = ssJsonSaveImpl(value.hsvSaturation());
            result["v"] = ssJsonSaveImpl(value.value());
            result["a"] = ssJsonSaveImpl(value.alpha());
            break;

        case QColor::Spec::Cmyk:
            result["c"] = ssJsonSaveImpl(value.cyan());
            result["m"] = ssJsonSaveImpl(value.magenta());
            result["y"] = ssJsonSaveImpl(value.yellow());
            result["k"] = ssJsonSaveImpl(value.black());
            result["a"] = ssJsonSaveImpl(value.alpha());
            break;

        case QColor::Spec::Hsl:
            result["h"] = ssJsonSaveImpl(value.hslHue());
            result["s"] = ssJsonSaveImpl(value.hslSaturation());
            result["l"] = ssJsonSaveImpl(value.lightness());
            result["a"] = ssJsonSaveImpl(value.alpha());
            break;

        case QColor::Spec::ExtendedRgb: {
            const auto rgba64 = value.rgba64();
            result["r"] = ssJsonSaveImpl(rgba64.red());
            result["g"] = ssJsonSaveImpl(rgba64.green());
            result["b"] = ssJsonSaveImpl(rgba64.blue());
            result["a"] = ssJsonSaveImpl(rgba64.alpha());
            break;
        }

        default:
            assert(false && "Unhandled color spec!");
    }

    return result;
}

void ssJsonLoadImpl(const QJsonValue& src, QColor& dst)
{
    assert(src.isObject());
    const auto obj = src.toObject();

    QColor::Spec spec;
    ssJsonLoadImpl(obj["spec"], spec);

    dst = QColor(spec);

    switch (spec) {
        case QColor::Spec::Invalid:
            break;

        case QColor::Spec::Rgb: {
            using RgbComponent = decltype (QColor().red());
            RgbComponent r{}, g{}, b{}, a{};
            ssJsonLoadImpl(obj["r"], r);
            ssJsonLoadImpl(obj["g"], g);
            ssJsonLoadImpl(obj["b"], b);
            ssJsonLoadImpl(obj["a"], a);
            dst.setRgb(r, g, b, a);
            break;
        }

        case QColor::Spec::Hsv: {
            using HsvComponent = decltype (QColor().hsvHue());
            HsvComponent h{}, s{}, v{}, a{};
            ssJsonLoadImpl(obj["h"], h);
            ssJsonLoadImpl(obj["s"], s);
            ssJsonLoadImpl(obj["v"], v);
            ssJsonLoadImpl(obj["a"], a);
            dst.setHsv(h, s, v, a);
            break;
        }

        case QColor::Spec::Cmyk: {
            using CmykComponent = decltype (QColor().cyan());
            CmykComponent c{}, m{}, y{}, k{}, a{};
            ssJsonLoadImpl(obj["c"], c);
            ssJsonLoadImpl(obj["m"], m);
            ssJsonLoadImpl(obj["y"], y);
            ssJsonLoadImpl(obj["k"], k);
            ssJsonLoadImpl(obj["a"], a);
            dst.setCmyk(c, m, y, k, a);
            break;
        }

        case QColor::Spec::Hsl: {
            using HslComponent = decltype (QColor().hslHue());
            HslComponent h{}, s{}, l{}, a{};
            ssJsonLoadImpl(obj["h"], h);
            ssJsonLoadImpl(obj["s"], s);
            ssJsonLoadImpl(obj["l"], l);
            ssJsonLoadImpl(obj["a"], a);
            dst.setHsl(h, s, l, a);
            break;
        }

        case QColor::Spec::ExtendedRgb: {
            using Rgb64Component = decltype (QColor().rgba64().red());
            Rgb64Component r{}, g{}, b{}, a{};
            ssJsonLoadImpl(obj["r"], r);
            ssJsonLoadImpl(obj["g"], g);
            ssJsonLoadImpl(obj["b"], b);
            ssJsonLoadImpl(obj["a"], a);
            dst.setRgba64(QRgba64::fromRgba64(r, g, b, a));
            break;
        }
    }

    assert(false && "Unhandled color spec!");
}

QJsonValue ssJsonSaveImpl(const QJsonValue& value)
{
    return value;
}

void ssJsonLoadImpl(const QJsonValue& src, QJsonValue& dst)
{
    dst = src;
}


QJsonValue ssJsonSaveImpl(const QJsonObject& value)
{
    return value;
}

void ssJsonLoadImpl(const QJsonValue& src, QJsonObject& dst)
{
    assert(src.isObject());
    dst = src.toObject();
}

QJsonValue ssJsonSaveImpl(const QJsonArray& value)
{
    return value;
}

void ssJsonLoadImpl(const QJsonValue& src, QJsonArray& dst)
{
    assert(src.isArray());
    dst = src.toArray();
}

QJsonValue ssJsonSaveImpl(const QTimeZone& value)
{
    return QString::fromLatin1(value.id());
}

void ssJsonLoadImpl(const QJsonValue& src, QTimeZone& value)
{
    const auto tzId = src.toString();
    value = tzId.isEmpty() ? QTimeZone() : QTimeZone(tzId.toLatin1());
}

QJsonValue ssJsonSaveImpl(const QDate& value)
{
    return value.toString(Qt::DateFormat::ISODate);
}

void ssJsonLoadImpl(const QJsonValue& src, QDate& value)
{
    value = QDate::fromString(src.toString(), Qt::DateFormat::ISODate);
}

QJsonValue ssJsonSaveImpl(const QTime& value)
{
    return value.toString(Qt::DateFormat::ISODateWithMs);
}

void ssJsonLoadImpl(const QJsonValue& src, QTime& value)
{
    value = QTime::fromString(src.toString(), Qt::DateFormat::ISODateWithMs);
}

QJsonValue ssJsonSaveImpl(const QDateTime& value)
{
    QJsonObject result;
    result["timeSpec"] = ssJsonSaveImpl(value.timeSpec());
    result["date"] = ssJsonSaveImpl(value.date());
    result["time"] = ssJsonSaveImpl(value.time());

    switch (value.timeSpec()) {
        case Qt::LocalTime:
        case Qt::UTC:
            break;

        case Qt::OffsetFromUTC:
            result["offset-from-UTC"] = ssJsonSaveImpl(value.offsetFromUtc());
            break;

        case Qt::TimeZone:
            result["timezone"] = ssJsonSaveImpl(value.timeZone());
            break;

        default:
            assert(false && "Unhandled timeSpec!");
    }

    return result;
}

void ssJsonLoadImpl(const QJsonValue& src, QDateTime& value)
{
    using OffsetFromUtc = decltype(value.offsetFromUtc());

    const auto obj = src.toObject();
    const auto timeSpec = ssJsonLoadImplRet<Qt::TimeSpec>(obj["timeSpec"]);
    const auto date = ssJsonLoadImplRet<QDate>(obj["date"]);
    const auto time = ssJsonLoadImplRet<QTime>(obj["time"]);

    switch (timeSpec) {
        case Qt::LocalTime:
        case Qt::UTC:
            value = QDateTime(date, time, timeSpec);
            break;

        case Qt::OffsetFromUTC: {
            const auto offsetFromUtc = ssJsonLoadImplRet<OffsetFromUtc>(obj["offset-from-UTC"]);
            value = QDateTime(date, time, timeSpec, offsetFromUtc);
            break;
        }

        case Qt::TimeZone: {
            const auto timeZone = ssJsonLoadImplRet<QTimeZone>(obj["timezone"]);
            value = QDateTime(date, time, timeZone);
            break;
        }

        default:
            assert(false && "Unhandled timeSpec!");
    }
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

} // namespace SuitableStruct
