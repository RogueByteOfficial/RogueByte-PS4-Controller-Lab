#!/usr/bin/env python3
"""
Asset Generator for RogueByte PS4 Controller Lab
Generates PS4 standard 512x512 icon0.png and 1920x1080 pic0.png / background.png
"""
import os
import math
from PIL import Image, ImageDraw, ImageFont

def create_icon(filename):
    size = (512, 512)
    img = Image.new('RGBA', size, (11, 15, 25, 255))
    draw = ImageDraw.Draw(img)

    # Outer border / subtle bevel
    draw.rounded_rectangle([(8, 8), (504, 504)], radius=48, outline=(40, 60, 95, 255), width=4)
    draw.rounded_rectangle([(16, 16), (496, 496)], radius=42, fill=(15, 23, 42, 255))

    # Circuit / Grid background lines
    for x in range(40, 500, 60):
        draw.line([(x, 30), (x, 480)], fill=(24, 35, 60, 120), width=1)
    for y in range(40, 500, 60):
        draw.line([(30, y), (480, y)], fill=(24, 35, 60, 120), width=1)

    # DualShock 4 stylized silhouette in center
    cx, cy = 256, 230

    # Controller main body
    body_poly = [
        (cx - 160, cy - 40), (cx - 130, cy - 90), (cx - 60, cy - 95),
        (cx + 60, cy - 95), (cx + 130, cy - 90), (cx + 160, cy - 40),
        (cx + 180, cy + 50), (cx + 150, cy + 130), (cx + 110, cy + 120),
        (cx + 70, cy + 60), (cx - 70, cy + 60),
        (cx - 110, cy + 120), (cx - 150, cy + 130), (cx - 180, cy + 50)
    ]
    draw.polygon(body_poly, fill=(30, 41, 59, 255), outline=(56, 189, 248, 200))

    # Touchpad area
    draw.rounded_rectangle([(cx - 65, cy - 88), (cx + 65, cy - 35)], radius=8, fill=(15, 23, 42, 255), outline=(0, 210, 255, 255), width=2)
    # Lightbar glow above touchpad
    draw.line([(cx - 45, cy - 92), (cx + 45, cy - 92)], fill=(0, 210, 255, 255), width=3)

    # D-Pad (Left)
    dpad_x, dpad_y = cx - 95, cy - 30
    draw.rectangle([(dpad_x - 8, dpad_y - 24), (dpad_x + 8, dpad_y + 24)], fill=(51, 65, 85, 255), outline=(148, 163, 184, 255))
    draw.rectangle([(dpad_x - 24, dpad_y - 8), (dpad_x + 24, dpad_y + 8)], fill=(51, 65, 85, 255), outline=(148, 163, 184, 255))

    # Action Buttons (Right) - Triangle, Circle, Cross, Square
    btn_x, btn_y = cx + 95, cy - 30
    draw.ellipse([(btn_x - 7, btn_y - 28), (btn_x + 7, btn_y - 14)], fill=(15, 23, 42, 255), outline=(52, 211, 153, 255), width=2) # △
    draw.ellipse([(btn_x + 14, btn_y - 7), (btn_x + 28, btn_y + 7)], fill=(15, 23, 42, 255), outline=(248, 113, 113, 255), width=2) # O
    draw.ellipse([(btn_x - 7, btn_y + 14), (btn_x + 7, btn_y + 28)], fill=(15, 23, 42, 255), outline=(96, 165, 250, 255), width=2) # X
    draw.ellipse([(btn_x - 28, btn_y - 7), (btn_x - 14, btn_y + 7)], fill=(15, 23, 42, 255), outline=(244, 114, 182, 255), width=2) # □

    # Dual Analog Sticks with Precision Reticle & Crosshair
    for sx, sy in [(cx - 50, cy + 22), (cx + 50, cy + 22)]:
        # Outer calibration ring
        draw.ellipse([(sx - 32, sy - 32), (sx + 32, sy + 32)], fill=(20, 29, 47, 255), outline=(37, 99, 235, 255), width=2)
        # Deadzone circle
        draw.ellipse([(sx - 14, sy - 14), (sx + 14, sy + 14)], outline=(245, 158, 11, 180), width=1)
        # Center crosshair
        draw.line([(sx - 28, sy), (sx + 28, sy)], fill=(56, 189, 248, 150), width=1)
        draw.line([(sx, sy - 28), (sx, sy + 28)], fill=(56, 189, 248, 150), width=1)
        # Thumbstick cap
        draw.ellipse([(sx - 18, sy - 18), (sx + 18, sy + 18)], fill=(40, 52, 75, 255), outline=(0, 210, 255, 255), width=2)
        # Stick center dot
        draw.ellipse([(sx - 3, sy - 3), (sx + 3, sy + 3)], fill=(16, 185, 129, 255))

    # Branding Banners at Bottom
    draw.rectangle([(24, 380), (488, 480)], fill=(15, 23, 42, 240))
    draw.line([(24, 380), (488, 380)], fill=(0, 210, 255, 255), width=2)

    # Text rendering using fallback default or bitmap drawing
    # Top banner "ROGUEBYTE"
    try:
        font_large = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 30)
        font_sub = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20)
        font_badge = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14)
    except:
        font_large = ImageFont.load_default()
        font_sub = font_large
        font_badge = font_large

    draw.text((cx, 405), "ROGUEBYTE", fill=(255, 255, 255), font=font_large, anchor="mm")
    draw.text((cx, 438), "PS4 CONTROLLER LAB", fill=(56, 189, 248), font=font_sub, anchor="mm")
    draw.text((cx, 465), "NATIVE DUALSHOCK 4 DIAGNOSTICS", fill=(148, 163, 184), font=font_badge, anchor="mm")

    img.save(filename, format='PNG')
    print(f"Generated {filename} ({size[0]}x{size[1]})")

