#include <SuitableStruct/Internals/DefaultTypes.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QByteArray>
#include <QString>
#include <QPoint>
#include <QJsonObject>
#include <QColor>
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

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

} // namespace SuitableStruct
