# impress terminal application

This is a part of cross-platform GUI Library for Go. See https://github.com/codeation/impress

## Building (Linux + docker)

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

## Building (Linux)

Currently, the application uses [GTK+ 3](https://www.gtk.org)
for rendering, event collecting, etc. You should install `libgtk+-3.0` and packages that depend on GTK.

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

## Building (macOS)

First install [Homebrew](https://brew.sh/) if you don't have installed.

To install [GTK+ 3](https://www.gtk.org) run:

```
brew install gtk+3
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
