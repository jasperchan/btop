set -e
export DEBIAN_FRONTEND=noninteractive
apt-get -qq update >/dev/null
apt-get -qq install -y --no-install-recommends libdrm-dev xz-utils >/dev/null
cd /src
make -j6 GPU_SUPPORT=true >/tmp/b.log 2>&1 || { echo "BUILD FAILED"; tail -20 /tmp/b.log; exit 1; }

VER=1.4.7
BUILD=6jc
PKG=/pkg
rm -rf $PKG && mkdir -p $PKG
make install PREFIX=/usr DESTDIR=$PKG >/dev/null 2>&1

# docs, mirroring the upstream SlackBuild layout
mkdir -p $PKG/usr/doc/btop-$VER
for f in CHANGELOG.md CODE_OF_CONDUCT.md CONTRIBUTING.md LICENSE README.md; do
  [ -f "$f" ] && cp "$f" $PKG/usr/doc/btop-$VER/
done

mkdir -p $PKG/install
cat > $PKG/install/slack-desc <<'SD'
# HOW TO EDIT THIS FILE:
# The "handy ruler" below makes it easier to edit a package description.  Line
# up the first '|' above the ':' following the base package name, and the '|'
# on the right side marks the last column you can put a character in.  You must
# make exactly 11 lines for the formatting to be correct.  It's also
# customary to leave one space after the ':' except on otherwise blank lines.

    |-----handy-ruler------------------------------------------------------|
btop: btop (A monitor of resources)
btop:
btop: Resource monitor that shows usage and stats for processor, memory,
btop: disks, network and processes.
btop:
btop: Built with GPU_SUPPORT=true and a patch resolving the i915 perf PMU
btop: name for discrete Intel GPUs, plus a per-engine utilisation
btop: breakdown, VRAM, temperature and power from hwmon.
btop:
btop: https://github.com/jasperchan/btop
btop:
SD

# strip to match upstream package size expectations
strip $PKG/usr/bin/btop 2>/dev/null || true

cd $PKG
tar cJf /out/btop-$VER-x86_64-$BUILD.txz .
echo "  package: $(stat -c%s /out/btop-$VER-x86_64-$BUILD.txz) bytes"
echo "  contents: $(tar tJf /out/btop-$VER-x86_64-$BUILD.txz | wc -l) entries"
tar tJf /out/btop-$VER-x86_64-$BUILD.txz | grep -E 'usr/bin|slack-desc|themes/nord' | sed 's/^/    /'
