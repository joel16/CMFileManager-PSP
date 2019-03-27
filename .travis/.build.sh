#!/bin/sh -e

rm -rf .git .travis common libs .gitattributes .gitignore .travis.yml ICON0.PNG LICENSE Makefile README.md
mkdir -p PSP/GAME/CMFileManager
mv app/EBOOT.PBP PSP/GAME/CMFileManager/APP.PBP && mv launcher/EBOOT.PBP PSP/GAME/CMFileManager/EBOOT.PBP
mv audio_driver/audio_driver.prx PSP/GAME/CMFileManager/audio_driver.prx
zip -r CMFileManager-PSP.zip PSP/
rm -rf app audio_driver launcher PSP
