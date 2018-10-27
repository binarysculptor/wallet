
Debian
====================
This directory contains files used to package libertyd/liberty-qt
for Debian-based Linux systems. If you compile libertyd/liberty-qt yourself, there are some useful files here.

## liberty: URI support ##


liberty-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install liberty-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your libertyqt binary to `/usr/bin`
and the `../../share/pixmaps/liberty128.png` to `/usr/share/pixmaps`

liberty-qt.protocol (KDE)
