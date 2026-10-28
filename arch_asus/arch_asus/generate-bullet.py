#!/usr/bin/env python3
"""Rebuild the grey round password bullet; Pillow is development-only."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter
COLOR = (90, 90, 90)  # Exact opaque colour of the original host bullet.
SIZE, UPSCALE = 20, 4
mask=Image.new('L',(SIZE*UPSCALE,SIZE*UPSCALE))
ImageDraw.Draw(mask).ellipse((6*UPSCALE,6*UPSCALE,14*UPSCALE,14*UPSCALE),fill=255)
glow=Image.new('RGBA',mask.size,COLOR)
glow.putalpha(mask.filter(ImageFilter.GaussianBlur(2*UPSCALE)).point(lambda v:int(v*0.55)))
core=Image.new('RGBA',mask.size,COLOR);core.putalpha(mask)
Image.alpha_composite(glow,core).resize((SIZE,SIZE),Image.Resampling.LANCZOS).save(Path(__file__).resolve().parent/'bullet.png')
