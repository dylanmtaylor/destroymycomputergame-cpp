#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <iostream>
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <random>
#include <ctime>
#include <thread>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

//other preprocessor macros
#define println(s) (printf("%s\n",(s))) //prints a string on it's own line using printf
template<typename T, int size>
int arraySize(T(&)[size]){return size;}

//conditional inclusions and other safe to change settings
#define debugAudio true
#define drawDamage true
#define ultraVerbose false
#define enableBackgroundMusic true
#define enableAnisotropicFiltering true
#define useFrameBufferObjects true
#define startFullscreen true
#define useFlamethrowerBurstPoints true
#define rasterizeDecals true
#define drawWireframe false
#define mouseWheelWeaponChangeScrollLoop false
#define showSupportedOpenGLExtensions false
#define runBenchmark false
#define showFramesPerSecond true
#define testFramesPerSecond false //displays "0123456789" instead of the actual frames per second
#define testAlphaBlending true //displays a sample image centered above the screen to test alpha blending
#define debugMouseMovement false
#define BPP 4
#define DEPTH 32
#define TRUE_RASTERIZED_NUM_WIDTH 320 //because we added padding to make it a power of two texture size (for compatibility)
#define TRUE_RASTERIZED_NUM_HEIGHT 64
#define TRUE_FLAME_ANIMATION_WIDTH 960


short getCurrentWeapon();
void setCurrentWeapon(short newWeapon);
void getScreenshot(std::string filename);
//void getScreenshot(const char* filename);
bool fileExists(const char* filename);
std::vector<std::string> getDirectoryContents(std::string path);
int randomInt(int, int);
extern int WIDTH, HEIGHT; //for tracking the size of the screen
void initializeGraphics();
void showGLInfo();
void fireWeapon();
void resetGame();
void toggleFullscreen();
bool isRunning();
bool isFiring();
void setFiring(bool currentlyFiring);
void setRunning(bool currentlyRunning);

struct Size {
    int w; //width
    int h; //height
};

struct Point {
    int x; //width
    int y; //height
};

#include "flame.h"
void setScreenSize(Size s);
Size getScreenSize();
Size getImageSize(const char* filename);
Point getMouseCoords();
void startFPSCounter();
void startInputListener();
void startAutomaticFiring();
void getScreenshot(const char* filename);
void drawScreen(SDL_Surface* screen, int h);
std::vector<Point> getPointsOnCircle(Point center, int radius,  int numPoints);
void flameOn(Point e);
#define println(s) (printf("%s\n",(s))) //prints a string on it's own line using printf

const short NONE = 9001;
const short FLAMETHROWER = 9002;
const short MACHINE_GUN = 9003;
const short MACHINE_SHOTGUN = 9004;
const short SHOTGUN = 9005;
const short RIFLE = 9006;
const Size shotgunBulletHoleSize = (Size) {16, 16};
const Size rifleBulletHoleSize = (Size) {32, 32};
const Size flameSize = (Size) {64, 128};
const int deltaMult = 1;
const int deltaD = (int)floor(-1 * deltaMult);
const int deltaI = (int)floor(1 * deltaMult);
const int ftMinRad = 24;
const int ftMaxRad = 64;
const int ftCirclePts = 32;
const int ftBurstRad = 8;
const int ftBurstPts = 32;
const int autoFiringDelay = 77;
const int sgShots = 50; //how many shots the shotgun fires at once
void toggleAlphaVisible();

#endif // COMMON_H_INCLUDED
