# it
Impress terminal application, part of cross-platform GUI Library for Go. See https://github.com/codeation/impress

## Proof of Concept Version

Notes:

- This project is still in the early stages of development and is not yet in a usable state.
- The project tested on Ubuntu 21.04 and MacOS Big Sur (11.5)

## Installation (Linux)

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

## Installation (macOS)

First install [Homebrew](https://brew.sh/) if you don't have installed.

To install [GTK+ 3](https://www.gtk.org) run:

```
brew install gtk+3
```

To build binary, download the sources and use the following command:

```
make
```

## Download

You can download the compiled binary file on the ["releases"](https://github.com/codeation/it/releases) page.
Please, check and verify the sha256 sum for downloaded files.
