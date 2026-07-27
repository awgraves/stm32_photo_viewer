# Photo Conversion

## What is it and why?
Image formats can be pretty complicated, especially with compression used in modern JPEG and PNG files.

Since I wanted to write 100% of the firmware by hand, I opted not to go down the complicated rabbit hole of image decompression logic.

Instead, I wrote a simple PC-side utility script in Python that only needs to be run once on a batch of photos before copying them over to the SD card.

The '.pic' custom file format is a simple as it gets: just **raw left->right, top->bottom RGB565 pixel data in BIG endian format that matches the height/width dimensions of the LCD display exactly**.

This simplifies the firmware logic to:
1. Opening a .pic file
2. Directly streaming the byte contents *as-is* over the SPI wires to the display

And avoids:
1. image decompression
2. dynamic height/width resizing
3. endianness swap (little endian on SD card / ARM Cortex CPU but big endian required over the SPI wire)

## Directions
1. Create a subdir here in the same dir as this README called 'inputs' and place any desired photos in there.
2. From project root, run `make convert` which will dump converted files into an 'outputs' subdir under /tools.
3. Copy the outputs subdir contents directly onto the root dir of the SD card.
