
Debian
====================
This directory contains files used to package libertyd/Liberty-Qt
for Debian-based Linux systems. If you compile libertyd/Liberty-Qt yourself, there are some useful files here.

## liberty: URI support ##


Liberty-Qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install Liberty-Qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your libertyqt binary to `/usr/bin`
and the `../../share/pixmaps/liberty128.png` to `/usr/share/pixmaps`

Liberty-Qt.protocol (KDE)

