QVTF
====

QImageIO plugin to load Valve Texture Files (read-only).
Using this you will be able to view VTF files in Qt programs like Gwenview.

In order to view thumbnails in KDE programs like Dolphin see [KIO Thumbnail
VTF Plugin](https://github.com/panzi/KIO-VTF-Thumb-Creator) and for a general
read-only integration into Gtk+ (e.g. view VTF images in Eye of GNOME) see
[pixbufloader-vtf](https://github.com/panzi/pixbufloader-vtf).

### Setup

	git clone https://github.com/panzi/qvtf.git
	mkdir qvtf/build
	cd qvtf/build
	cmake .. -DCMAKE_BUILD_TYPE=Release
	make
	sudo make install

### Dependencies

 * [VTFLib](https://github.com/panzi/VTFLib)

### License

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
