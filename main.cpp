// game constants
#define SCREEN_SIZE_X 320.0
#define SCREEN_SIZE_Y 240.0

// gameplay params
#define GAME_SCALE 3
#define MENU_SCALE 4
#define TIME_BETWEEN_FRUITS 2.0


// sim libraries
#include "simulator_libraries/FEHLCD.h"
#include "simulator_libraries/FEHUtility.h"
#include "simulator_libraries/FEHRandom.h"

// paige libraries :)
#include "custom_libraries/Vector.cpp"
#include "custom_libraries/Sprite.cpp"
#include "custom_libraries/Fruit.cpp"

// c++ libraries
#include <fstream>
#include <iostream>
#include <vector>


void showMenu();

void showStatsScreen();
void showHowToScreen();
void showCredits();
void quitGame();

void doGameplayLoop();
Fruit* makeRandomFruit();
void showGameOverScreen(float);

void waitForBackButtonPress();

void waitForTouch();
void waitForNoTouch();
void waitForTap();

void writeTime(float);
float playerHighScore();
int gameCount();

/*
 * Entry point to the application
 */
int main() {
    showMenu();
    return 0;
}

/*
 * menu screens!
 */
void showMenu() {
    // display menu
    Sprite logo("menu/logo", Vector2(41, 21));
    logo.scale(MENU_SCALE);
    logo.move(Vector2(7, 4) * MENU_SCALE);

    Sprite play("menu/play_button", Vector2(46, 25));
    play.scale(MENU_SCALE);
    play.move(Vector2(4, 30) * MENU_SCALE);

    Sprite stats("menu/stats_button", Vector2(26, 19));
    stats.scale(MENU_SCALE);
    stats.move(Vector2(52, 2) * MENU_SCALE);

    Sprite howto("menu/how_to_button", Vector2(27, 16));
    howto.scale(MENU_SCALE);
    howto.move(Vector2(52, 22) * MENU_SCALE);

    Sprite credits("menu/credits_button", Vector2(26, 15));
    credits.scale(MENU_SCALE);
    credits.move(Vector2(52, 41) * MENU_SCALE);

    // start the menu
    while (true) {
        LCD.SetBackgroundColor(BLACK);
        LCD.Clear();

        // draw menu
        logo.draw();
        play.draw();
        stats.draw();
        howto.draw();
        credits.draw();

        // check for button presses
        int xpos, ypos;
        if (LCD.Touch(&xpos, &ypos)) {
            Vector2 screenPoint(xpos, ypos);

            if (play.isPointWithin(screenPoint)) {
                doGameplayLoop();
            } else if (stats.isPointWithin(screenPoint)) {
                showStatsScreen();
            } else if (howto.isPointWithin(screenPoint)) {
                showHowToScreen();
            } else if (credits.isPointWithin(screenPoint)) {
                showCredits();
            }

            waitForNoTouch();
        }

        // update screen
        LCD.Update();
    }
}


void showStatsScreen() {
    LCD.Clear();

    // draw stats background in the center of the screen
    Sprite statsBack("menu/stats_background", Vector2(80, 60));
    statsBack.scale(3);
    statsBack.anchorPoint(Vector2(0.5, 0.5));
    statsBack.pos(Vector2(SCREEN_SIZE_X/2, SCREEN_SIZE_Y/2));

    statsBack.draw();

    //display high score
    float highScore = playerHighScore();
    LCD.SetFontColor(BLACK);
    LCD.WriteAt(highScore, 200, 91);

    //display game count
    int gameCounter = gameCount();
    LCD.SetFontColor(BLACK);
    LCD.WriteAt(gameCounter, 250, 130);
    LCD.Update();

    waitForBackButtonPress();
}


void showHowToScreen() {
    LCD.Clear();

    // draw howto screen in the middle
    Sprite howToBack("menu/instruction_background", Vector2(80, 60));
    howToBack.scale(3);
    howToBack.anchorPoint(Vector2(0.5, 0.5));
    howToBack.pos(Vector2(SCREEN_SIZE_X/2, SCREEN_SIZE_Y/2));
    howToBack.draw();
    LCD.Update();

    waitForBackButtonPress();
}


void showCredits() {
    LCD.Clear();

    Sprite creditsBack("menu/credits_background", Vector2(80, 60));
    creditsBack.scale(4);
    creditsBack.anchorPoint(Vector2(0.5, 0.5));
    creditsBack.pos(Vector2(SCREEN_SIZE_X/2, SCREEN_SIZE_Y/2));
    creditsBack.draw();
    LCD.Update();

    waitForBackButtonPress();
}


/*
 * gameplay!!
 */
