Slackware package build for Unraid.

The host has no build toolchain, so everything runs in a container. From the
repo root:

    docker run --rm -v "$PWD:/src" -v "$PWD/packaging:/out" \
      -v "$PWD/packaging:/scripts" gcc:15 bash /scripts/package.sh

That leaves `btop-<VER>-x86_64-<BUILD>.txz` in `packaging/`. Bump `BUILD` in
`package.sh` first, or `upgradepkg` sees the same version and skips the
install. `build-fork.sh` only compiles the binary, for a quick check without
packaging.

    upgradepkg --install-new packaging/btop-<VER>-x86_64-<BUILD>.txz
    cp packaging/btop-<VER>-x86_64-<BUILD>.txz /boot/extra/

The copy in `/boot/extra` is what makes Unraid reinstall it on every boot, so
it survives OS upgrades. Nothing is needed at runtime; `libdrm-dev` is a build
dependency only, and the scripts install it themselves.
