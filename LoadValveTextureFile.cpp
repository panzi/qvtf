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

#include "LoadValveTextureFile.h"

#include <VTFLib.h>
#include <QImage>
#include <QVariant>

Q_EXPORT_PLUGIN2(LoadValveTextureFile, LoadValveTextureFilePlugin)

QImageIOPlugin::Capabilities LoadValveTextureFilePlugin::capabilities(QIODevice *device, const QByteArray &format) const {
    if (format.isNull() && !device) {
        return 0;
    }

    if (!format.isNull() && format.toLower() != "vtf") {
        return 0;
    }

    if (device && !ValveTextureFileHandler::canRead(device)) {
        return 0;
    }

    return CanRead;
}

QStringList LoadValveTextureFilePlugin::keys() const {
    return QStringList() << "vtf";
}

QImageIOHandler* LoadValveTextureFilePlugin::create(QIODevice *device, const QByteArray &format) const {
    if (format.isNull() || format.toLower() == "vtf") {
        ValveTextureFileHandler* handler = new ValveTextureFileHandler();
        handler->setDevice(device);
        handler->setFormat("vtf");
        return handler;
    }
    return 0;
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

QVariant ValveTextureFileHandler::option(ImageOption option) const {
    switch (option) {
    case Animation: return true;
    case Size:
        if (state == Read) {
            return QSize(vtf.GetWidth(), vtf.GetHeight());
        }
    case ImageFormat: // TODO
    default:
        return QVariant();
    }
}

bool ValveTextureFileHandler::supportsOption(ImageOption option) const {
    switch (option) {
    case Animation: return true;
    case Size: return true;
    case ImageFormat: // TODO
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

    switch (srcformat) {
    case IMAGE_FORMAT_RGB888:
        qformat   = QImage::Format_RGB888;
        dstformat = srcformat;
        break;

    case IMAGE_FORMAT_ABGR8888:
        qformat   = QImage::Format_ARGB32;
        dstformat = IMAGE_FORMAT_ARGB8888;
        break;

    default:
        if (VTFLib::CVTFFile::GetImageFormatInfo(srcformat).uiAlphaBitsPerPixel > 0) {
            qformat   = QImage::Format_ARGB32;
            dstformat = IMAGE_FORMAT_ARGB8888;
        }
        else {
            qformat   = QImage::Format_RGB888;
            dstformat = IMAGE_FORMAT_RGB888;
        }
    }

    if ((vlUInt)image->width() != width || (vlUInt)image->height() != height || image->format() != qformat) {
        *image = QImage(width, height, qformat);
        if (image->isNull()) {
            return false;
        }
    }

    const vlByte* frame = vtf.GetData(currentFrame, 0, 0, 0);

    if (!VTFLib::CVTFFile::Convert(frame, image->bits(), width, height, srcformat, dstformat)) {
        qWarning(VTFLib::LastError.Get());
        return false;
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
        qWarning(VTFLib::LastError.Get());
        state = Error;
        return false;
    }

    currentFrame = 0;
    state = Read;

    return true;
}
