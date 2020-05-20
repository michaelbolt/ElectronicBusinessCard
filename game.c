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
static Laser playerLasers[LASER_MAX_PLAYER];
static Laser enemyLasers[LASER_MAX_ENEMY];

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
            display_drawSprite(enemyLasers[i].x, enemyLasers[i].y, acePilotLaser[enemyLasers[i].keyFrame]);
            enemyLasers[i].keyFrame++;          // update keyFrame counter
            enemyLasers[i].keyFrame &= 0x03;    // wrap keyFrame counter
        }
    }
}


//******************************************************************************
// Enemy Functions *************************************************************
//******************************************************************************

// static global array for enemies
static Enemy enemies[MAX_ENEMIES];


/*
 * ! Add an enemy at the right side of the screen
 * !
 * ! \param type: constant indicating which type of
 * !              enemy character to add:
 * !                ENEMY_TYPE_TORPEDO,
 * !                ENEMY_TYPE_BUZZ,
 * !                ENEMY_TYPE_PILOT
 * ! \param y: y coordinate of the new enemy
 * !
 * ! Enables and initializes the first available enemy in
 * ! the enemies array
 */
void addEnemy(uint16_t type, uint16_t y) {
    uint16_t i = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        // add enemy to next available location
        if (!enemies[i].type) {
            enemies[i].x = ENEMY_X_START;
            enemies[i].y = y;
            enemies[i].keyFrame = 0;
            switch(type) {
                // Interstellar Torpedo
                //   local = speed dividend
                case ENEMY_TYPE_TORPEDO:
                    enemies[i].type = ENEMY_TYPE_TORPEDO;
                    enemies[i].local = ENEMY_TORPEDO_SPEED_0;
                    break;
                // Buzz Drone
                //   local = current y direction
                case ENEMY_TYPE_BUZZ:
                    enemies[i].type = ENEMY_TYPE_BUZZ;
                    enemies[i].local = ENEMY_BUZZ_UP;
                    break;
                // Ace Pilot
                //   local = current y speed
                case ENEMY_TYPE_PILOT:
                    enemies[i].type = ENEMY_TYPE_PILOT;
                    enemies[i].local = 0;
                    break;
            }
            return;
        }
    }
}


/*
 * ! Update position of all enemies
 */
void updateEnemies() {
    // update all enabled enemies
    uint16_t i = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        switch(enemies[i].type) {
            // Interstellar Torpedo behavior
            case ENEMY_TYPE_TORPEDO:
                // move to the left
                enemies[i].x -= (enemies[i].local >> ENEMY_TORPEDO_DIVIDER);
                // increase speed speed
                enemies[i].local++;
                break;
            // Buzz Drone behavior
            case ENEMY_TYPE_BUZZ:
                // move to the left
                enemies[i].x -= ENEMY_BUZZ_SPEED_X;
                // if moving up...
                if (enemies[i].local) {
                    enemies[i].y += ENEMY_BUZZ_SPEED_Y;
                    if (enemies[i].y > ENEMY_Y_MAX) {
                        enemies[i].y = ENEMY_Y_MAX;
                        enemies[i].local = ENEMY_BUZZ_DOWN;
                    }
                }
                // else, moving down...
                else {
                    enemies[i].y -= ENEMY_BUZZ_SPEED_Y;
                    if (enemies[i].y < 0) {
                        enemies[i].y = 0;
                        enemies[i].local = ENEMY_BUZZ_UP;
                    }
                }
                break;
            // Ace Pilot behavior
            case ENEMY_TYPE_PILOT:
                // move to the left
                enemies[i].x -= ENEMY_PILOT_SPEED_X;
                // try to track the player's Y coordinate...
                // if above player window, accelerate downward
                if (enemies[i].y > (player.y + ENEMY_PILOT_WINDOW)) {
                    enemies[i].local -= 1;
                    if (enemies[i].local < (-1*ENEMY_PILOT_SPEED_Y_MAX))
                        enemies[i].local = (-1*ENEMY_PILOT_SPEED_Y_MAX);
                }
                // if below playe windowr, accelerate upward
                else if (enemies[i].y < (player.y - ENEMY_PILOT_WINDOW)) {
                    enemies[i].local += 1;
                    if (enemies[i].local < (1*ENEMY_PILOT_SPEED_Y_MAX))
                        enemies[i].local = (1*ENEMY_PILOT_SPEED_Y_MAX);
                }
                // if in player window, slow down
                else {
                    if (enemies[i].local > 0)
                        enemies[i].local--;
                    else if (enemies[i].local < 0)
                        enemies[i].local++;
                }
                // update and limit y position
                enemies[i].y += enemies[i].local;
                if (enemies[i].y > ENEMY_Y_MAX)
                    enemies[i].y = ENEMY_Y_MAX;
                else if (enemies[i].y < 0)
                    enemies[i].y = 0;
                // laser counter is implemented in upper half of keyFrame:
                //   decrement if > 0
                if (enemies[i].keyFrame & 0xFF00) {
                    enemies[i].keyFrame -= 0x0100;
                }
                //   if 0, fire and reset counter
                else {
                    fireLaser(LASER_TYPE_ENEMY, enemies[i].x, enemies[i].y);
                    enemies[i].keyFrame += (ENEMY_PILOT_LASER_COUNT << 8);
                }
                break;
            // default: disabled enemy
            default:
                break;
        }
        // disable if off screen
        if (enemies[i].x < 0)
            enemies[i].type = ENEMY_TYPE_DISABLED;
    }
}


/*
 * ! Draw all onscreen enemies
 */
void drawEnemies() {
    // draw all enabled enemy sprites
    uint16_t i = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        // draw sprite
        switch(enemies[i].type) {
            case ENEMY_TYPE_TORPEDO:
                display_drawSprite(enemies[i].x, enemies[i].y, interstellarTorpedo[enemies[i].keyFrame]);
                break;
            case ENEMY_TYPE_BUZZ:
                display_drawSprite(enemies[i].x, enemies[i].y, buzzDrone[enemies[i].keyFrame]);
                break;
            case ENEMY_TYPE_PILOT:
                display_drawSprite(enemies[i].x, enemies[i].y, acePilot[enemies[i].keyFrame & 0x00FF]);
                break;
        }
        enemies[i].keyFrame++;          // increment keyFrame counter
        enemies[i].keyFrame &= 0xFF03;  // wrap keyFrame counter
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
    for (i = 0; i < LASER_MAX_ENEMY; i++)
        enemyLasers[i].enabled = LASER_STATE_DISABLED;
    // disable all enemies
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].type = ENEMY_TYPE_DISABLED;
    }
}
