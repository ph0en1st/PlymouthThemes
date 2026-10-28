#!/usr/bin/env bash
# Rootless rendering with the installed Plymouth libraries; no host configuration.
set -euo pipefail
source_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
output=$(realpath -m -- "${1:-$source_dir/previews}")
mkdir -p -- "$output"
# pkg-config emits compiler arguments, intentionally split into an array.
read -r -a flags <<< "$(pkg-config --cflags --libs ply-splash-core ply-splash-graphics libpng)"
cc -Wall -Wextra -Werror -rdynamic "$source_dir/preview/preview.c" -o "$output/preview" "${flags[@]}" -ldl
printf '[Plymouth Theme]\nName=ShadowUSB preview\nModuleName=script\n\n[script]\nImageDir=%s/assets\nScriptFile=%s/shadowusb-glint.script\n' "$source_dir" "$source_dir" > "$output/preview.plymouth"
if [[ ${2:-} == --motion ]]; then
    "$output/preview" "$output/preview.plymouth" "$output" 1920 1080 --motion > "$output/motion.log" 2>&1
    printf 'Motion PNG frames: %s/1920x1080-motion-*.png\n' "$output"
    exit 0
fi
for size in 1920x1080 1920x1200 1366x768 2560x1440; do
    "$output/preview" "$output/preview.plymouth" "$output" "${size%x*}" "${size#*x}" > "$output/$size.log" 2>&1
    printf 'Rendered %s\n' "$size"
done
printf 'PNG previews: %s\nThis does not test DRM/KMS, initramfs or SDDM hand-off.\n' "$output"
