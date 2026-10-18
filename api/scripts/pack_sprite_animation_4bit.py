import os
import struct
from PIL import Image

INPUT_DIR = "assets/raw_pngs"
OUTPUT_DIR = "assets/4bit"

os.makedirs(OUTPUT_DIR, exist_ok=True)

SYSTEM_PALETTE = {
    0:  (0, 0, 0),        # TFT_BLACK
    1:  (255, 255, 255),  # TFT_WHITE
    2:  (211, 211, 211),  # TFT_LIGHTGREY
    3:  (255, 0, 0),      # TFT_RED
    4:  (255, 165, 0),    # TFT_ORANGE
    5:  (255, 255, 0),    # TFT_YELLOW
    6:  (0, 255, 0),      # TFT_GREEN
    7:  (0, 255, 255),    # TFT_CYAN
    8:  (0, 0, 255),      # TFT_BLUE
    9:  (255, 0, 255),    # TFT_MAGENTA
    10: (128, 0, 0),      # TFT_MAROON
    11: (0, 128, 0),      # TFT_DARKGREEN
    12: (0, 128, 128),    # TFT_DARKCYAN
    13: (0, 0, 128),      # TFT_NAVY
    14: (255, 192, 203),  # TFT_PINK
}

def get_closest_palette_index(r, g, b):
    min_distance = float('inf')
    closest_idx = 1

    for idx, (pr, pg, pb) in SYSTEM_PALETTE.items():
        distance = (r - pr)**2 + (g - pg)**2 + (b - pb)**2
        if distance < min_distance:
            min_distance = distance
            closest_idx = idx

    return closest_idx

for TARGET_SIZE in [18]:
    print(f"--- EXPORTING 4-BIT SPRITES FOR TARGET SIZE: {TARGET_SIZE} ---")

    for filename in os.listdir(INPUT_DIR):
        if not filename.endswith(".png"):
            continue

        filepath = os.path.join(INPUT_DIR, filename)
        src_img = Image.open(filepath).convert("RGBA")
        
        orig_w, orig_h = src_img.size
        
        num_frames = max(1, orig_w // orig_h)
        
        target_width = TARGET_SIZE * num_frames
        resized_img = src_img.resize((target_width, TARGET_SIZE), Image.Resampling.LANCZOS)

        basename = os.path.splitext(filename)[0]
        out_path = os.path.join(OUTPUT_DIR, f"{basename}.spr")

        pixel_indices = []

        for frame_idx in range(num_frames):
            start_x = frame_idx * TARGET_SIZE
            
            for y in range(TARGET_SIZE):
                for x in range(TARGET_SIZE):
                    r, g, b, a = resized_img.getpixel((start_x + x, y))

                    if a < 128:
                        pixel_idx = 15  # Index 15 is Chroma Key
                    else:
                        pixel_idx = get_closest_palette_index(r, g, b)
                        if pixel_idx == 15:
                            pixel_idx = 14  # Fallback to Pink

                    pixel_indices.append(pixel_idx & 0x0F)

        with open(out_path, "wb") as f:
            for i in range(0, len(pixel_indices), 2):
                high_nibble = pixel_indices[i]
                low_nibble  = pixel_indices[i + 1]
                packed_byte = (high_nibble << 4) | low_nibble
                f.write(struct.pack("B", packed_byte))
                
        print(f" Saved: {out_path} ({num_frames} frame(s), size: {os.path.getsize(out_path)} bytes)")

print("--- ALL 4-BIT SPRITE STRIPS ENCODED SUCCESSFULLY ---")