def create_background(filename):
    size = (1920, 1080)
    img = Image.new('RGBA', size, (11, 14, 20, 255))
    draw = ImageDraw.Draw(img)

    # Ambient gradient glow lines
    for i in range(12):
        y = 90 * i
        draw.line([(0, y), (1920, y)], fill=(18, 25, 38, 255), width=1)
    for i in range(20):
        x = 100 * i
        draw.line([(x, 0), (x, 1080)], fill=(18, 25, 38, 255), width=1)

    # Diagonal accent lines
    draw.line([(0, 1080), (700, 0)], fill=(28, 42, 65, 180), width=2)
    draw.line([(1200, 1080), (1920, 0)], fill=(28, 42, 65, 180), width=2)

    # Top accent bar
    draw.rectangle([(0, 0), (1920, 6)], fill=(37, 99, 235, 255))
    draw.rectangle([(700, 0), (1220, 6)], fill=(0, 210, 255, 255))

    # Watermark text
    try:
        font_bg = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 64)
        font_sub = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 26)
    except:
        font_bg = ImageFont.load_default()
        font_sub = font_bg

    draw.text((960, 960), "ROGUEBYTE PS4 CONTROLLER LAB", fill=(26, 36, 54, 255), font=font_bg, anchor="mm")
    draw.text((960, 1010), "Native PlayStation 4 Homebrew Diagnostic & Calibration Suite", fill=(35, 48, 70, 255), font=font_sub, anchor="mm")

    img.save(filename, format='PNG')
    print(f"Generated {filename} ({size[0]}x{size[1]})")

if __name__ == "__main__":
    os.makedirs("assets", exist_ok=True)
    os.makedirs("sce_sys", exist_ok=True)
    create_icon("assets/icon0.png")
    create_icon("sce_sys/icon0.png")
    create_background("assets/background.png")
    create_background("sce_sys/pic0.png")
    create_background("sce_sys/pic1.png")
