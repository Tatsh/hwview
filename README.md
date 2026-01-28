# Hardware Viewer

[![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black)](https://www.gentoo.org/)
[![macOS](https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=F0F0F0)](https://www.apple.com/macos)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white)](https://www.microsoft.com/en-us/windows)
[![CMake](https://img.shields.io/badge/cmake-black.svg?logo=cmake&logoColor=064F8C)](https://cmake.org/)
[![Prettier](https://img.shields.io/badge/Prettier-enabled-black?logo=prettier)](https://prettier.io/)
[![Qt 6.7+ supported](https://img.shields.io/badge/qt-6.7+-black.svg?logo=qt&logoColor=00fa6f)](https://doc.qt.io/)
[![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B)](https://isocpp.org)
[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/hwview)](https://github.com/Tatsh/hwview/tags)
[![License](https://img.shields.io/github/license/Tatsh/hwview)](https://github.com/Tatsh/hwview/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/hwview/v0.0.1/master)](https://github.com/Tatsh/hwview/compare/v0.0.1...master)
[![CodeQL](https://github.com/Tatsh/hwview/actions/workflows/codeql.yml/badge.svg)](https://github.com/Tatsh/hwview/actions/workflows/codeql.yml)
[![Tests](https://github.com/Tatsh/hwview/actions/workflows/tests.yml/badge.svg)](https://github.com/Tatsh/hwview/actions/workflows/tests.yml)
[![Coverage Status](https://coveralls.io/repos/github/Tatsh/hwview/badge.svg?branch=master)](https://coveralls.io/github/Tatsh/hwview?branch=master)
[![QA](https://github.com/Tatsh/hwview/actions/workflows/qa.yml/badge.svg)](https://github.com/Tatsh/hwview/actions/workflows/qa.yml)
[![GitHub Pages](https://github.com/Tatsh/hwview/actions/workflows/pages.yml/badge.svg)](https://tatsh.github.io/hwview/)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/hwview?logo=github&style=flat)](https://github.com/Tatsh/hwview/stargazers)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor=did%3Aplc%3Auq42idtvuccnmtl57nsucz72&query=%24.followersCount&style=social&logo=bluesky&label=Follow+%40Tatsh)](https://bsky.app/profile/Tatsh.bsky.social)
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Tatsh-black?logo=buymeacoffee)](https://buymeacoffee.com/Tatsh)
[![Libera.Chat](https://img.shields.io/badge/Libera.Chat-Tatsh-black?logo=liberadotchat)](irc://irc.libera.chat/Tatsh)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)
[![Patreon](https://img.shields.io/badge/Patreon-Tatsh2-F96854?logo=patreon)](https://www.patreon.com/Tatsh2)

<!-- [![Flathub link](https://flathub.org/api/badge?locale=en)]
(https://flathub.org/apps/sh.tat.hwview) -->

A cross-platform hardware viewer application inspired by Redmond. View hardware devices, drivers,
and system resources on Linux, macOS, and Windows.

## Features

### Multiple views

- _Devices by type_ — categorised view with ability to show drivers in use.
- _Devices by connection_ — hierarchical view showing the device tree.
- _Devices by driver_ — devices grouped by their kernel driver.
- _Drivers by type_ — drivers organised by device category.
- _Drivers by device_ — drivers with their associated devices.
- _Resources by type_ — flat view of system resources (IRQs, I/O ports, memory ranges, DMA
  channels).
- _Resources by connection_ — resources shown in hierarchy.

### Device properties

Detailed information for each device including:

- General device information (type, manufacturer, location)
- Driver details (provider, version, digital signer, files)
- Device-specific properties (hardware IDs, compatible IDs, etc.)
- System events related to the device
- Hardware resources (IRQ, memory ranges, I/O ports)

### Export and import

Save device information for later viewing or sharing.

### Headless mode

Command-line export without GUI (build-time option).

## Gallery

![Screenshot](https://raw.githubusercontent.com/Tatsh/hwview/master/screenshot1.png)

![Screenshot](https://raw.githubusercontent.com/Tatsh/hwview/master/screenshot2.png)

## Building

Required dependencies:

- CMake at build time
- [ECM](https://invent.kde.org/frameworks/extra-cmake-modules) at build time
- Qt 6.7 or later with Concurrency, Network, and Widgets components

In the cloned project:

```shell
mkdir build
cd build
cmake ..
make
```

On MacPorts, set the `CMAKE_PREFIX_PATH` variable to
`/opt/local/libexec/ffmpeg7;/opt/local/libexec/qt6`.

## Windows support

Support for Windows will be limited until Linux and macOS support are considered complete.
