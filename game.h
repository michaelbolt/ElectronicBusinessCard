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
// constant definitions
#define PLAYER_X_POSITION       4       // x position of sprite
#define PLAYER_Y_MAX            24      // max y position
#define PLAYER_SPEED_MAX        3       // max y speed
#define PLAYER_SPEED_STEP       1       // y speed step
#define PLAYER_STATE_NORMAL      0x01   // normal state
#define PLAYER_STATE_EXPLODED    0x02   // exploded state
#define PLAYER_LASER_DELAY      3
extern const uint8_t playerShip[][8];
// struct definition
typedef struct PLAYER_S {
    // physics
    int16_t  y;     // y position
    int16_t  speed; // y speed
    // flags/game logic
    uint16_t state;         // current player state
    uint16_t laserCounter;  // number of frames until next laser
    uint16_t keyFrame;      // which sprite keyFrame to draw
} Player;
// functions
void playerController(void);
void drawPlayer(void);



#endif  /* GAME_H_ */
