#!/usr/bin/env bash
# Build an installable Coral .deb with runtime library Depends.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
VERSION="${CORAL_VERSION:-1.2.12}"
REVISION="${CORAL_REVISION:-3}"
ARCH="$(dpkg --print-architecture)"
PKG_NAME="coral"
PKG_VER="${VERSION}-${REVISION}"
OUT_DIR="${CORAL_OUT_DIR:-$ROOT/dist}"
STAGE="$(mktemp -d /tmp/coral-deb.XXXXXX)"
cleanup() { rm -rf "$STAGE"; }
trap cleanup EXIT

echo "==> Building Coral Release"
make -C "$ROOT/coral/Builds/LinuxMakefile" -j"$(nproc)" CONFIG=Release

BIN_SRC="$ROOT/coral/Builds/LinuxMakefile/build/Coral"
if [[ ! -x "$BIN_SRC" ]]; then
  echo "Release binary missing: $BIN_SRC" >&2
  exit 1
fi

echo "==> Staging package tree"
PKG="$STAGE/${PKG_NAME}_${PKG_VER}_${ARCH}"
install -d "$PKG/DEBIAN"
install -d "$PKG/usr/bin"
install -d "$PKG/usr/share/applications"
install -d "$PKG/etc/xdg/autostart"
install -d "$PKG/usr/share/coral/Factsoft"
install -d "$PKG/usr/share/pixmaps"
install -d "$PKG/usr/share/doc/${PKG_NAME}"
install -d "$PKG/usr/share/icons/hicolor/scalable/apps"

install -m 0755 "$BIN_SRC" "$PKG/usr/bin/coral"
strip --strip-unneeded "$PKG/usr/bin/coral"

# Factory presets
cp -a "$ROOT/Factsoft/"*.fac "$PKG/usr/share/coral/Factsoft/"

# Brand mark
SVG="$ROOT/coral/Images/coral.svg"
if [[ ! -f "$SVG" ]]; then
  echo "Missing $SVG" >&2
  exit 1
fi
install -m 0644 "$SVG" "$PKG/usr/share/icons/hicolor/scalable/apps/coral.svg"

python3 - "$SVG" "$PKG" <<'PY'
import sys
from pathlib import Path
from PIL import Image, ImageDraw

svg_note, pkg = Path(sys.argv[1]), Path(sys.argv[2])
COLOR = (0x7E, 0xB8, 0xEA, 255)
BARS = [(22, 44, 16, 40, 8), (45, 18, 16, 92, 8), (68, 34, 16, 60, 8), (91, 50, 16, 28, 8)]

def make_icon(size: int) -> Image.Image:
    scale = 4
    canvas = size * scale
    img = Image.new("RGBA", (canvas, canvas), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    s = canvas / 128.0
    for x, y, w, h, r in BARS:
        box = [x * s, y * s, (x + w) * s, (y + h) * s]
        draw.rounded_rectangle(box, radius=max(1.0, r * s), fill=COLOR)
    return img.resize((size, size), Image.Resampling.LANCZOS)

for size in (16, 22, 24, 32, 48, 64, 128, 256, 512):
    dest = pkg / "usr/share/icons/hicolor" / f"{size}x{size}" / "apps"
    dest.mkdir(parents=True, exist_ok=True)
    make_icon(size).save(dest / "coral.png", "PNG")

make_icon(256).save(pkg / "usr/share/pixmaps/coral.png", "PNG")
PY

cat > "$PKG/usr/share/applications/coral.desktop" <<'EOF'
[Desktop Entry]
Name=Coral
GenericName=Audio Enhancer
Comment=Warm, clear system-wide audio enhancement
Exec=env CORAL_SHOW=1 /usr/bin/coral
TryExec=/usr/bin/coral
Icon=coral
Terminal=false
Type=Application
Categories=AudioVideo;Audio;AudioVideoEditing;
Keywords=audio;equalizer;dsp;sound;enhancer;
StartupWMClass=Coral
StartupNotify=false
X-GNOME-UsesNotifications=true
EOF

cat > "$PKG/etc/xdg/autostart/coral.desktop" <<'EOF'
[Desktop Entry]
Name=Coral
Comment=Start Coral audio enhancement in the system tray
Exec=/usr/bin/coral --run_minimized
TryExec=/usr/bin/coral
Icon=coral
Terminal=false
Type=Application
X-GNOME-Autostart-enabled=true
X-GNOME-UsesNotifications=true
EOF

install -m 0644 "$ROOT/LICENSE" "$PKG/usr/share/doc/${PKG_NAME}/copyright"
cat > "$PKG/usr/share/doc/${PKG_NAME}/README" <<EOF
Coral ${VERSION}

Install:
  sudo apt install ./${PKG_NAME}_${PKG_VER}_${ARCH}.deb

No extra setup. apt installs libraries, then Coral starts and routes
system audio. Close/minimize hides to the tray; it also starts on login.

The apt note about an unsandboxed local file is normal for a .deb in
your Downloads folder and does not mean install failed.
EOF

cat > "$PKG/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e

start_for_installing_user() {
  u="${SUDO_USER:-}"
  if [ -z "$u" ] || [ "$u" = "root" ]; then
    return 0
  fi
  uid="$(id -u "$u" 2>/dev/null)" || return 0
  runtime="/run/user/$uid"
  if [ ! -d "$runtime" ]; then
    return 0
  fi
  if pgrep -u "$uid" -x coral >/dev/null 2>&1; then
    return 0
  fi
  home="$(getent passwd "$u" | cut -d: -f6)"
  runuser -u "$u" -- env \
    HOME="$home" \
    DISPLAY="${DISPLAY:-:0}" \
    WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}" \
    XDG_RUNTIME_DIR="$runtime" \
    DBUS_SESSION_BUS_ADDRESS="unix:path=$runtime/bus" \
    CORAL_SHOW=1 \
    /usr/bin/coral >/dev/null 2>&1 &
}

