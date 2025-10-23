#!/usr/bin/env bash
set -euo pipefail

APP_NAME="${APP_NAME:-MyQtApp}"
APP_ICON="${APP_ICON:-packaging/appicon.png}"

ROOT_DIR="$(pwd)"
BUILD_DIR="$ROOT_DIR/build-appimage"
APPDIR="$BUILD_DIR/AppDir"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Detect build system
if [[ -f "$ROOT_DIR/CMakeLists.txt" ]]; then
  BUILD_SYS="cmake"
elif ls "$ROOT_DIR"/*.pro >/dev/null 2>&1; then
  BUILD_SYS="qmake"
  PRO_FILE="$(ls "$ROOT_DIR"/*.pro | head -n1)"
else
  echo "Could not find CMakeLists.txt or a .pro file in project root."
  exit 1
fi
echo "==> Build system detected: $BUILD_SYS"

# Build
if [[ "$BUILD_SYS" == "cmake" ]]; then
  cmake -S "$ROOT_DIR" -B . -DCMAKE_BUILD_TYPE=Release
  cmake --build . --config Release -j"$(nproc)"
  BIN_PATH="$(find . -type f -perm -111 -name "$APP_NAME" | head -n1 || true)"
else
  qmake "$PRO_FILE" CONFIG+=release
  make -j"$(nproc)"
  BIN_PATH="$(find . -maxdepth 3 -type f -perm -111 -name "$APP_NAME" | head -n1 || true)"
fi

if [[ -z "${BIN_PATH:-}" || ! -x "$BIN_PATH" ]]; then
  echo "ERROR: Built executable '$APP_NAME' not found."
  echo "Tip: set APP_NAME=YourBinaryName when running this script."
  exit 1
fi
echo "==> Built binary at: $BIN_PATH"

# Prepare AppDir
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/256x256/apps"
cp "$BIN_PATH" "$APPDIR/usr/bin/$APP_NAME"

cat > "$APPDIR/usr/share/applications/$APP_NAME.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
Exec=$APP_NAME
Icon=$APP_NAME
Categories=Utility;
EOF

if [[ -f "$ROOT_DIR/$APP_ICON" ]]; then
  cp "$ROOT_DIR/$APP_ICON" "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"
fi

# Help Qt plugin find QML (harmless if you don't use QML)
export QML_SOURCES_PATHS="$ROOT_DIR"
# Force the plugin to use Qt6 qmake explicitly (belt & suspenders)
export QMAKE=/usr/bin/qmake6

# Bundle with linuxdeploy + Qt plugin (no-FUSE mode already enabled in the image)
 /usr/local/bin/linuxdeploy --appdir "$APPDIR" \
  --executable "$APPDIR/usr/bin/$APP_NAME" \
  --desktop-file "$APPDIR/usr/share/applications/$APP_NAME.desktop" \
  ${APP_ICON:+ --icon-file "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"} \
  --output appimage \
  --plugin qt

echo
echo "==> AppImage(s) created:"
ls -lh "$BUILD_DIR"/*.AppImage || true
echo "Portable AppDir is at: $APPDIR"
