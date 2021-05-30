# CMFileManager PSP [![Build Status](https://travis-ci.org/joel16/CMFileManager-PSP.svg?branch=master)](https://travis-ci.org/joel16/CMFileManager-PSP) ![Github latest downloads](https://img.shields.io/github/downloads/joel16/CMFileManager-PSP/total.svg)

CMFileManager PSP is a PSP application built using the unofficial PSPSDK and [glib2d (modded)](https://github.com/joel16/gLib2D). The user interface in this homebrew is heavily inspired and based on the LineageOS/CM file manager. The aim of this project is to provide a stable and up to date file manager that makes use of updated tools, whilst offering a modernized and user friendly interace. CMFileManager PSP is the **only** homebrew that properly uses the 'move' function correctly for moving files/folder from one place to another. Unlike other file managers for PSP which copies a file from one place to another and then deletes it, which can take a while depending on the file size.

**Features:**

- Copy files/folders.
- Move files/folders.
- Delete files/folders.
- Multi file handling functions for copy/move/delete.
- File properties. (File size, creation time, modification time, access times, permissions)
- Creating directories.
- Renaming files/folders.
- Image viewer (BMP, GIF - non animated, JPG, PCX, PNG, PGM, PPM and TGA).
- Auto USB mount (with ef0:/ and ms0:/ support for PSP Go).
- Dark theme mode.
- EBOOT (homebrew/game) launcher - Also allows you to rip icon0/pic1 from eboot if it exists.
- Browse other drives like flah0, flash1 and UMD (if not using PSP Go).
- Support for both ef0:/ (internal) and ms0:/ (external) drives on PSP Go.
- Screenshots in .BMP format. Use L + R to capture a screenshot.
- Audio Player 16-Bit short 41K - 48K Hz. (FLAC, IT, MOD, MP3, OGG, OPUS, S3M, WAV and XM)
- FTP connection


**FTP Mode Instructions:**
1. Press *START* to open settings menu and select `FTP connection`.
3. Select a network connection when the Network dialog pops up.
4. After it is connected successfully, a message will appear saying `FTP Connection established IP:Port`. Use an FTP client like WinSCP and enter the IP and port (1337) and make sure to use anonymous login.
6. Once an FTP connection is established from your server, you *may* not see any folders or files. If this is the case, you can manually enter the path of the device like `ip:port/ms0:` (A real example would look something like: `127.0.0.1:1337/ms0:`)


**Buttons:**

- Press *DPAD Up* to move the cursor up.
- Press *DPAD Right* to move the cursor down.
- Press *DPAD Left* to position the cursor to the start of the directory.
- Press *DPAD Right* to position the cursor to the end of the directory.
- Press *Start* to open settings.
- Press *Select* to open menubar.
- Press *Triangle* to bring up file options.
- Press *enter button (depending on your region)* to enter directory/open file.
- Press *cancel button (depending on your region)* to go back to previous directory/menu.
- Press *L + R* to caputre screenshot.
- Press *Home* to display exit dialog.
- Press *Start* on FTP mode or Audio player to disable/enable screen to save battery.


**Image Viewer controls:**
- L/R - Rotate anti-clockwise/clockwise
- Up/Down - Zoom in/out
- Select - Help menu
- Left/Right - Previous/Next image
- Analog stick - Position image after zoom
- Triangle - Flip vertically
- Square - Flip horizantally
