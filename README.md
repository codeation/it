# impress terminal application

![build release](https://github.com/codeation/it/actions/workflows/it-build-release-actions.yml/badge.svg)

This is a part of cross-platform GUI Library for Go. See [project site](https://codeation.github.io/impress/)
for [impress terminal details](https://codeation.github.io/impress/it-driver.html).

## Building

Currently, the application uses [GTK](https://www.gtk.org/) for rendering, event collecting, etc. You should install GTK and packages that depend on GTK. Also `pkg-config` must be installed.

On **Debian/Ubuntu** you can run:

```
sudo apt-get install libgtk-4-dev
```

To build binary, download the sources:

```
git clone https://github.com/codeation/it.git
cd it/four
```

and use the following command in project directory:

```
make
```

See [impress terminal details](https://codeation.github.io/impress/it-driver.html) for other options.
