/*
 * game.c
 *
 * Created on: May 13, 2020
 *     Author: Michael Bolt
 */

#include "game.h"

// gamewide globals -  must be at top to provide access to all functions
static uint32_t playerScore = 0;    // total score for the player
static uint16_t playerLives = 3;    // number of lives the player has
static uint16_t highScore = 65535;  // high score for all time

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
    // fire laser if button is pressed and player is alive
    if (player.state != PLAYER_STATE_EXPLODED) {
        if (readButton(BTN_SHOOT)) {
            if (!player.laserCounter) {
                fireLaser(LASER_TYPE_PLAYER, PLAYER_X_POSITION, player.y);
                player.laserCounter = PLAYER_LASER_DELAY;
            }
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
    // else, they've exploded...
    else {
        // wait for player.keyFrame to hit 0 before respawning
        if (!(player.keyFrame--)) {
            // re-spawn if lives left
            if (playerLives)        gameInit(RESPAWN);
        }
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
            // disable laser if off screen and increase score
            if (enemyLasers[i].x < 0) {
                enemyLasers[i].enabled = LASER_STATE_DISABLED;
                playerScore += 10;
            }
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
        if (enemies[i].type) {
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
}


//******************************************************************************
// Collision Detection Functions ***********************************************
//******************************************************************************

// static global array for explosions
static Explosion explosions[MAX_EXPLOSIONS];

/*
 * ! Add an explosion at the current location
 * !
 * ! \param x: x coordiante of exploding sprite
 * ! \param y: y coordinate of exploding sprite
 * !
 * ! Enables and initializes the first available explosion
 * ! in the explosions Explosion array
 */
void addExplosion(uint16_t x, uint16_t y) {
    uint16_t i = 0;
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        // add next available explosion
        if (!explosions[i].enabled) {
            explosions[i].enabled = EXPLOSION_STATE_ENABLED;
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].keyFrame = 0;
            return;
        }
    }
}


/*
 * ! Checks collisions between all on-screen game objects
 */
void checkCollisions() {
    uint16_t i = 0,
             j = 0;
    // check player laser -> enemy collisions
    for (i = 0; i < LASER_MAX_PLAYER; i++) {
        if (playerLasers[i].enabled) {                          // if laser enabled..
            uint8_t hit = MAX_ENEMIES;                          // local to hold enemy id
            for (j = 0; j < MAX_ENEMIES; j++) {                 // iterate over enemies
                if (enemies[j].type) {                              // if enemy enabled...
                    // check if X hitboxes overlap
                    if ((playerLasers[i].x + 7 >= enemies[j].x    ) &&
                        (playerLasers[i].x     <= enemies[i].x + 7)   ){
                        // torpedo Y hitbox
                        if((enemies[j].type == ENEMY_TYPE_TORPEDO)     &&
                           (playerLasers[i].y + 4 >= enemies[j].y + 2) &&
                           (playerLasers[i].y + 3 <= enemies[j].y + 5)    )
                            hit = j;
                        // buzz drone / ace pilot Y hitbox
                        else if((playerLasers[i].y + 4 >= enemies[j].y    ) &&
                                (playerLasers[i].y + 3 <= enemies[j].y + 7)   )
                            hit = j;
                    }
                }
            }
            // if something was hit..
            if (hit != MAX_ENEMIES) {
                addExplosion(enemies[hit].x, enemies[hit].y);   // add explosion
                switch(enemies[hit].type) {                     // update score
                case ENEMY_TYPE_TORPEDO:
                    playerScore += 100;
                    break;
                case ENEMY_TYPE_BUZZ:
                    playerScore += 200;
                    break;
                case ENEMY_TYPE_PILOT:
                    playerScore += 300;
                    break;
                }
                playerLasers[i].enabled = LASER_STATE_DISABLED; // remove player laser
                enemies[hit].type = ENEMY_TYPE_DISABLED;        // remove destroyed enemy
            }
        }
    }
    // check player collisions if not currently exploded
    if (player.state == PLAYER_STATE_NORMAL) {
       uint16_t hit = 0;                            // boolean to indicate if a hit is found
       // iterate over enemies
       for (i = 0; i < MAX_ENEMIES; i++) {
           // if no hit found yet and enemy enabled...
           if (!hit && enemies[i].type) {
               // check if X hitboxes overlap
               if ((PLAYER_X_POSITION + 7 >= enemies[i].x    ) &&
                   (PLAYER_X_POSITION     <= enemies[i].x + 7)   ){
                   // torpedo Y hitbox
                   if ((enemies[i].type == ENEMY_TYPE_TORPEDO) &&
                       (player.y + 7 >= enemies[i].y + 2)      &&
                       (player.y     <= enemies[i].y + 5)        ){
                       hit = 1;
                   }
                   // buzz drone / ace pilot Y hitbox
                   else if ((player.y + 7 >= enemies[i].y    ) &&
                            (player.y     <= enemies[i].y + 7)){
                       hit = 1;
                   }
               }
           }
       }
       // iterate over enemy lasers
       for (i = 0; i < LASER_MAX_ENEMY; i++) {
           // if no hit found yet and enemy laser enabled...
           if (!hit && enemyLasers[i].enabled) {
               // check if X hitboxes overlap
               if ((PLAYER_X_POSITION + 7 >= enemyLasers[i].x    ) &&
                   (PLAYER_X_POSITION     <= enemyLasers[i].x + 7)   ){
                   // check if Y hitboxes overlap
                   if ((player.y + 7 >= enemyLasers[i].y    ) &&
                       (player.y     <= enemyLasers[i].y + 7)   ){
                       hit = 1;
                   }
               }
           }
       }
       // if the player was hit...
       if (hit) {
           player.state = PLAYER_STATE_EXPLODED;        // player is now 'sploded
           player.keyFrame = PLAYER_RESPAWN_DELAY;      // delay between spawning
           playerLives--;                               // decrement playerLives counter
           addExplosion(PLAYER_X_POSITION, player.y);    // player's ship explodes
       }
    }
}


/*
 * ! Draw all on-screen explosions and disable once animation is complete
 */
void drawExplosions() {
    uint16_t i = 0;
    for (i = 0;i < MAX_EXPLOSIONS; i++) {
        if (explosions[i].enabled) {
            display_drawSprite(explosions[i].x, explosions[i].y, explosion[explosions[i].keyFrame]);
            explosions[i].keyFrame++;           // update keyFrame counter
            explosions[i].keyFrame &= 0x07;     // wrap keyFrame counter
            if (!explosions[i].keyFrame)        // if animation is complete..
                explosions[i].enabled = EXPLOSION_STATE_DISABLED;   // disable explosion
        }
    }
}


//******************************************************************************
// Game Logic Functions ********************************************************
//******************************************************************************

/*
 * ! Initialize data for start of game / respawn
 * !
 * ! \param respawn: boolean indicating if this is a respawn (True) or
 * !                 new game (False)
 */
void gameInit(uint8_t respawn) {
    // if not respawning, initialize score and lives
    if (!respawn) {
        // initialize score
        playerScore = 0;
        // initialize player lives
        playerLives = 3;
    }
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
    // disable all explosions
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        explosions[i].enabled = 0;
    }
}


