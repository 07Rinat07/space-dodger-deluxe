#!/usr/bin/env bash
set -euo pipefail

echo "Space Dodger Deluxe mobile environment check"
echo

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOCAL_CMAKE="$ROOT_DIR/.dev-tools/cmake/bin/cmake"
LOCAL_GRADLE="$ROOT_DIR/.dev-tools/gradle/gradle-8.9/bin/gradle"
LOCAL_GRADLE_HOME="$ROOT_DIR/.dev-tools/gradle-home"
LOCAL_ADB="$ROOT_DIR/.android-sdk/platform-tools/adb"
LOCAL_ANDROID_HOME="$ROOT_DIR/.android-sdk"
LOCAL_ANDROID_NDK="$ROOT_DIR/.android-sdk/ndk/26.3.11579264"

check_command() {
    local name="$1"
    local fallback="${2:-}"
    local version=""
    if command -v "$name" >/dev/null 2>&1; then
        version="$("$name" --version 2>/dev/null | awk 'NF { print; exit }' || "$name" version 2>/dev/null | awk 'NF { print; exit }' || true)"
        echo "[ok] $name: $version"
    elif [[ -n "$fallback" && -x "$fallback" ]]; then
        if [[ "$name" == "gradle" ]]; then
            version="$(GRADLE_USER_HOME="$LOCAL_GRADLE_HOME" "$fallback" --version 2>/dev/null | awk '/^Gradle / { print; exit }' || true)"
        else
            version="$("$fallback" --version 2>/dev/null | awk 'NF { print; exit }' || "$fallback" version 2>/dev/null | awk 'NF { print; exit }' || true)"
        fi
        echo "[ok] $name: $version ($fallback)"
    else
        echo "[missing] $name"
    fi
}

check_command cmake "$LOCAL_CMAKE"
check_command gradle "$LOCAL_GRADLE"
check_command adb "$LOCAL_ADB"
check_command xcodebuild

echo
if [[ -n "${ANDROID_HOME:-}" ]]; then
    echo "[ok] ANDROID_HOME=$ANDROID_HOME"
elif [[ -d "$LOCAL_ANDROID_HOME" ]]; then
    echo "[ok] ANDROID_HOME=$LOCAL_ANDROID_HOME (local)"
else
    echo "[missing] ANDROID_HOME"
fi

if [[ -n "${ANDROID_NDK_HOME:-}" ]]; then
    echo "[ok] ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
elif [[ -d "$LOCAL_ANDROID_NDK" ]]; then
    echo "[ok] ANDROID_NDK_HOME=$LOCAL_ANDROID_NDK (local)"
else
    echo "[missing] ANDROID_NDK_HOME"
fi

echo
echo "Android build:"
echo "  cd platform/android"
echo "  cp -R ../../assets app/src/main/assets/"
echo "  ANDROID_HOME=$LOCAL_ANDROID_HOME ANDROID_NDK_HOME=$LOCAL_ANDROID_NDK \\"
echo "  GRADLE_USER_HOME=$LOCAL_GRADLE_HOME \\"
echo "  $LOCAL_GRADLE assembleDebug -PRAYLIB_ANDROID_SOURCE=$ROOT_DIR/build/_deps/raylib-src"
echo
echo "iOS build:"
echo "  cd platform/ios"
echo "  ./build_ios.sh"
