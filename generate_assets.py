#!/usr/bin/env python3
"""
Generate placeholder assets for f3vita VPK package.
Requires: uv add pillow
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_icon():
    """Create 128x128 icon0.png"""
    img = Image.new('RGBA', (128, 128), (0, 0, 0, 255))
    draw = ImageDraw.Draw(img)
    
    # Draw a simple "F3" text
    try:
        font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans-Bold.ttf", 48)
    except:
        font = ImageFont.load_default()
    
    # Draw background gradient-like effect
    for y in range(128):
        gray = int(30 + (y / 128) * 40)
        draw.line([(0, y), (128, y)], fill=(gray, gray, gray + 20, 255))
    
    # Draw "F3" centered
    text = "F3"
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    x = (128 - text_width) // 2
    y = (128 - text_height) // 2 - 10
    draw.text((x, y), text, fill=(0, 255, 255, 255), font=font)
    
    # Draw "vita" below
    try:
        small_font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans.ttf", 24)
    except:
        small_font = ImageFont.load_default()
    
    text2 = "vita"
    bbox2 = draw.textbbox((0, 0), text2, font=small_font)
    text2_width = bbox2[2] - bbox2[0]
    x2 = (128 - text2_width) // 2
    draw.text((x2, y + 50), text2, fill=(200, 200, 200, 255), font=small_font)
    
    return img


def create_bg():
    """Create 840x500 LiveArea background"""
    img = Image.new('RGBA', (840, 500), (20, 20, 30, 255))
    draw = ImageDraw.Draw(img)
    
    # Draw gradient background
    for y in range(500):
        blue = int(30 + (y / 500) * 50)
        draw.line([(0, y), (840, y)], fill=(20, 20, blue, 255))
    
    # Draw title
    try:
        font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans-Bold.ttf", 64)
    except:
        font = ImageFont.load_default()
    
    text = "f3vita"
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    x = (840 - text_width) // 2
    draw.text((x, 180), text, fill=(0, 255, 255, 255), font=font)
    
    # Draw subtitle
    try:
        small_font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans.ttf", 28)
    except:
        small_font = ImageFont.load_default()
    
    subtitle = "Storage Verification Tool"
    bbox2 = draw.textbbox((0, 0), subtitle, font=small_font)
    subtitle_width = bbox2[2] - bbox2[0]
    x2 = (840 - subtitle_width) // 2
    draw.text((x2, 260), subtitle, fill=(180, 180, 180, 255), font=small_font)
    
    return img


def create_startup():
    """Create 280x158 startup button"""
    img = Image.new('RGBA', (280, 158), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Draw rounded rectangle background
    draw.rounded_rectangle([(10, 10), (270, 148)], radius=15, fill=(0, 100, 150, 220))
    
    # Draw play icon (triangle)
    points = [(100, 50), (100, 108), (180, 79)]
    draw.polygon(points, fill=(255, 255, 255, 255))
    
    return img


def main():
    # Create directories
    os.makedirs("sce_sys/livearea/contents", exist_ok=True)
    
    # Generate and save images
    print("Generating icon0.png (128x128)...")
    icon = create_icon()
    icon.save("sce_sys/icon0.png", "PNG")
    
    print("Generating bg.png (840x500)...")
    bg = create_bg()
    bg.save("sce_sys/livearea/contents/bg.png", "PNG")
    
    print("Generating startup.png (280x158)...")
    startup = create_startup()
    startup.save("sce_sys/livearea/contents/startup.png", "PNG")
    
    print("Done! Assets generated in sce_sys/")


if __name__ == "__main__":
    main()