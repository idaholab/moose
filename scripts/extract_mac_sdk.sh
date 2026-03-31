#!/bin/bash

SDK_VERSION="14.5"
XCODE_VERSION="15.4"
XCODE_SHA_SUM="82d3d61804ff3f4c7c82085e91dc701037ddaa770e542848b2477e22f4e8aa7a"

set -eo pipefail

if [ "$(uname)" != "Darwin" ]; then
    echo "ERROR: System is not a Mac" >&2
    exit 1
fi

XCODE_XIP="/Users/$(whoami)/Downloads/Xcode_${XCODE_VERSION}.xip"
if [ ! -f "$XCODE_XIP" ]; then
    echo "ERROR: You must first download Xcode ${XCODE_VERSION} to use this script." >&2
    echo "" >&2
    echo "First, sign-in within a web browser at https://developer.apple.com." >&2
    echo "" >&2
    echo "Once signed in, either:" >&2
    echo " - Go to https://developer.apple.com/download/all/, search for 'Xcode ${XCODE_VERSION}' and download it, or" >&2
    echo " - Open https://download.developer.apple.com/Developer_Tools/Xcode_${XCODE_VERSION}/Xcode_${XCODE_VERSION}.xip in the same web browser" >&2
    echo "" >&2
    echo "This script expects Xcode ${XCODE_VERSION} to be downloaded to:" >&2
    echo "$XCODE_XIP" >&2
    exit 1
fi

SDK_DIR="/Users/$(whoami)/sdks"
SDK_DEST="${SDK_DIR}/MacOSX${SDK_VERSION}.sdk"
if [ -e "$SDK_DEST" ]; then
    echo "SDK destination ${SDK_DEST} already exists; delete and run again" >&2
    exit 1
fi

echo "Extracting SDK from ${XCODE_XIP} to ${SDK_DEST}."
echo "This may take a few minutes."
echo ""

echo "Verifying Xcode download..."
RESULT="$(shasum -a 256 "$XCODE_XIP" | awk '{print $1}')" || exit $?
if [ "$RESULT" != "$XCODE_SHA_SUM" ]; then
    echo "ERROR: Xcode file verification failed" >&2
    exit 1
fi

WORKDIR="$(mktemp -d /tmp/xcode-sdk-extract.XXXXXX)"
function cleanup() {
    rm -rf "$WORKDIR"
}
trap cleanup EXIT SIGINT
cd "$WORKDIR" || exit $?

echo "Expanding Xcode..."
xip --expand "$XCODE_XIP"

if ! [ -e "$SDK_DIR" ]; then
    echo "Making directory ${SDK_DIR}..."
    mkdir "$SDK_DIR" || exit $?
fi
SDK_DEST="${SDK_DIR}/MacOSX${SDK_VERSION}.sdk"
XCODE_APP="${WORKDIR}/Xcode.app"
SDK_SRC="${XCODE_APP}/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK_VERSION}.sdk"

echo "Extracting SDK..."
ditto "$SDK_SRC" "$SDK_DEST"

echo "Cleaning up expanded Xcode..."
rm -rf "$XCODE_APP"

echo ""
echo "Successfully extracted SDK to ${SDK_DEST}."
echo "You may delete ${XCODE_XIP}."