void doGameplayLoop() {
    // make character
    Sprite character("pointer", Vector2(8, 8));
    character.anchorPoint(Vector2(0.5, 0.5));
    character.scale(2);

    // fruit list
    vector<Fruit*> projectiles;

    // timing vars
    float lastTime = TimeNow();
    float lastFruitSpawnTime = TimeNow() - 4;

    // gameplay vars
    float playerTime = 0.0;
    bool gameOver = false;

    // game should keep playing as long as the screen is being touched
    int xpos, ypos;
    while (LCD.Touch(&xpos, &ypos) && !gameOver) {
        Vector2 mousePos = Vector2(xpos, ypos);

        // clear
        LCD.Clear();

        // draw background
        LCD.SetBackgroundColor(BLACK);

        // draw character
        character.move(mousePos);

        // step fruits
        float dt = TimeNow()-lastTime;
        for(Fruit* f : projectiles) {

            f->stepPath(dt);

            // check if its hitting the mouse
            if(f->sprite()->isPointWithin(mousePos)){
                gameOver = true;
            }

        }
        lastTime = TimeNow();

        // create new fruit
        if (TimeNow() - lastFruitSpawnTime > 3) {
            projectiles.push_back(makeRandomFruit());
            lastFruitSpawnTime = TimeNow();
        }

        //increment timer
        playerTime = playerTime + dt;

        //write current time to top right corner of screen
        LCD.WriteAt(playerTime, 240, 15);

        // update screen
        LCD.Update();
    }

    // garbage collect fruit
    for(Fruit* f : projectiles) {
        delete f;
    }

    showGameOverScreen(playerTime);

    // write time to file
    writeTime(playerTime);

    waitForTap();
}

// makes and returns a new random fruit to be used in gameplay
Fruit* makeRandomFruit() {
    // list all possible fruits
    string fruits[] = {LEMON, APPLE, WATERMELON, TANGERINE};
    // choose a random one
    string chosenFruit = fruits[Random.RandInt() % 4];

    // choose a spawning position for the fruit
    Vector2 spawnPos =  Vector2(Random.RandInt(), Random.RandInt());

    // choose a random speed for the fruit between 1 and 2
    float speed = 1 + (Random.RandInt() % 10)/10.0;

    // make the fruit and scale it
    Fruit* newFruit = new Fruit(chosenFruit, speed, spawnPos);
    newFruit->sprite()->anchorPoint(Vector2(0.5, 0.5));//
    newFruit->sprite()->scale(2);

    return newFruit;
}

// final screen after the player touches a fruit
void showGameOverScreen(float finalTime) {
    LCD.SetBackgroundColor(BLACK);
    LCD.Clear();

    // draw gameover screen in the middle
    Sprite gameOverScreen("menu/game_over_background", Vector2(80, 60));
    gameOverScreen.anchorPoint(Vector2(0.5, 0.5));
    gameOverScreen.move(Vector2(SCREEN_SIZE_X/2, SCREEN_SIZE_Y/2));

    // write final time to screen
    LCD.SetFontColor(CYAN);
    LCD.WriteAt(finalTime, 187, 166);

    LCD.Update();
}

void waitForBackButtonPress() {
    // draw back button
    Sprite backButton("menu/back_button", Vector2(16, 8));
    backButton.anchorPoint(Vector2(0, 1));
    backButton.pos(Vector2(8, SCREEN_SIZE_Y - 8));//
    backButton.draw();
    LCD.Update();

    // wait for player to touch the button
    bool touching = false;
    int xpos=0, ypos=0;

    while(!(touching && backButton.isPointWithin(Vector2(xpos, ypos)))) {
        touching = LCD.Touch(&xpos, &ypos);
    }
}


// waiting functions
void waitForNoTouch() {
    int x,y;
    while (LCD.Touch(&x, &y)) {
        Sleep(1.0/60.0);
    }
}

void waitForTouch() {
    int x,y;
    while (!LCD.Touch(&x, &y)) {
        Sleep(1.0/60.0);
    }
}

void waitForTap() {
    waitForNoTouch();
    waitForTouch();
    waitForNoTouch();
}

// write time to file
void writeTime(float t) {
    ofstream playerTimeFile;
    playerTimeFile.open("playerTimeFile.txt", ofstream::app);
    playerTimeFile  << t << endl;
    playerTimeFile.close();

}

// find the player's highscore from the file
float playerHighScore() {
    ifstream playerTimeFile;
    playerTimeFile.open("playerTimeFile.txt");
    
    float score, highScore=0;
    while (playerTimeFile >> score){
        if (highScore < score) {
            highScore = score;
        }
    }

    playerTimeFile.close();

    return highScore;
}

// count the saved games
int gameCount() {
    ifstream playerTimeFile;
    playerTimeFile.open("playerTimeFile.txt");

    int counter = 0;
    float temp;
    while (playerTimeFile >> temp){
        counter++;
    }

    playerTimeFile.close();

    return counter;
}