# /// script
# requires-python = ">=3.14"
# dependencies = [
#     "numpy>=2.5.1",
#     "pathlib>=1.0.1",
#     "pillow>=12.3.0",
# ]
# ///
import os
import numpy as np
from PIL import Image, ImageOps

# paths are relative from project root
INPUT_DIR = "./tools/originals"
OUTPUT_DIR = "./tools/outputs"

OUTPUT_SIZE = (320, 480)  # width, height
PADDING_COLOR = "#000"


def convert(input_path):
    try:
        with Image.open(input_path).convert("RGB") as im:
            rotated = ImageOps.exif_transpose(im)
            processed = ImageOps.pad(
                rotated,
                size=OUTPUT_SIZE,
                color=PADDING_COLOR,
                centering=(0.5, 0.5),
            )

            rgb_array = np.array(processed, dtype=np.uint8)

            # extract color channels
            r = rgb_array[:, :, 0]
            g = rgb_array[:, :, 1]
            b = rgb_array[:, :, 2]

            # shift components to RGB565
            r5 = (r >> 3).astype(np.uint16)
            g6 = (g >> 2).astype(np.uint16)
            b5 = (b >> 3).astype(np.uint16)

            # Create the 16-bit integers
            rgb565_array = (r5 << 11) | (g6 << 5) | b5

            # Convert to big-endian 16-bit before writing raw bytes
            rgb565_be = rgb565_array.astype(">u2")
            filename = input_path.split("/")[-1].split(".")[0]
            rgb565_be.tofile(f"{OUTPUT_DIR}/{filename}.pic")
    except OSError:
        print(f"Could not read file: {input_path}")


def process_inputs():
    for entry in os.scandir(INPUT_DIR):
        convert(entry.path)


if __name__ == "__main__":
    process_inputs()
