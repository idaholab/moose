#!/bin/bash

SDK_VERSION="14.5"
XCODE_VERSION="15.4"
XCODE_SHA_SUM="82d3d61804ff3f4c7c82085e91dc701037ddaa770e542848b2477e22f4e8aa7a"

set -eo pipefail

if [ "$(uname)" != "Darwin" ]; then
    echo "ERROR: This script should not be run on a system that is not a Mac." >&2
    exit 1
fi

SDK_DIR="/Users/$(whoami)/sdks"
SDK_DEST="${SDK_DIR}/MacOSX${SDK_VERSION}.sdk"
if [ -e "$SDK_DEST" ]; then
    echo "ERROR: SDK ${SDK_DEST} already exists!" >&2
    echo "" >&2
    echo "If you want to continue, delete it and run again." >&2
    exit 1
fi

XCODE_XIP="/Users/$(whoami)/Downloads/Xcode_${XCODE_VERSION}.xip"
if [ ! -f "$XCODE_XIP" ]; then
    echo "ERROR: You must first download Xcode ${XCODE_VERSION} to use this script!" >&2
    echo "" >&2
    echo "Go to https://developer.apple.com/download/all in a web browser." >&2
    echo "This will require you to sign in."
    echo ""
    echo "Once signed in, within the same web browser open" >&2
    echo "https://download.developer.apple.com/Developer_Tools/Xcode_${XCODE_VERSION}/Xcode_${XCODE_VERSION}.xip"
    echo "and download it into your Downloads folder." >&2
    echo "" >&2
    echo "Once Xcode is downloaded to ${XCODE_XIP}," >&2
    echo "run this script again. You do not need to install Xcode!" >&2
    exit 1
fi

echo "Extracting SDK from ${XCODE_XIP} to ${SDK_DEST}."
echo ""
echo "This may take a few minutes!"
echo ""

echo "Verifying download..."
RESULT="$(shasum -a 256 "$XCODE_XIP" | awk '{print $1}')"
if [ "$RESULT" != "$XCODE_SHA_SUM" ]; then
    echo "ERROR: Xcode file verification failed" >&2
    exit 1
fi

WORKDIR="$(mktemp -d /tmp/extract_mac_sdk.XXXXXX)"
function cleanup() {
    rm -rf "$WORKDIR" || true
}
trap cleanup EXIT SIGINT
cd "$WORKDIR"

echo "Expanding Xcode..."
xip --expand "$XCODE_XIP"

if ! [ -e "$SDK_DIR" ]; then
    echo "Making directory ${SDK_DIR}..."
    mkdir "$SDK_DIR"
fi

echo "Extracting SDK..."
SDK_SRC="${WORKDIR}/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK_VERSION}.sdk"
ditto "$SDK_SRC" "$SDK_DEST"

echo "Cleaning up..."
rm -rf "$WORKDIR"

echo ""
echo "Extracted SDK to ${SDK_DEST}."
echo "You may delete ${XCODE_XIP}."
