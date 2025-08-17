#!/bin/bash
set -e
if [ ! -f "moonlight-qt.pro" ]; then
    echo "Error: Must run from moonlight-qt root directory"
    exit 1
fi

echo "Checking Qt installation..."
if command -v qmake6 >/dev/null 2>&1; then
    echo "qmake6 found: $(which qmake6)"
    qmake6 --version
else
    echo "Error: qmake6 not found. Please install Qt 6.7+"
    exit 1
fi

echo "Checking pkg-config..."
if ! command -v pkg-config >/dev/null 2>&1; then
    echo "Error: pkg-config not found. Please install pkg-config"
    exit 1
fi

echo "Checking jellyfin-ffmpeg..."
if ! pkg-config --exists jellyfin-ffmpeg; then
    echo "Error: jellyfin-ffmpeg not found"
    echo "Make sure to set up jellyfin-ffmpeg as described in README.md"
    echo "and set PKG_CONFIG_PATH if using the macOS development setup"
    exit 1
fi

# Check submodules
echo "Checking submodules..."
if ! [ -d "moonlight-common-c/moonlight-common-c" ] && [ -d "qmdnsengine/qmdnsengine" ]; then
    echo "Error: Submodules not found. Run: git submodule update --init --recursive"
    exit 1
fi

echo "Testing qmake configuration..."
if ! qmake6 moonlight-qt.pro; then
    echo "Error: qmake configuration failed"
    exit 1
fi

echo "Testing build start..."
if ! make --dry-run >/dev/null 2>&1; then
    echo "Error: Build configuration has issues"
    exit 1
fi

echo ""
echo "Ready to build with:"
echo "   make debug    # for development"
echo "   make release  # for distribution"

if command -v create-dmg >/dev/null 2>&1; then
    echo "create-dmg is available for DMG creation"
else
    echo "create-dmg is not available. Install create-dmg: brew install create-dmg"
fi