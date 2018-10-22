# it
Impress terminal application, part of cross-platform GUI Library for Go. See https://github.com/codeation/impress

## Proof of Concept Version

Notes:

- This project is still in the early stages of development and is not yet in a usable state.
- The project tested on Debian 9 and MacOS 10.13.

## Installation

Currently, the application uses [GTK+ 3](https://www.gtk.org)
for rendering, event collecting, etc. You should install `libgtk+-3.0` and packages that depend on GTK.

On Debian/ Ubuntu you can run:

```
sudo apt-get install libgtk-3-dev
```

Also [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) must be installed.

To build binary, download the sources and use the following command:

```
make
```

## Download

You can download the compiled binary file on the ["releases"](https://github.com/codeation/it/releases) page.
Please, check and verify the sha256 sum for downloaded files.
