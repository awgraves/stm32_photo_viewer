# Photo Conversion

## What is it and why?
Image formats can be pretty complicated, especially with modern compression used in modern JPEG and PNG files.

Since I wanted to write 100% of the firmware by hand, I opted not to spend weeks on just the image compression/decompression logic aspect.

Instead, I wrote a simple PC-side utility script in Python that only needs to be run once on a batch of photos before copying them over to the sd card.

The '.pic' custom file format is a simple as it gets: just **raw left->right, top->bottom RGB565 pixel data in BIG endian format that matches the height/width dimensions of the LCD display exactly** 

This reduces the firmware steps to just:
1. Open a .pic file
2. Directly stream the byte contents *as-is* over the SPI wires to the display with no further processing!

And avoids:
1. image decompression
2. dynamic height/width resizing
3. endianness swap (little endian on sd card / ARM Cortex CPU but big endian required over the SPI wire).

## Directions
1. Create a subdir here in the same dir as this README called 'originals' and place any desired photos in there.
2. From project root, run `make convert` which will dump converted files into an 'outputs' subdir under /tools.
3. Copy the outputs subdir contents directly onto the root dir of the SD card.
