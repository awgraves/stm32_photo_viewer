# /// script
# requires-python = ">=3.14"
# dependencies = [
#     "numpy>=2.5.1",
#     "pathlib>=1.0.1",
#     "pillow>=12.3.0",
#     "tqdm>=4.70.0",
# ]
# ///
import os
import numpy as np
from PIL import Image, ImageOps
from tqdm import tqdm

# paths are relative from project root
INPUT_DIR = "./tools/inputs"
OUTPUT_DIR = "./tools/outputs"

OUTPUT_SIZE = (320, 480)  # width, height
PADDING_COLOR = "#000"


class ImagesToPicFilesConverter:
    inputs = []
    errors = []

    def convert(self):
        self._create_output_dir()
        self._populate_inputs_list()
        self._run_conversions()

    def _create_output_dir(self):
        os.makedirs(OUTPUT_DIR, exist_ok=True)
        return

    def _populate_inputs_list(self):
        for entry in tqdm(os.scandir(INPUT_DIR)):
            if entry.is_file():
                self.inputs.append(entry.path)
        return

    def _run_conversions(self):
        for f in tqdm(self.inputs):
            self._convert_file(f)

        for err in self.errors:
            print(err)
        return

    def _convert_file(self, input_path):
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
        except Exception as e:
            self.errors.append(e)


if __name__ == "__main__":
    ImagesToPicFilesConverter().convert()
