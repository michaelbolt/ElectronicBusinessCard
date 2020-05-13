/*
 * game.c
 *
 * Created on: May 13, 2020
 *     Author: Michael Bolt
 */

#include "game.h"


static Player player;

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
}

void drawPlayer(void) {
    // if the player is normal
    if (player.state == PLAYER_STATE_NORMAL) {
        display_drawSprite(PLAYER_X_POSITION, player.y, playerShip[player.keyFrame]);
        player.keyFrame++;
        player.keyFrame &= 0x03;
    }
}




void gameInit(void) {
    player.speed = 0;
    player.y = 16;
    player.state = PLAYER_STATE_NORMAL;
    player.laserCounter = 0;
    player.keyFrame = 0;
}
