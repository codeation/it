# it
Impress terminal application, part of cross-platform GUI Library for Go. See https://github.com/codeation/impress

## Alpha Version

Notes:

- The project tested on Debian 11.1 and macOS Big Sur (11.5)
- The library may contain bugs

## Building (Linux)

Currently, the application uses [GTK+ 3](https://www.gtk.org)
for rendering, event collecting, etc. You should install `libgtk+-3.0` and packages that depend on GTK.

On Debian/ Ubuntu you can run:

```
sudo apt-get install libgtk-3-dev
```

Also [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) must be installed.

To build binary, download the sources and use the following command in project directory:

```
make
```

## Building (macOS)

First install [Homebrew](https://brew.sh/) if you don't have installed.

To install [GTK+ 3](https://www.gtk.org) run:

```
brew install gtk+3
```

To build binary, download the sources and use the following command in project directory:

```
make
```

## Download

You can download the compiled binary file on the ["releases"](https://github.com/codeation/it/releases) page.
Please, check and verify the sha256 sum for downloaded files.

## Issues

Feel free to open [issue](https://github.com/codeation/impress/issues)