if [ "$1" = configure ]; then
  if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor >/dev/null 2>&1 || true
  fi
  if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q /usr/share/applications >/dev/null 2>&1 || true
  fi
  start_for_installing_user || true
fi
exit 0
EOF

cat > "$PKG/DEBIAN/prerm" <<'EOF'
#!/bin/sh
set -e
if [ "$1" = remove ] || [ "$1" = upgrade ]; then
  pkill -x coral >/dev/null 2>&1 || true
fi
exit 0
EOF

cat > "$PKG/DEBIAN/postrm" <<'EOF'
#!/bin/sh
set -e
if [ "$1" = remove ] || [ "$1" = purge ]; then
  if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor >/dev/null 2>&1 || true
  fi
  if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q /usr/share/applications >/dev/null 2>&1 || true
  fi
fi
exit 0
EOF
chmod 0755 "$PKG/DEBIAN/postinst" "$PKG/DEBIAN/prerm" "$PKG/DEBIAN/postrm"

SIZE_KB="$(du -sk "$PKG" | awk '{print $1}')"

# Direct NEEDED libs plus dlopen tray + pactl + sound server.
# t64 alternatives keep the package installable on Ubuntu 22.04 and 24.04+.
DEPENDS="libc6, libstdc++6, libgcc-s1, libgtk-3-0 | libgtk-3-0t64, libgdk-pixbuf-2.0-0, libglib2.0-0 | libglib2.0-0t64, libfreetype6, libpulse0, libgl1, libx11-6, libayatana-appindicator3-1 | libappindicator3-1, pulseaudio-utils, pulseaudio | pipewire-pulse"

cat > "$PKG/DEBIAN/control" <<EOF
Package: ${PKG_NAME}
Version: ${PKG_VER}
Section: sound
Priority: optional
Architecture: ${ARCH}
Maintainer: Coral Linux Port <coral-linux@local>
Installed-Size: ${SIZE_KB}
Depends: ${DEPENDS}
Recommends: gnome-shell-extension-appindicator | xfce4-indicator-plugin
Homepage: https://github.com/NeuroDev204/coral
Description: Coral - system-wide audio enhancement
 Coral is a Linux audio enhancer. It creates a virtual sink, processes
 system audio (EQ, ambience, surround, bass, dynamic boost), and plays
 it back on your headphones or speakers.
 .
 Installing this package also installs the required GTK, PulseAudio /
 PipeWire, OpenGL, and AppIndicator libraries via apt.
EOF

(
  cd "$PKG"
  find usr etc -type f -print0 | sort -z | xargs -0 md5sum > DEBIAN/md5sums
)

echo "==> Building .deb"
mkdir -p "$OUT_DIR"
DEB="$OUT_DIR/${PKG_NAME}_${PKG_VER}_${ARCH}.deb"
fakeroot dpkg-deb --root-owner-group --build "$PKG" "$DEB"
# also drop a copy next to the repo for easy sharing
cp -f "$DEB" "$ROOT/${PKG_NAME}_${PKG_VER}_${ARCH}.deb"

echo
dpkg-deb -I "$DEB"
echo
echo "Package: $DEB"
ls -lh "$DEB"
echo
echo "Install on another machine:"
echo "  sudo apt install ./$(basename "$DEB")"
