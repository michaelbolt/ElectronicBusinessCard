/*
 * game.c
 *
 * Created on: May 13, 2020
 *     Author: Michael Bolt
 */

#include "game.h"


//******************************************************************************
// Player Character Functions **************************************************
//******************************************************************************

// static global Player struct
static Player player;


/*
 * ! Handle user input and update player character
 * !
 * ! Uses an acceleration based player movement method to determine
 * ! the new location of the player location; additionally, fires
 * ! player lasers if sufficient time has passed since the last
 * ! laser was fired and the button is pressed
 */
void playerController(void) {
    // update speed based on input
    if(readButton(BTN_UP))
        player.speed += PLAYER_SPEED_STEP;
    else if (readButton(BTN_DOWN))
        player.speed -= PLAYER_SPEED_STEP;
    else {
        if (player.speed > 0)
            player.speed--;
        else if (player.speed < 0)
            player.speed++;
    }
    // limit max speed
    if (player.speed > PLAYER_SPEED_MAX)
        player.speed = PLAYER_SPEED_MAX;
    else if (player.speed < (-1*PLAYER_SPEED_MAX))
        player.speed = (-1*PLAYER_SPEED_MAX);
    // update and limit position
    player.y += player.speed;
    if (player.y > (PLAYER_Y_MAX))
        player.y = (PLAYER_Y_MAX);
    else if (player.y < 0)
        player.y = 0;
    // decrement laserCounter if > 0
    if (player.laserCounter)
        player.laserCounter--;
    // fire laser if button is pressed
    if (readButton(BTN_SHOOT)) {
        if (!player.laserCounter) {
            fireLaser(LASER_TYPE_PLAYER, PLAYER_X_POSITION, player.y);
            player.laserCounter = PLAYER_LASER_DELAY;
        }
    }
}


/*
 * ! Draw player sprite (if not exploded)
 */
void drawPlayer(void) {
    // if the player has not exploded
    if (player.state == PLAYER_STATE_NORMAL) {
        display_drawSprite(PLAYER_X_POSITION, player.y, playerShip[player.keyFrame]);
        player.keyFrame++;          // increment keyFrame counter
        player.keyFrame &= 0x03;    // wrap keyFrame counter
    }
}



//******************************************************************************
// Laser Functions *************************************************************
//******************************************************************************

// static global arrays for player and enemy lasers
Laser playerLasers[LASER_MAX_PLAYER];
Laser enemyLasers[LASER_MAX_ENEMY];

/*
 * ! Fire a laser from the current sprite's location
 * !
 * ! \param type: constant indicating if the laser is a
 * !              player or enemy laser
 * !              LASER_TYPE_PLAYER, LASER_TYPE_ENEMY
 * ! \param x: x coordinate of sprite firing the laser
 * ! \param y: y coordinate of sprite firing the laser
 * !
 * ! Enables and initializes the first available laser in
 * ! the playerLasers or enemyLasers Laser array
 */
void fireLaser(uint16_t type, uint16_t x, uint16_t y) {
    // player laser
    if (type) {
        uint16_t i = 0;
        for (i = 0; i < LASER_MAX_PLAYER; i++) {
            // fire next available laser from player position
            if (!playerLasers[i].enabled) {
                playerLasers[i].enabled = LASER_STATE_ENABLED;
                playerLasers[i].x = PLAYER_X_POSITION;
                playerLasers[i].y = y;
                playerLasers[i].keyFrame = 0;
                return;
            }
        }
    }
    // enemy laser
    else {
        uint16_t i = 0;
        for (i = 0; i < LASER_MAX_ENEMY; i++) {
            // fire next available laser from enemy position
            if (!enemyLasers[i].enabled) {
                enemyLasers[i].enabled = LASER_STATE_ENABLED;
                enemyLasers[i].x = x;
                enemyLasers[i].y = y;
                enemyLasers[i].keyFrame = 0;
                return;
            }
        }
    }
}


/*
 * ! Update position of all lasers
 */
void updateLasers(void) {
    // update all enabled player lasers
    uint16_t i = 0;
    for (i = 0; i < LASER_MAX_PLAYER; i++) {
        if (playerLasers[i].enabled) {
            // move to the right
            playerLasers[i].x += LASER_SPEED;
            // disable laser if off screen
            if (playerLasers[i].x > SSD1306_COL_STOP)
                playerLasers[i].enabled = LASER_STATE_DISABLED;
        }
    }
    // update all enabled enemy lasers
    for (i = 0; i < LASER_MAX_ENEMY; i++) {
        if (enemyLasers[i].enabled) {
            // move to the left
            enemyLasers[i].x -= LASER_SPEED;
            // disable laser if off screen
            if (enemyLasers[i].x < 0)
                enemyLasers[i].enabled = LASER_STATE_DISABLED;
        }
    }
}


/*
 * ! Draw all on-screen lasers
 */
void drawLasers(void) {
    // draw all enabled player lasers
    uint16_t i = 0;
    for (i = 0; i < LASER_MAX_PLAYER; i++) {
        if (playerLasers[i].enabled) {
            display_drawSprite(playerLasers[i].x, playerLasers[i].y, playerLaser[playerLasers[i].keyFrame]);
            playerLasers[i].keyFrame++;         // update keyFrame counter
            playerLasers[i].keyFrame &= 0x03;   // wrap keyFrame counter
        }
    }
    // draw all enabled enemy lasers
    for (i = 0; i < LASER_MAX_ENEMY; i++) {
        if (enemyLasers[i].enabled) {
            display_drawSprite(enemyLasers[i].x, enemyLasers[i].y, playerLaser[enemyLasers[i].keyFrame]);
            enemyLasers[i].keyFrame++;          // update keyFrame counter
            enemyLasers[i].keyFrame &= 0x03;    // wrap keyFrame counter
        }
    }
}


//******************************************************************************
// Game Logic Functions ********************************************************
//******************************************************************************

/*
 * ! Initialize all data for start of game
 */
void gameInit(void) {
    // initialize player character
    player.y = 16;
    player.speed = 0;
    player.state = PLAYER_STATE_NORMAL;
    player.laserCounter = 0;
    player.keyFrame = 0;
    // disable all player and enemy lasers
    uint16_t i = 0;
    for (i = 0; i < LASER_MAX_PLAYER; i++)
        playerLasers[i].enabled = LASER_STATE_DISABLED;
    for( i = 0; i < LASER_MAX_ENEMY; i++)
        enemyLasers[i].enabled = LASER_STATE_DISABLED;
}
