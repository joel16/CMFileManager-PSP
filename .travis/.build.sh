#!/bin/sh -e

if [ "$BUILD_PSP" = "YES" ]; then
	echo PSPDEV = $PSPDEV
	echo psp-config = `psp-config --psp-prefix`
	git clone https://github.com/dogo/oslibmodv2.git
	cd oslibmodv2/
	make clean && make && make install install
	cd ../
	rm -rf oslibmodv2/
	make -j 4
fi

rm -rf .git common include libs src .gitattributes .gitignore CMFileManager.elf CMFileManager.prx ICON0.png LICENSE Makefile PARAM.SFO
