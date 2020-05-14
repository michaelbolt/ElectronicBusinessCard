/*
 * game.h
 *
 * Created on: May 13, 2020
 *     Author: Michael Bolt
 */

#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>
#include "hardware.h"
#include "ssd1306.h"



void gameInit(void);

/********************
 * Player Character *
 ********************/
// constant defines
#define PLAYER_X_POSITION       2       // x position of sprite
#define PLAYER_Y_MAX            24      // max y position
#define PLAYER_SPEED_MAX        3       // max y speed
#define PLAYER_SPEED_STEP       1       // y speed step
#define PLAYER_STATE_NORMAL     0x01    // normal state
#define PLAYER_STATE_EXPLODED   0x02    // exploded state
#define PLAYER_LASER_DELAY      4       // number of frames between consecutive lasers
extern const uint8_t playerShip[][8];   // player sprite defined in gameSprites.h
// struct definition
typedef struct PLAYER_S {
    // physics
    int16_t  y;     // y position
    int16_t  speed; // y speed
    // flags/game logic
    uint16_t state;         // current player state
    uint16_t laserCounter;  // number of frames until next laser fire
    uint16_t keyFrame;      // which sprite keyFrame to draw
} Player;
// functions
void playerController(void);
void drawPlayer(void);

/**********
 * Lasers *
 **********/
// constant defines
#define LASER_SPEED             6       // x speed of lasers
#define LASER_STATE_ENABLED     1       // enabled state
#define LASER_STATE_DISABLED    0       // disabled state
#define LASER_TYPE_PLAYER       1       // use to fire player laser
#define LASER_TYPE_ENEMY        0       // use to fire enemy laser
#define LASER_MAX_PLAYER        3       // max number of player lasers
#define LASER_MAX_ENEMY         5       // max number of enemy lasers
extern const uint8_t playerLaser[4][8]; // sprite defined in gameSprites.c
// struct definition
typedef struct LASER_S {
    // location
    int16_t x;  // x position
    int16_t y;  // y position
    // flags/game logic
    uint16_t enabled;   // current laser state
    uint16_t keyFrame;  // which sprite keyFrame to draw
} Laser;
void fireLaser(uint16_t type, uint16_t x, uint16_t y);
void updateLasers(void);
void drawLasers(void);



#endif  /* GAME_H_ */
