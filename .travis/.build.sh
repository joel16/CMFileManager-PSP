#!/bin/sh -e

if [ "$BUILD_PSP" = "YES" ]; then
    echo PSPDEV = $PSPDEV
    echo psp-config = `psp-config --psp-prefix`
    make -j 4
fi
