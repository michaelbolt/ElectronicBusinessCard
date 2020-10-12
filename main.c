/*
 * main.c
 *
 *  Created on: April 9, 2020
 *      Author: Michael Bolt
 */

#include <msp430.h>
#include <stdint.h>
#include "hardware.h"
#include "i2c.h"
#include "ssd1306.h"
#include "game.h"


// main
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    clockSytem_init();  // set MCLK and SMCLK to 16 MHz
    timer_init();       // initialize 16 Hz timer for constant frame rate
    buttons_init();     // configure pushbuttons and debouncing routine
    i2c_init();         // initialize I2C interface
    ssd1306_init();     // initialize SSD1306 OLED

    // enable interrupts
    __bis_SR_register(GIE);

    // clear screen
    {
        uint16_t r = 0,
                 c = 0;
        const uint8_t blank[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        for (r = 0; r < SSD1306_COLUMNS; r += 8) {
            display_frameStart();
            for (c = 0; c < SSD1306_ROWS; c += 8) {
                display_drawSprite(r, c, blank);
            }
            display_drawFrame();
        }
    }
    // infinite main loop
    while(1) {
        // initialize game
        unsigned int frameCount = 0;
        gameInit(NEW_GAME);
        // game loop until the player runs out of lives
        unsigned int num_lives = 3;
        unsigned int enemy_spawn_counter = ENEMY_SPAWN_RATE_MIN;
        while(num_lives) {
            // randomly spawn enemies
            enemy_spawn_counter--;
            if (!enemy_spawn_counter) {
                uint16_t enemy_type = randomNumber() & 0x03;
                uint16_t y_position = randomNumber() % ENEMY_Y_MAX;
                addEnemy(enemy_type, y_position);
                enemy_spawn_counter = randomNumber() &0x0F;
                if (enemy_spawn_counter < ENEMY_SPAWN_RATE_MIN)
                    enemy_spawn_counter = ENEMY_SPAWN_RATE_MIN;
            }
            // start frame
            display_frameStart();

            playerController();                 // 1. update player character with user input
            updateEnemies();                    // 2. update enemy positions and actions
            updateLasers();                     // 3. update player and enemy laser positions
            checkCollisions();                  // 4. check if anybody needs to blow up,
                                                //    update lives and score counters
            drawPlayer();                       // 5. draw the player sprite (if alive)
            drawEnemies();                      // 6. draw the enemy sprites (if alive)
            drawLasers();                       // 7. draw the laser sprites (if alive)
            drawExplosions();                   // 8. draw any explosions
            drawScore(PLAYER_SCORE, 128, 27);   // 9a. draw the current player score
            drawScore(HIGH_SCORE, 74, 27);      // 9b. draw the current high score
            num_lives = drawLives();            // 10. draw the current number of lives

            // render the screen
            display_drawFrame();

            // sleep until next frame
            timer_sleep();

            frameCount++;
        }
        // when game over occurs...
        if (saveHighScore()) {                  // if a high-score did occur...
            // clear screen
            {
                uint16_t r = 0,
                         c = 0;
                const uint8_t blank[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                for (r = 0; r < SSD1306_COLUMNS; r += 8) {
                    display_frameStart();
                    for (c = 0; c < SSD1306_ROWS; c += 8) {
                        display_drawSprite(r, c, blank);
                    }
                    display_drawFrame();
                }
            }
            // clear frameCount to begin New High Score animation
            frameCount = 0;
            while (frameCount < HIGH_SCORE_ANIMATION_LENGTH) {
                // start frame
                display_frameStart();

                // add explosions
                highScoreAnimation(frameCount);     // add animations
                drawExplosions();                   // animate explosions
                drawScore(HIGH_SCORE, 74, 27);      // draw the new high score

                // render the screen
                display_drawFrame();

                // sleep until the next frame
                timer_sleep();

                // increment New High Score animation frame counter
                frameCount++;
            }
        }
        // restart the infinite main loop
    }
}



