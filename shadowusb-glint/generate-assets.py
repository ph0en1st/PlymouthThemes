#!/usr/bin/env python3
"""Generate transparent light layers, not screen-sized animation frames."""
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

SIZE = 640
LAYERS = 24
LIGHT = (222, 242, 255)
GLOW = (115, 185, 244)
ROOT = Path(__file__).resolve().parent
OUT = ROOT / 'assets'
OUT.mkdir(exist_ok=True)
logo = Image.open(ROOT / 'assets-src/logo.png').convert('RGBA').resize((SIZE, SIZE), Image.Resampling.LANCZOS)
logo.save(OUT / 'logo.png')
alpha = np.asarray(logo.getchannel('A'), dtype=float) / 255
rgb = np.asarray(logo, dtype=float)[:, :, :3] / 255
# Bright faces catch more light, but blue facets retain a softer reflection.
material = 0.35 + 0.65 * rgb.mean(axis=2)
y, x = np.mgrid[0:SIZE, 0:SIZE]
projection = (x - y + SIZE) / (2 * SIZE)
for i, center in enumerate(np.linspace(-0.15, 1.15, LAYERS)):
    distance = projection - center
    beam = 0.80 * np.exp(-0.5 * (distance / 0.035)**2)
    beam += 0.23 * np.exp(-0.5 * (distance / 0.10)**2)
    mask = Image.fromarray(np.uint8(np.clip(beam * alpha * material, 0, 1) * 255))
    glow = Image.new('RGBA', logo.size, GLOW)
    glow.putalpha(mask.filter(ImageFilter.GaussianBlur(7)).point(lambda v: int(v * 0.52)))
    shine = Image.new('RGBA', logo.size, LIGHT)
    shine.putalpha(mask)
    Image.alpha_composite(glow, shine).save(OUT / f'glint-{i}.png')
entry = Image.new('RGBA', (600, 64))
ImageDraw.Draw(entry).rounded_rectangle((1, 1, 598, 62), radius=10, fill=(9, 13, 18, 245), outline=(110, 153, 195, 255), width=2)
entry.save(OUT / 'entry.png')

# Raster password bullets work even when initramfs fonts lack U+2022.
UPSCALE = 4
bullet = Image.new('RGBA', (20*UPSCALE, 20*UPSCALE))
mask = Image.new('L', bullet.size)
ImageDraw.Draw(mask).ellipse((6*UPSCALE,6*UPSCALE,14*UPSCALE,14*UPSCALE),fill=255)
glow = Image.new('RGBA', bullet.size, (157,203,252))
glow.putalpha(mask.filter(ImageFilter.GaussianBlur(2*UPSCALE)).point(lambda v: int(v*0.45)))
core = Image.new('RGBA', bullet.size, (224,226,232)); core.putalpha(mask)
Image.alpha_composite(glow,core).resize((20,20),Image.Resampling.LANCZOS).save(OUT/'bullet.png')
