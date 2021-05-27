#!/bin/sh -e

rm -rf .git .travis common libs .gitattributes .gitignore .travis.yml ICON0.PNG LICENSE Makefile.psp README.md
mkdir -p PSP/GAME/CMFileManager
mv app/EBOOT.PBP PSP/GAME/CMFileManager/APP.PBP && mv launcher/EBOOT.PBP PSP/GAME/CMFileManager/EBOOT.PBP
mv audio_driver/audio_driver.prx PSP/GAME/CMFileManager/audio_driver.prx
mv display_driver/display_driver.prx PSP/GAME/CMFileManager/display_driver.prx
mv fs_driver/fs_driver.prx PSP/GAME/CMFileManager/fs_driver.prx
mv impose_driver/impose_driver.prx PSP/GAME/CMFileManager/impose_driver.prx
mv input_driver/input_driver.prx PSP/GAME/CMFileManager/input_driver.prx
zip -r CMFileManager-PSP.zip PSP/
rm -rf app audio_driver display_driver fs_driver impose_driver, input_driver launcher PSP
