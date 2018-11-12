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

rm -rf .git .travis common include libs opt src .gitattributes .gitignore .travis.yml CMFileManager.elf CMFileManager.prx ICON0.PNG LICENSE Makefile PARAM.SFO README.md sdk.lzma
