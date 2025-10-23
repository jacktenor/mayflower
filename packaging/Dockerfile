FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build essentials
    build-essential git ca-certificates curl file patchelf cmake make \
    # Qt 6 development (Widgets/Gui/Tools)
    qt6-base-dev qt6-base-dev-tools qmake6 \
    # QML (harmless if unused)
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtqml qml6-module-qtquick-layouts \
    # X11/xcb deps so linuxdeploy can detect/bundle xcb correctly
    libx11-dev libx11-xcb-dev libxcb1-dev libxcb-util-dev \
    libxcb-render0-dev libxcb-image0-dev libxcb-icccm4-dev \
    libxcb-keysyms1-dev libxcb-randr0-dev libxcb-shm0-dev \
    libxcb-xfixes0-dev libxcb-shape0-dev libxcb-sync-dev \
    libxcb-xinerama0-dev libxkbcommon-dev libxkbcommon-x11-dev \
    libfreetype6-dev libfontconfig1-dev zlib1g-dev \
    # OpenGL (harmless if unused)
    mesa-common-dev libglu1-mesa-dev \
    desktop-file-utils \
    && rm -rf /var/lib/apt/lists/*

# linuxdeploy + qt plugin
RUN curl -L -o /usr/local/bin/linuxdeploy \
      https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage \
 && chmod +x /usr/local/bin/linuxdeploy \
 && curl -L -o /usr/local/bin/linuxdeploy-plugin-qt \
      https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage \
 && chmod +x /usr/local/bin/linuxdeploy-plugin-qt

# Make all AppImages (linuxdeploy + plugin + appimagetool) self-extract (no FUSE)
ENV APPIMAGE_EXTRACT_AND_RUN=1

# Point the Qt plugin at the *Qt6* qmake explicitly
ENV QMAKE=/usr/bin/qmake6

WORKDIR /src

