# ASUS / Arch Linux Plymouth Themes Collection

A collection of custom Plymouth boot splash themes designed for ASUS laptops running Arch Linux.
Say goodbye to the boring text boot and welcome some eye candy.

---

## Themes

### 1. asus_hud3

A BGRT-like theme that integrates the vendor logo with a futuristic HUD animation.

Style: Sci-Fi / HUD
Logo: Uses a local copy of the ASUS vendor logo stored inside the theme directory
Credits: Animation logic is based on the work of Aditya Shakya (@adi1090x)

Preview: will be added later

---

### 2. arch_asus

A clean, dedicated theme featuring a rotating Arch Linux logo with subtle ASUS ROG aesthetics.
Perfect for letting everyone know "I use Arch, btw" before the OS even loads.

Style: Minimalist / Spinning Logo

Preview: will be added later

---

### 3. shadowusb-glint

ShadowUSB: a stationary metallic hood on pure black with a soft diagonal glint.
Includes reproducible assets and rootless Plymouth preview tools.
See [theme documentation](shadowusb-glint/README.md).
The ArchISO profile contains only the runtime files; generation is not a build step.

## Installation

The ASUS themes come with an automated installation script.

### Prerequisites

plymouth
git

### How to Install

1. Clone the repository

git clone https://github.com/andr31ab/PlymouthThemes.git
cd PlymouthThemes

2. Install asus_hud3

cd asus_hud3
chmod +x install.sh
sudo ./install.sh

3. Install arch_asus

cd arch_asus
chmod +x install.sh
sudo ./install.sh

Note:
The install.sh script should automatically set the theme and regenerate initramfs.
If it does not, apply the theme manually.

### Manual Theme Activation

sudo plymouth-set-default-theme -R arch_asus

---

## Testing (Without Reboot)

sudo plymouthd --debug
sudo plymouth --show-splash
sleep 10
sudo plymouth --quit

---

## Credits

@adi1090x — original animation concepts and scripts
@andr31ab / Handle — theme adaptation and configuration

---

## License

GPL-3.0
