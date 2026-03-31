#!/bin/bash
set -eo pipefail

if [ "$(uname)" != "Darwin" ]; then
    echo "ERROR: This script should not be run on a system that is not a Mac." >&2
    exit 1
fi

# Determine Xcode and SDK version
#
# These requirements come from developer.apple.com/xcode/system-requirements,
# to which we don't yet support Xcode 26.4
#
# When updating/bumping these, you should also update
# conda/mpi/base_build.sh to use the newer versions
PRODUCT_VERSION="$(sw_vers -productVersion)"
IFS='.' read -r OS_MAJOR OS_MINOR _ _ <<< "$PRODUCT_VERSION"
UNAME="$(uname -m)"
XCODE_DOWNLOAD_SUFFIX=
if [ "$OS_MAJOR" -le 12 ]; then
    echo "$ERROR: Mac OS ${PRODUCT_VERESION} is not supported" >&2
    exit 1
fi
if [ "$OS_MAJOR" == "13" ] && [ "$OS_MINOR" -lt 5 ]; then
    XCODE_VERSION="14.3.1"
    XCODE_SHASUM="b5cc7bf37447c32a971b37d71c7da1af7abb45cee4b96fe126a1d3b0d2c260af"
    SDK_VERSION="13.3"
elif [ "$OS_MAJOR" == "13" ]; then
    XCODE_VERSION="15.2"
    SDK_VERSION="14.2"
elif [ "$OS_MAJOR" == "14" ] && [ "$OS_MAJOR" -lt 5 ]; then
    XCODE_VERSION="15.4"
    XCODE_SHASUM="82d3d61804ff3f4c7c82085e91dc701037ddaa770e542848b2477e22f4e8aa7a"
    SDK_VERSION="14.5"
elif [ "$OS_MAJOR" == "14" ] || { [ "$OS_MAJOR" == "15" ]; [ "$OS_MINOR" == "1" ]; }; then
    XCODE_VERSION="16.2"
    XCODE_SHASUM="0e367d06eb7c334ea143bada5e4422f56688aabff571bedf0d2ad9434b7290de"
    SDK_VERSION="15.2"
elif [ "$OS_MAJOR" == "15" ] && [ "$OS_MINOR" == "2" ]; then
    XCODE_VERSION="16.3"
    XCODE_SHASUM="c593177b73e45f31e1cf7ced131760d8aa8e1532f5bbf8ba11a4ded01da14fbb"
    SDK_VERSION="15.4"
elif [ "$OS_MAJOR" == "15" ]; then
    XCODE_VERSION="16.4"
    XCODE_SHASUM="2dbf65ba28fb85b34e72c14c529a42d5c3189ab0f11fb29fdebd5f4ee6c87900"
    SDK_VERSION="15.5"
elif [ "$OS_MAJOR" == "26" ]; then
    XCODE_VERSION="26.3"
    if [ "$UNAME" == "arm64" ]; then
        XCODE_DOWNLOAD_SUFFIX="_Apple_silicon"
        XCODE_SHASUM="f3a3a6394f03dd2b562bd0d78fbfedf31bed23c48ff5a881cadeb72b5552a1e9"
    else
        XCODE_DOWNLOAD_SUFFIX="_Universal"
        XCODE_SHASUM="cf87232e0419785170edcfa070b750f28808ec00b489ab540c08b7d197c79ae4"
    fi
    SDK_VERSION="26.2"
fi

# Destination where we're going to unload just the SDK from Xcode,
# which shouldn't exist
SDK_DIR="/Users/$(whoami)/sdks"
SDK_DEST="${SDK_DIR}/MacOSX${SDK_VERSION}.sdk"
if [ -e "$SDK_DEST" ]; then
    echo "ERROR: SDK ${SDK_DEST} already exists!" >&2
    echo "" >&2
    echo "If you want to continue, delete it and run again." >&2
    exit 1
fi

# Source where Xcode should be downloaded to, which should exist
XCODE_NAME="Xcode_${XCODE_VERSION}${XCODE_DOWNLOAD_SUFFIX}.xip"
XCODE_XIP="/Users/$(whoami)/Downloads/${XCODE_NAME}"
if [ ! -f "$XCODE_XIP" ]; then
    echo "ERROR: You must first download Xcode ${XCODE_VERSION} to use this script!" >&2
    echo "" >&2
    echo "1. Go to https://developer.apple.com/download/all/?q=Xcode%20${XCODE_VERSION}"
    echo "   in a web browser. This will require you to sign in." >&2
    echo "2. Expand 'View details' under the entry named 'Xcode ${XCODE_VERSION}'." >&2
    echo "3. Click on the link 'Xcode ${XCODE_VERSION}${XCODE_DOWNLOAD_SUFFIX//_/ }.xip', downloading it into" >&2
    echo "   your Downloads folder."  >&2
    echo "" >&2
    echo "Once Xcode is downloaded to '${XCODE_XIP}'," >&2
    echo "run this script again. You do not need to install Xcode!" >&2
    exit 1
fi

# We're startin'
echo "Extracting SDK from ${XCODE_XIP} to ${SDK_DEST}."
echo ""
echo "This may take a few minutes!"
echo ""

# Verify sha 256 sum for the download
echo "Verifying download..."
RESULT="$(shasum -a 256 "$XCODE_XIP" | awk '{print $1}')"
if [ "$RESULT" != "$XCODE_SHASUM" ]; then
    echo "ERROR: Xcode file verification failed!" >&2
    echo "Delete ${XCODE_XIP} and download Xcode again." >&2
    exit 1
fi

# Setup a work directory that we cleanup on exit
WORKDIR="$(mktemp -d "/Users/$(whoami)/Downloads/extract_mac_sdk_XXXXXX")"
function cleanup() {
    rm -rf "$WORKDIR" || true
}
trap cleanup EXIT SIGINT

# Expand Xcode into the work directory
echo "Expanding Xcode..."
cd "$WORKDIR"
xip --expand "$XCODE_XIP"

# Create ~/sdks if it doesn't exist
if ! [ -e "$SDK_DIR" ]; then
    echo "Making directory ${SDK_DIR}..."
    mkdir "$SDK_DIR"
fi

# Extract the SDK from the expanded Xcode into ~/sdks
echo "Extracting SDK..."
SDK_SRC="${WORKDIR}/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK_VERSION}.sdk"
ditto "$SDK_SRC" "$SDK_DEST"

# Remove the workdir with the expanded Xcode
echo "Cleaning up..."
rm -rf "$WORKDIR"

# We're done
echo ""
echo "Extracted SDK to ${SDK_DEST}."
echo "You may delete ${XCODE_XIP}."
