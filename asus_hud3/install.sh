#!/bin/bash
THEME_NAME="asus_hud3"
THEME_DIR="/usr/share/plymouth/themes/$THEME_NAME"

echo "Installing $THEME_NAME..."
sudo mkdir -p "$THEME_DIR"
sudo cp -rfv $THEME_NAME/* $THEME_DIR
sudo plymouth-set-default-theme -R "$THEME_NAME"

if [ -f /usr/bin/dracut ]; then
    echo "Detected Void/Dracut, rebuilding initramfs..."
    sudo dracut -f
fi
echo "Done!"
