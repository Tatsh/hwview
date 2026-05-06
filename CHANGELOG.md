<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]

## [0.0.3] - 2026-05-06

### Added

- Flatpak manifest (`sh.tat.hwview.yml`) for Flathub publishing.
- Snap package definition using the `kde-neon-6` extension on `core24`.
- Windows ARM64 (`winarm64`) build and packaging support.
- Build matrix coverage for macOS arm64 and x86_64, MSYS2 mingw64, ucrt64, and
  clangarm64, and Linux x86_64 and aarch64.
- Build-provenance attestations on draft GitHub release uploads.
- Tests and coverage badges in the README.

### Changed

- Windows runtime dependencies are now installed for all toolchains, not only
  MinGW.
- CPack package file names now include the build type and architecture so
  Debug, Release, x64, and ARM artefacts no longer collide.

### Fixed

- A warning is logged instead of silently discarding errors from
  `DeviceMonitor::start()`.

## [0.0.2] - 2026-01-28

### Added

- Name mappings for Apple Silicon devices.

### Fixes

- Possibly fix signing issue on macOS.

## [0.0.1] - 2026-01-28

First version. Limited macOS and Windows support.

[unreleased]: https://github.com/Tatsh/hwview/compare/v0.0.3...HEAD
[0.0.3]: https://github.com/Tatsh/hwview/compare/v0.0.2...v0.0.3
[0.0.2]: https://github.com/Tatsh/hwview/compare/v0.0.1...v0.0.2
[0.0.1]: https://github.com/Tatsh/hwview/releases/tag/v0.0.1
