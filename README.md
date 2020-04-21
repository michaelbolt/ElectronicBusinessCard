# ElectronicBusinessCard
This repository contains the code I am developing for my Electronic Business Card project. To see writeups and follow my progress, follow my blog: [Mr. Dr. Prof. Bolt](https://mrdrprofbolt.wordpress.com/).

### Current State of the Project:
The MSP430FR2433 microcontroller is able to communicate with the SSD1306 OLED display through I2C and handle 8x8 sprite-based animations. A Python GUI is available to help with creating animated sprite data.

### Using the Sprite Maker GUI
1. Download the `SpriteMaker.py` file from the `PythonScripts` folder.
2. Navigte to the directory where the source file is located. Use `cd [folder name]` to navigate on Mac/Linux or `DIR [folder name]` to navigate on Windows.
3. Run the script from the command line as `python SpriteMaker.py`. If you would like to see command line options, add the `-h` option.
4. Enjoy!

### Using the SSD1306 display functions
1. At the start of each frame, call `display_frameStart()`; this erases anything drawn on the last frame from VRAM.
2. Draw up to 16 sprites at (x,y) coordinates with `display_drawSprite(uint16_t x, uint16_t y, const uint8_t *const sprite)`.
3. After drawing all sprites, call `display_drawFrame()`; this updates all changed regions of the SSD1306 display with minimal I2C communication.