# /// script
# requires-python = ">=3.14"
# dependencies = [
#     "numpy>=2.5.1",
#     "pillow>=12.3.0",
# ]
# ///
import numpy as np
from PIL import Image, ImageOps

# paths are relative from project root
INPUT_DIR = "./tools/originals"
OUTPUT_DIR = "./tools/outputs"

OUTPUT_SIZE = (320, 480)  # width, height
PADDING_COLOR = "#FFF"


def test():
    try:
        with Image.open(f"{INPUT_DIR}/may_and_olivia.jpg").convert("RGB") as im:
            processed = ImageOps.pad(
                im.transpose(Image.Transpose.ROTATE_270),
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
            rgb565_be.tofile(f"{OUTPUT_DIR}/may_and_olivia.pic")
    except OSError:
        print("Could not read file")


if __name__ == "__main__":
    test()
