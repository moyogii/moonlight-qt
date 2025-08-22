# Moonlight Mac

## Disclaimer
This fork was created to learn more about PC streaming and the MacOS operating system to optimize it for my specific setup. Due to this it has a bunch of experimental/in-progress features added that may or may not work for you. If you're looking for a stable experience, you should use the [official Moonlight client](https://github.com/moonlight-stream/moonlight-qt).

## About

[Moonlight](https://moonlight-stream.org) is an open source PC client for NVIDIA GameStream and [Sunshine](https://github.com/LizardByte/Sunshine).

Moonlight also has mobile versions for [Android](https://github.com/moonlight-stream/moonlight-android) and [iOS](https://github.com/moonlight-stream/moonlight-ios).

You can follow development on our [Discord server](https://moonlight-stream.org/discord) and help translate Moonlight into your language on [Weblate](https://hosted.weblate.org/projects/moonlight/moonlight-qt/).

 [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/glj5cxqwy2w3bglv/branch/master?svg=true)](https://ci.appveyor.com/project/cgutman/moonlight-qt/branch/master)
 [![Downloads](https://img.shields.io/github/downloads/moonlight-stream/moonlight-qt/total)](https://github.com/moonlight-stream/moonlight-qt/releases)
 [![Translation Status](https://hosted.weblate.org/widgets/moonlight/-/moonlight-qt/svg-badge.svg)](https://hosted.weblate.org/projects/moonlight/moonlight-qt/)

## Features
 - Hardware accelerated video decoding on Windows, Mac, and Linux
 - H.264, HEVC, and AV1 codec support (AV1 requires Sunshine and a supported host GPU)
 - YUV 4:4:4 support (Sunshine only)
 - HDR streaming support
 - 7.1 surround sound audio support
 - 10-point multitouch support (Sunshine only)
 - Gamepad support with force feedback and motion controls for up to 16 players
 - Support for both pointer capture (for games) and direct mouse control (for remote desktop)
 - Support for passing system-wide keyboard shortcuts like Alt+Tab to the host
 - Automatic enabling and disabling of AWDL (Apple Wireless Direct Link)
 - Game Mode support
 - AV1 Hardware Decoding (Apple Silicon M3 and above)
 
## Downloads
- [macOS](https://github.com/moyogii/moonlight-qt/releases)

#### Special Thanks

[![Hosted By: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

Hosting for Moonlight's Debian and L4T package repositories is graciously provided for free by [Cloudsmith](https://cloudsmith.com).

## Building

### macOS Build Requirements
* Qt 6.7 SDK or later (earlier versions may work but are not officially supported)
* Xcode 14 or later (earlier versions may work but are not officially supported)
* [create-dmg](https://github.com/sindresorhus/create-dmg) (only if building DMGs for use on non-development Macs)
* Jellyfin-FFmpeg 7.1+ (required - see setup instructions below)

## Jellyfin-FFmpeg Setup

**Important**: This project requires Jellyfin-FFmpeg instead of standard FFmpeg for enhanced codec support and streaming optimizations.

### Quick Setup for macOS
```bash
# Install dependencies
brew install pkg-config qt@6 create-dmg

# Set up jellyfin-ffmpeg development environment
mkdir -p $HOME/jellyfin/dev/{include,lib/pkgconfig}

# Clone jellyfin-ffmpeg source for headers
cd $HOME/jellyfin
git clone --depth 1 --branch v7.1.1-7 https://github.com/jellyfin/jellyfin-ffmpeg.git source

# Copy FFmpeg headers to development directory
cp -r source/libav* dev/include/

# Create pkg-config file
cat > dev/lib/pkgconfig/jellyfin-ffmpeg.pc << 'EOF'
prefix=$HOME/jellyfin/dev
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: jellyfin-ffmpeg
Description: Jellyfin FFmpeg libraries
Version: 7.1.1-7
Requires: 
Libs: -L${libdir} -lavcodec -lavformat -lavutil -lswscale -lavfilter -lavdevice -lswresample
Cflags: -I${includedir}
EOF

# Set environment for builds
export PKG_CONFIG_PATH="$HOME/jellyfin/dev/lib/pkgconfig:$PKG_CONFIG_PATH"
```

### Build Setup Steps
1. **Set up Jellyfin-FFmpeg** (see instructions above)
2. Install the latest Qt SDK (and optionally, the Qt Creator IDE) from https://www.qt.io/download
    * You can install Qt via Homebrew on macOS: `brew install qt@6`
    * You may also use your Linux distro's package manager for the Qt SDK as long as the packages are Qt 5.9 or later.
    * This step is not required for building on Steam Link, because the Steam Link SDK includes Qt 5.14.
3. Run `git submodule update --init --recursive` from within `moonlight-qt/`
4. Open the project in Qt Creator or build from qmake on the command line.
    * To build a binary for use on non-development machines, use the scripts in the `scripts` folder.
        * For Windows builds, use `scripts\build-arch.bat` and `scripts\generate-bundle.bat`. Execute these scripts from the root of the repository within a Qt command prompt. Ensure  7-Zip binary directory is on your `%PATH%`.
        * For macOS builds, the GitHub Actions workflow automatically creates DMG files. For manual DMG creation, use `create-dmg` directly.
        * For Steam Link builds, run `scripts/build-steamlink-app.sh` from the root of the repository.
    * To build from the command line for development use on macOS or Linux:
        ```bash
        # Ensure jellyfin-ffmpeg is in PKG_CONFIG_PATH (if using the macOS setup)
        export PKG_CONFIG_PATH="$HOME/jellyfin/dev/lib/pkgconfig:$PKG_CONFIG_PATH"
        
        # Configure and build
        qmake6 moonlight-qt.pro
        make debug  # or 'make release'
        ```
    * **Validation Script**: Use `./scripts/validate-build.sh` to verify your build environment is correctly configured
    * **GitHub Actions**: The project includes automated macOS builds that create distributable DMG files:
        * Builds are triggered on pushes to main/master branches and pull requests
        * Release builds are automatically attached to GitHub releases  
        * DMG artifacts are available for download from the Actions tab
    * To create an embedded build for a single-purpose device, use `qmake6 "CONFIG+=embedded" moonlight-qt.pro` and build normally.
        * This build will lack windowed mode, Discord/Help links, and other features that don't make sense on an embedded device.
        * For platforms with poor GPU performance, add `"CONFIG+=gpuslow"` to prefer direct KMSDRM rendering over GL/Vulkan renderers. Direct KMSDRM rendering can use dedicated YUV/RGB conversion and scaling hardware rather than slower GPU shaders for these operations.

## Contribute
1. Fork us
2. Write code
3. Send Pull Requests

Check out our [website](https://moonlight-stream.org) for project links and information.
