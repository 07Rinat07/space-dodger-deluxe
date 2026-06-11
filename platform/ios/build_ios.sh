#!/usr/bin/env bash
set -euo pipefail

cmake -S "$(dirname "$0")" -B "$(dirname "$0")/build-ios" \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO

cmake --build "$(dirname "$0")/build-ios" --config Release