/*
 * ! handle a game over scenario.
 */
void gameOver(void) {
    gameInit(NEW_GAME);
}


/*
 * ! Draw a right-aligned score at the chosen (x,y) coordinate
 *
 * ! \param score: value to write
 * ! \param x: x value for rightmost edge of score
 * ! \param y: y value to draw score at
 * !
 * ! Draws the provided score number using 3x5 digit sprites from right
 * ! to left from the provided (x,y) coordinatee
 */
void drawScore(uint16_t player_score, uint16_t x, uint16_t y) {
    uint16_t score = 0;                         // local copy of score to manipulate
    if (player_score)   score = playerScore;
    else                score = highScore;
    uint16_t index = DIGIT_WIDTH;               // position to start writing from
    do {
        // draw each digit one at a time
        uint16_t lsb = score % 10;  // store LSB
        score /= 10;                // remove LSB
        display_drawSprite(x - index, y, digits[lsb]);
        index += DIGIT_WIDTH;
    } while (score);
}


/*
 * ! Draw the player lives in the top left corner of the screen
 * !
 * ! \returns the current number of lives
 */
uint16_t drawLives(void) {
    uint16_t i = 0;             // local counter for looping
    uint16_t index = 0;         // location to draw to
    for(i = 0; i < playerLives; i++) {
        display_drawSprite(index, 27, playerLife[0]);   // draw each sprite
        index += LIFE_WIDTH;                            // update position to write to
    }
    return playerLives;
}
