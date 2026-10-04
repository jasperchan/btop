set -e
export DEBIAN_FRONTEND=noninteractive
apt-get -qq update >/dev/null
apt-get -qq install -y --no-install-recommends libdrm-dev >/dev/null
cd /src
make -j6 GPU_SUPPORT=true >/src/build-fork.log 2>&1 || { echo "BUILD FAILED"; tail -20 /src/build-fork.log; exit 1; }
cp bin/btop /out/btop-fork-build
echo "  built OK: $(stat -c%s /out/btop-fork-build) bytes"
