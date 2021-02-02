/*    LoadValveTextureFile - load Valve Texture Format files in Qt4 applications
 *    Copyright (C) 2014  Mathias Panzenböck
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ValveTextureFile.h"

#include <VTFLib.h>
#include <QImage>
#include <QVariant>


#if QT_VERSION < 0x050000
Q_EXPORT_STATIC_PLUGIN(ValveTextureFilePlugin)
Q_EXPORT_PLUGIN2(ValveTextureFile, ValveTextureFilePlugin)
#endif // QT_VERSION < 0x050000

QImageIOPlugin::Capabilities ValveTextureFilePlugin::capabilities(QIODevice *device, const QByteArray &format) const {
    if (format.isNull() && !device) {
        return QImageIOPlugin::Capabilities();
    }

    if (!format.isNull() && format.toLower() != "vtf") {
        return QImageIOPlugin::Capabilities();
    }

    if (device && !ValveTextureFileHandler::canRead(device)) {
        return QImageIOPlugin::Capabilities();
    }

    return CanRead;
}

#if QT_VERSION < 0x050000
QStringList ValveTextureFilePlugin::keys() const {
    return QStringList() << "vtf";
}
#endif

QImageIOHandler* ValveTextureFilePlugin::create(QIODevice *device, const QByteArray &format) const {
    if (format.isNull() || format.toLower() == "vtf") {
        ValveTextureFileHandler* handler = new ValveTextureFileHandler();
        handler->setDevice(device);
        handler->setFormat("vtf");
        return handler;
    }
    return nullptr;
}

bool ValveTextureFileHandler::canRead(QIODevice *device) {
    if (!device) {
        qWarning("ValveTextureFileHandler::canRead() called with 0 pointer");
        return false;
    }

    QByteArray fourcc = device->peek(4);
    return fourcc.size() == 4 && memcmp(fourcc.data(), "VTF", 4) == 0;
}

ValveTextureFileHandler::ValveTextureFileHandler() : state(Ready), currentFrame(-1), vtf() {}

ValveTextureFileHandler::~ValveTextureFileHandler() {}

bool ValveTextureFileHandler::canRead() const {
    switch (state) {
    case Read:  return (vlUInt)currentFrame < vtf.GetFrameCount();
    case Error: return false;
    default:    return canRead(device());
    }
}

int ValveTextureFileHandler::currentImageNumber() const {
    if (state != Read) return -1;
    return currentFrame;
}

QRect ValveTextureFileHandler::currentImageRect() const {
    if (state != Read) return QRect();
    return QRect(0, 0, vtf.GetWidth(), vtf.GetHeight());
}

int ValveTextureFileHandler::imageCount() const {
    if (state != Read) return -1;
    return vtf.GetFrameCount();
}

bool ValveTextureFileHandler::jumpToImage(int imageNumber) {
    if (imageNumber < 0 || !read()) return false;
    if ((vlUInt)imageNumber >= vtf.GetFrameCount()) return false;
    currentFrame = imageNumber;
    return true;
}

bool ValveTextureFileHandler::jumpToNextImage() {
    if (!read()) return false;
    if (currentFrame > 0 && (vlUInt)currentFrame >= vtf.GetFrameCount()) return false;
    ++ currentFrame;
    return true;
}

int ValveTextureFileHandler::loopCount() const {
    if (state != Read) return 0;
    if (vtf.GetFrameCount() > 1) return -1;
    return 0;
}

int ValveTextureFileHandler::nextImageDelay() const {
    return 250;
}

static QStringList vtfFlagNames(vlUInt flags) {
    QStringList sflags;
    if (flags & TEXTUREFLAGS_POINTSAMPLE) {
        sflags.append(QLatin1String("PointSample"));
    }
    if (flags & TEXTUREFLAGS_TRILINEAR) {
        sflags.append(QLatin1String("Trilinear"));
    }
    if (flags & TEXTUREFLAGS_CLAMPS) {
        sflags.append(QLatin1String("ClampS"));
    }
    if (flags & TEXTUREFLAGS_CLAMPT) {
        sflags.append(QLatin1String("ClampT"));
    }
    if (flags & TEXTUREFLAGS_ANISOTROPIC) {
        sflags.append(QLatin1String("Anisotropic"));
    }
    if (flags & TEXTUREFLAGS_HINT_DXT5) {
        sflags.append(QLatin1String("Hint DXT5"));
    }
    if (flags & TEXTUREFLAGS_SRGB) {
        sflags.append(QLatin1String("SRGB"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_NOCOMPRESS) {
        sflags.append(QLatin1String("Deprecated NoCompress"));
    }
    if (flags & TEXTUREFLAGS_NORMAL) {
        sflags.append(QLatin1String("Normal"));
    }
    if (flags & TEXTUREFLAGS_NOMIP) {
        sflags.append(QLatin1String("NoMip"));
    }
    if (flags & TEXTUREFLAGS_NOLOD) {
        sflags.append(QLatin1String("NoLOD"));
    }
    if (flags & TEXTUREFLAGS_MINMIP) {
        sflags.append(QLatin1String("MinMip"));
    }
    if (flags & TEXTUREFLAGS_PROCEDURAL) {
        sflags.append(QLatin1String("Procedural"));
    }
    if (flags & TEXTUREFLAGS_ONEBITALPHA) {
        sflags.append(QLatin1String("OneBitAlpha"));
    }
    if (flags & TEXTUREFLAGS_EIGHTBITALPHA) {
        sflags.append(QLatin1String("EightBitAlpha"));
    }
    if (flags & TEXTUREFLAGS_ENVMAP) {
        sflags.append(QLatin1String("EnvMap"));
    }
    if (flags & TEXTUREFLAGS_RENDERTARGET) {
        sflags.append(QLatin1String("RenderTarget"));
    }
    if (flags & TEXTUREFLAGS_DEPTHRENDERTARGET) {
        sflags.append(QLatin1String("DepthRenderTarget"));
    }
    if (flags & TEXTUREFLAGS_NODEBUGOVERRIDE) {
        sflags.append(QLatin1String("NoDebugOverride"));
    }
    if (flags & TEXTUREFLAGS_SINGLECOPY) {
        sflags.append(QLatin1String("SingleCopy"));
    }
    if (flags & TEXTUREFLAGS_UNUSED0) {
        sflags.append(QLatin1String("Unused0"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_ONEOVERMIPLEVELINALPHA) {
        sflags.append(QLatin1String("Deprecated OneOverMipLevelInAlpha"));
    }
    if (flags & TEXTUREFLAGS_UNUSED1) {
        sflags.append(QLatin1String("Unused1"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_PREMULTCOLORBYONEOVERMIPLEVEL) {
        sflags.append(QLatin1String("Deprecated PremultColorByOneOverMipLevel"));
    }
    if (flags & TEXTUREFLAGS_UNUSED2) {
        sflags.append(QLatin1String("Unused2"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_NORMALTODUDV) {
        sflags.append(QLatin1String("Deprecated NormalTODUDV"));
    }
    if (flags & TEXTUREFLAGS_UNUSED3) {
        sflags.append(QLatin1String("Unused3"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_ALPHATESTMIPGENERATION) {
        sflags.append(QLatin1String("Deprecated AlphaTestMipGeneration"));
    }
    if (flags & TEXTUREFLAGS_NODEPTHBUFFER) {
        sflags.append(QLatin1String("NoDepthBuffer"));
    }
    if (flags & TEXTUREFLAGS_UNUSED4) {
        sflags.append(QLatin1String("Unused4"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_NICEFILTERED) {
        sflags.append(QLatin1String("Deprecated NiceFiltered"));
    }
    if (flags & TEXTUREFLAGS_CLAMPU) {
        sflags.append(QLatin1String("ClampU"));
    }
    if (flags & TEXTUREFLAGS_VERTEXTEXTURE) {
        sflags.append(QLatin1String("VertexTexture"));
    }
    if (flags & TEXTUREFLAGS_SSBUMP) {
        sflags.append(QLatin1String("SSBump"));
    }
    if (flags & TEXTUREFLAGS_UNUSED5) {
        sflags.append(QLatin1String("Unused5"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_UNFILTERABLE_OK) {
        sflags.append(QLatin1String("Deprecated Unfilterable Ok"));
    }
    if (flags & TEXTUREFLAGS_BORDER) {
        sflags.append(QLatin1String("Border"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_SPECVAR_RED) {
        sflags.append(QLatin1String("Deprecated SpecVar Red"));
    }
    if (flags & TEXTUREFLAGS_DEPRECATED_SPECVAR_ALPHA) {
        sflags.append(QLatin1String("Deprecated SpeCVar Alpha"));
    }
    if (flags & TEXTUREFLAGS_LAST) {
        sflags.append(QLatin1String("Last"));
    }
    return sflags;
}

QVariant ValveTextureFileHandler::option(ImageOption option) const {
    switch (option) {
    case Animation: return true;
    case Size:
        if (state == Read) {
            return QSize(vtf.GetWidth(), vtf.GetHeight());
        }
        break;
    case ImageFormat:
        if (state == Read) {
            if (VTFLib::CVTFFile::GetImageFormatInfo(vtf.GetFormat()).uiAlphaBitsPerPixel > 0) {
                return QImage::Format_ARGB32;
            }
            else {
                return QImage::Format_RGB888;
            }
        }
        break;
    case Description:
        if (state == Read) {
            vlSingle rX = 0, rY = 0, rZ = 0;

            vtf.GetReflectivity(rX, rY, rZ);
            SVTFImageFormatInfo formatInfo = VTFLib::CVTFFile::GetImageFormatInfo(vtf.GetFormat());

            QString descr = QString::fromUtf8(
                "Version: %1.%2\n\n"
                "Format: %3\n\n"
                "Depth: %4\n\n"
                "Bumpmap Scale: %5\n\n"
                "Reflectivity: %6, %7, %8\n\n"
                "Faces: %9\n\n"
                "Mipmaps: %10\n\n"
                "Frames: %11\n\n"
                "Start Frame: %12\n\n"
                "Flags: %13\n\n"
                "Bits Per Pixel: %14\n\n"
                "Alpha Channel: %15\n\n"
                "Compressed: %16\n\n")
                    .arg(vtf.GetMajorVersion())
                    .arg(vtf.GetMinorVersion())
                    .arg(QString::fromUtf8(formatInfo.lpName))
                    .arg(vtf.GetDepth())
                    .arg(vtf.GetBumpmapScale())
                    .arg(rX).arg(rY).arg(rZ)
                    .arg(vtf.GetFaceCount())
                    .arg(vtf.GetMipmapCount())
                    .arg(vtf.GetFrameCount())
                    .arg(vtf.GetStartFrame())
                    .arg(vtfFlagNames(vtf.GetFlags()).join(QLatin1String(", ")))
                    .arg(formatInfo.uiBitsPerPixel)
                    .arg(QLatin1String(formatInfo.uiAlphaBitsPerPixel > 0 ? "True" : "False"))
                    .arg(QLatin1String(formatInfo.bIsCompressed ? "True" : "False"));

            if (vtf.GetHasThumbnail()) {
                SVTFImageFormatInfo thumbFormatInfo = VTFLib::CVTFFile::GetImageFormatInfo(vtf.GetThumbnailFormat());

                descr += QString::fromUtf8(
                    "Thumbnail Format: %1\n\n"
                    "Thumbnail Size: %2x%3\n\n"
                    "Thumbnail Bits Per Pixel: %4\n\n"
                    "Thumbnail Alpha Channel: %5\n\n"
                    "Thumbnail Compressed: %6\n\n")
                        .arg(QString::fromUtf8(thumbFormatInfo.lpName))
                        .arg(vtf.GetThumbnailWidth())
                        .arg(vtf.GetThumbnailHeight())
                        .arg(thumbFormatInfo.uiBitsPerPixel)
                        .arg(QLatin1String(thumbFormatInfo.uiAlphaBitsPerPixel > 0 ? "True" : "False"))
                        .arg(QLatin1String(thumbFormatInfo.bIsCompressed ? "True" : "False"));
            }

            return descr;
        }
        break;
    default:
        break;
    }

    return QVariant();
}

bool ValveTextureFileHandler::supportsOption(ImageOption option) const {
    switch (option) {
    case Animation:
    case Size:
    case ImageFormat:
    case Description:
        return true;
    default:
        return false;
    }
}

bool ValveTextureFileHandler::read(QImage *image) {
    if (!image) {
        qWarning("ValveTextureFileHandler::read() called with 0 pointer");
        return false;
    }

    if (!read() || (vlUInt)currentFrame >= vtf.GetFrameCount()) {
        return false;
    }

    vlUInt width  = vtf.GetWidth();
    vlUInt height = vtf.GetHeight();
    QImage::Format qformat;
    VTFImageFormat srcformat = vtf.GetFormat();
    VTFImageFormat dstformat;

    if (VTFLib::CVTFFile::GetImageFormatInfo(srcformat).uiAlphaBitsPerPixel > 0) {
        qformat   = QImage::Format_ARGB32;
        dstformat = IMAGE_FORMAT_RGBA8888;
    }
    else {
        qformat   = QImage::Format_RGB888;
        dstformat = IMAGE_FORMAT_RGB888;
    }

    const vlByte* frame = vtf.GetData(currentFrame, 0, 0, 0);

    if (!frame) {
        return false;
    }

    if ((vlUInt)image->width() != width || (vlUInt)image->height() != height || image->format() != qformat) {
        *image = QImage(width, height, qformat);
        if (image->isNull()) {
            return false;
        }
    }

    uchar *bits = image->bits();

    if (!VTFLib::CVTFFile::Convert(frame, bits, width, height, srcformat, dstformat)) {
        qDebug("%s", VTFLib::LastError.Get());
        return false;
    }

    if (dstformat == IMAGE_FORMAT_RGBA8888) {
        // For some reason the colors are swapped around this way.
        // I don't know if the error is in VTFLib, Qt or my usage of either.
        for (size_t i = 0, n = width * height * 4; i < n; i += 4) {
            uchar b = bits[i + 0];
            uchar r = bits[i + 2];

            bits[i + 0] = r;
            bits[i + 2] = b;
        }
    }

    if (currentFrame == 0) {
        // only load all the meta info on first frame
        vlSingle rX = 0, rY = 0, rZ = 0;

        vtf.GetReflectivity(rX, rY, rZ);
        SVTFImageFormatInfo formatInfo = VTFLib::CVTFFile::GetImageFormatInfo(vtf.GetFormat());

        image->setText(QLatin1String("Version"),QString::fromUtf8("%1.%2").arg(vtf.GetMajorVersion()).arg(vtf.GetMinorVersion()));
        image->setText(QLatin1String("Format"),QLatin1String(formatInfo.lpName));
        image->setText(QLatin1String("Depth"),QString::number(vtf.GetDepth()));
        image->setText(QLatin1String("Bumpmap Scale"),QString::number(vtf.GetBumpmapScale()));
        image->setText(QLatin1String("Reflectivity"),QString::fromUtf8("%1, %2, %3").arg(rX).arg(rY).arg(rZ));
        image->setText(QLatin1String("Faces"),QString::number(vtf.GetFaceCount()));
        image->setText(QLatin1String("Mipmaps"),QString::number(vtf.GetMipmapCount()));
        image->setText(QLatin1String("Frames"),QString::number(vtf.GetFrameCount()));
        image->setText(QLatin1String("Start Frame"),QString::number(vtf.GetStartFrame()));
        image->setText(QLatin1String("Flags"),vtfFlagNames(vtf.GetFlags()).join(QLatin1String(", ")));
        image->setText(QLatin1String("Bits Per Pixel"),QString::number(formatInfo.uiBitsPerPixel));
        image->setText(QLatin1String("Alpha Channel"),QLatin1String(formatInfo.uiAlphaBitsPerPixel > 0 ? "True" : "False"));
        image->setText(QLatin1String("Compressed"),QLatin1String(formatInfo.bIsCompressed ? "True" : "False"));

        if (vtf.GetHasThumbnail()) {
            SVTFImageFormatInfo thumbFormatInfo = VTFLib::CVTFFile::GetImageFormatInfo(vtf.GetThumbnailFormat());

            image->setText(QLatin1String("Thumbnail Format"),QLatin1String(thumbFormatInfo.lpName));
            image->setText(QLatin1String("Thumbnail Size"),QString::fromUtf8("%1x%2").arg(vtf.GetThumbnailWidth()).arg(vtf.GetThumbnailHeight()));
            image->setText(QLatin1String("Thumbnail Bits Per Pixel"),QString::number(thumbFormatInfo.uiBitsPerPixel));
            image->setText(QLatin1String("Thumbnail Alpha Channel"),QLatin1String(thumbFormatInfo.uiAlphaBitsPerPixel > 0 ? "True" : "False"));
            image->setText(QLatin1String("Thumbnail Compressed"),QLatin1String(thumbFormatInfo.bIsCompressed ? "True" : "False"));
        }
    }

    ++ currentFrame;

    return true;
}

bool ValveTextureFileHandler::read()  {
    switch (state) {
    case Read:  return true;
    case Error: return false;
    case Ready:
        break;
    }

    QByteArray data = device()->readAll();

    if (!vtf.Load((const vlVoid*)data.data(), (vlSize)data.size())) {
        qWarning("%s", VTFLib::LastError.Get());
        state = Error;
        return false;
    }

    currentFrame = 0;
    state = Read;

    return true;
}
