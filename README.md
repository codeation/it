# impress terminal application

![build release](https://github.com/codeation/it/actions/workflows/it-build-release-actions.yml/badge.svg)

This is a part of cross-platform GUI Library for Go. See https://github.com/codeation/impress

The impress library uses a separate application to low-level drawing.

<img src="https://codeation.github.io/images/it_scheme.png" width="580" height="274" />

The main GUI application contains pure Go code. There is not any low-level library required to build the main application. The impress terminal is running in parallel. Impress terminal was written in C and used the GTK-3 library. Named pipes are used to communicate between the main application and impress terminal.

Impress terminal started and stopped by main application. By default, executable impress terminal binary launched from current directory. The environment variable `IMPRESS_TERMINAL_PATH` may be used to specify full pathname to impress terminal binary.

## Building (Linux)

Currently, the application uses [GTK+ 3](https://www.gtk.org) for rendering, event collecting, etc. You should install `libgtk+-3.0` and packages that depend on GTK.

On Debian/ Ubuntu you can run:

```
sudo apt-get install libgtk-3-dev
```

Also [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) must be installed.

To build binary, download the sources:

```
git clone https://github.com/codeation/it.git
cd it
```

and use the following command in project directory:

```
make
```

## Building (Debian 12 + docker)

To build binary, download the sources:

```
git clone https://github.com/codeation/it.git
cd it
```

and use the following command in project directory:

```
./build.sh
```

This command creates a container with libgtk-3-dev packages from the gcc compiler container. The build is done inside the docker container without installing additional packages on the host.

Once the build is complete, the docker images can be removed using the `docker image rm` command. You can remove containers named:

```
amd64/gcc or arm64v8/gcc
it-build/gcc
```

*On Debian 12, `libharfbuzz-gobject` should be installed. If you are getting `error while loading shared libraries: libharfbuzz-gobject.so.0: cannot open shared object file: No such file or directory`, run `sudo apt-get install libharfbuzz-gobject0`.*

## Building (macOS)

*The latest releases aren't tested on Apple machines. The earlier version tested on both Intel and Silicon platform and worked well. Please, open [issue](https://github.com/codeation/impress/issues), if some bugs have raised. MRs are welcome too.*

First install [Homebrew](https://brew.sh/) if you don't have installed.

To install [GTK+ 3](https://www.gtk.org) run:

```
brew install gtk+3
```

To install `pkg-config` run:

```
brew install pkg-config
```

To build binary, download the sources:

```
git clone https://github.com/codeation/it.git
cd it
```

and use the following command in project directory:

```
make
```

## Download

You can download the compiled binary `it` file on the ["releases"](https://github.com/codeation/it/releases) page.

Github Actions builds binaries since v0.2.4 to prevent any corruption. Make sure the release is built from a signed commit.

Please, check and verify the sha256 sum for downloaded files from previous releases.

The `codeation` GPG key (Id `A8109B6877BC845E`) is used to sign commits in the main branch.

## Issues

Feel free to open [issue](https://github.com/codeation/impress/issues)
