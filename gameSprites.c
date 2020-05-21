/*
 * hardware.h
 *
 *  Created on: May 4, 2020
 *      Author: Michael Bolt
 */


#include <stdint.h>

// Player sprite
const uint8_t playerShip[4][8] = {
    {0x89, 0xD3, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C},
    {0x91, 0xD3, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C},
    {0x91, 0xCB, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C},
    {0x89, 0xCB, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C}
    };

// Laser sprites
const uint8_t playerLaser[4][8] = {
    {0x00, 0x08, 0x18, 0x10, 0x18, 0x18, 0x18, 0x18},
    {0x08, 0x18, 0x10, 0x10, 0x18, 0x18, 0x18, 0x18},
    {0x18, 0x10, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18},
    {0x10, 0x00, 0x08, 0x18, 0x18, 0x18, 0x18, 0x18}
    };
const uint8_t acePilotLaser[4][8] = {
    {0x81, 0x81, 0x81, 0x81, 0x81, 0x80, 0x01, 0x01},
    {0x81, 0x81, 0x81, 0x81, 0x81, 0x80, 0x80, 0x01},
    {0x81, 0x81, 0x81, 0x81, 0x81, 0x01, 0x80, 0x80},
    {0x81, 0x81, 0x81, 0x81, 0x81, 0x01, 0x01, 0x80}
    };

// Enemy sprites
const uint8_t interstellarTorpedo[4][8] = {
    {0x18, 0x2C, 0x2C, 0x2C, 0x34, 0x3C, 0x3C, 0x00},
    {0x18, 0x2C, 0x2C, 0x2C, 0x34, 0x3C, 0x3C, 0x3C},
    {0x18, 0x2C, 0x2C, 0x2C, 0x34, 0x3C, 0x00, 0x3C},
    {0x18, 0x2C, 0x2C, 0x2C, 0x34, 0x3C, 0x00, 0x00}
    };
const uint8_t buzzDrone[4][8] = {
    {0x04, 0x08, 0x98, 0x74, 0x26, 0x19, 0x10, 0x20},
    {0x10, 0x20, 0x1A, 0x25, 0xB4, 0x58, 0x04, 0x08},
    {0x00, 0x20, 0x3E, 0x24, 0x2C, 0x7C, 0x04, 0x00},
    {0x00, 0x46, 0x58, 0x2C, 0x24, 0x1A, 0x62, 0x00}
    };
const uint8_t acePilot[4][8] = {
    {0x81, 0x99, 0xE7, 0x7E, 0x7E, 0x66, 0x22, 0x24},
    {0x81, 0x99, 0xE7, 0x7E, 0x7E, 0x66, 0x42, 0x22},
    {0x81, 0x99, 0xE7, 0x7E, 0x7E, 0x66, 0x44, 0x42},
    {0x81, 0x99, 0xE7, 0x7E, 0x7E, 0x66, 0x24, 0x44}
    };

// Explosion sprite
const uint8_t explosion[8][8] = {
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x28, 0x10, 0x10, 0x28, 0x00, 0x00},
    {0x00, 0x48, 0x20, 0x00, 0x00, 0x20, 0x48, 0x00},
    {0x48, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x48},
    {0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64},
    {0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32},
    {0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09},
    {0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00}
    };

// digit sprites
const uint8_t digits[10][8] = {
    {0x1F, 0x11, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x09, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x17, 0x15, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x11, 0x15, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1C, 0x04, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1D, 0x15, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1F, 0x15, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x10, 0x10, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1F, 0x15, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1C, 0x14, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00}
    };
