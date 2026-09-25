import os
import struct
from PIL import Image

TARGET_SIZE = 40
INPUT_DIR = "assets/raw_pngs"
OUTPUT_DIR = "assets/8bit"

os.makedirs(OUTPUT_DIR, exist_ok=True)

print("--- EXPORTING 8-BIT RGB 3:3:2 SPRITES ---")

for filename in os.listdir(INPUT_DIR):
    if not filename.endswith(".png"):
        continue
        
    filepath = os.path.join(INPUT_DIR, filename)
    src_img = Image.open(filepath).convert("RGBA")
    resized_img = src_img.resize((TARGET_SIZE, TARGET_SIZE), Image.Resampling.LANCZOS)
    
    basename = os.path.splitext(filename)[0]
    out_path = os.path.join(OUTPUT_DIR, f"{basename}.spr")
    
    with open(out_path, "wb") as f:
        for y in range(TARGET_SIZE):
            for x in range(TARGET_SIZE):
                r, g, b, a = resized_img.getpixel((x, y))
                
                # Check transparency
                if a < 128:
                    # Value 128 is reserved for transparency
                    pixel_8bit = 128
                else:
                    # Convert 24-bit RGB down to 8-bit RGB 3:3:2 
                    # Scale components to fit bit-widths
                    # (R: 3 bits, G: 3 bits, B: 2 bits)
                    r_3bit = (r >> 5) & 0x07
                    g_3bit = (g >> 5) & 0x07
                    b_2bit = (b >> 6) & 0x03
                    
                    # Pack into a single byte: RRRGGGBB
                    pixel_8bit = (r_3bit << 5) | (g_3bit << 2) | b_2bit
                    
                    # If the valid color lands exactly on 128, remap it to 127
                    if pixel_8bit == 128:
                        pixel_8bit = 127
                        
                f.write(struct.pack("B", pixel_8bit))
                        
print("--- ALL 8-BIT RGB 3:3:2 SPRITES CONVERTED SUCCESSFULLY! ---")
