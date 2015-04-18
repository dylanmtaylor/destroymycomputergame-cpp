/*
"Destroy My Computer" - a simple, open-source OpenGL stress-relief game written by Dylan Taylor and Scott Ketelaar.
This code uses new functionality found in the upcoming C++0x standard. Expect things to break.
This game is currently a work in progress, and should not be considered stable in any way.
When done, this code will be uploaded to GitHub and released under the GPL license.
*/
#include "common.h"
#include <mutex>
#include "ogg.h"
#include "damage.h"
#include "flame.h"
#ifdef _WIN32
#include "windows_specific.h"
#endif

#ifdef __linux__
char SCRSHOT_NAME[] = "/tmp/screen.bmp"; //store the image file to /tmp.
#else
char SCRSHOT_NAME[] = "screen.bmp";
#endif

char BGM_FILE[] = "res/bgm.ogg";
char FIRE_SOUND[] ="res/fire.ogg";
char RIFLE_SOUND[] ="res/rifle.ogg";
char MACHINEGUN_SOUND[] = "res/machinegun.ogg";
char SHOTGUN_SOUND[] ="res/shotgun.ogg";
SDL_Event event;
int mouse_x, mouse_y = 0;
bool running = false; //whether or not the game is running
bool firing = false;


std::mt19937 mtRandomization; //Mersenne twister random number generation, see http://en.wikipedia.org/wiki/Mersenne_twister
SDL_Surface *screen;
bool busy = false;
short currentWep = RIFLE;
//this is easier and faster than writing code to detect the amount of padding

int randomInt(int min, int max) {     //quickly generates very high quality random integers
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(mtRandomization);
}

short getCurrentWeapon() {
    return currentWep;
}

void setCurrentWeapon(short newWeapon) {
    currentWep = newWeapon;
}

bool isRunning() {
    return running;
}

Point getMouseCoords() {
    return (Point){mouse_x,mouse_y};
}

bool isFiring() {
    return firing;
}

void setFiring(bool currentlyFiring) {
    firing = currentlyFiring;
}

void setRunning(bool currentlyRunning) {
    running = currentlyRunning;
}
/**
    This method actually plays the file. Don't call this directly, as it won't be threaded. Use play_ogg instead.
*/
void ogg_player(char *filename, bool loop) {
    ogg_stream ogg;
    printf("Attempting to play %s...\n", filename);
    alutInit(0, 0);
    do {
        try {
            ogg.open(filename);
            #if debugAudio
                ogg.display();
            #endif
            if(!ogg.playback()) throw string("Ogg refused to play.\n");
            while(ogg.update()) {
                if(!ogg.playing()) {
                    if(!ogg.playback()) {
                        throw string("Ogg abruptly stopped.\n");
                    } else {
                        cout << "Ogg stream was interrupted.\n";
                    }
                }
            }
        } catch(string error) {
            cout << error;
            cin.get();
            ogg.release();
            return;
        }
        ogg.release();
        println("looping playback stream");
    } while (loop && running);
    alutExit();
    return;
}

void play_ogg(char *filename, bool loop) { //creates a new thread to play the audio in
    if (fileExists(filename)) {
        #ifndef __MINGW32__
        std::thread t(ogg_player,filename, loop);
        t.detach();
        #endif
    } else {
        printf("ERROR: %s does not exist.\n", filename);
    }
}

void loadGame() {
    initializeGraphics();
    #if enableBackgroundMusic
        //now we need to load our background music
        printf("background music should load...\n");
        play_ogg(BGM_FILE,true);
    #else
        println("background music is disabled.");
    #endif
        play_ogg(MACHINEGUN_SOUND, true);
}

void displayKey(SDL_KeyboardEvent *key)
{
	printf( "%s (%d)\n", SDL_GetKeyName(key->keysym.sym), key->keysym.sym);
}

void inputHandler() {
    while (isRunning()) {
        try {
            if(SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    setRunning(false);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    println("Mouse button pressed");
                    switch (event.button.button) {
                        case SDL_BUTTON_WHEELDOWN:
                            setCurrentWeapon(((((getCurrentWeapon() - 1) > 9001) && ((getCurrentWeapon() - 1) < 9007)) ? (getCurrentWeapon() - 1) :
                                #if mouseWheelWeaponChangeScrollLoop
                                    9006
                                #else
                                    getCurrentWeapon()
                                #endif
                            ));
                            break;
                        case SDL_BUTTON_WHEELUP:
                            setCurrentWeapon(((((getCurrentWeapon() + 1) > 9001) && ((getCurrentWeapon() + 1) < 9007)) ? (getCurrentWeapon() + 1) :
                                #if mouseWheelWeaponChangeScrollLoop
                                    9006
                                #else
                                    getCurrentWeapon()
                                #endif
                            ));
                            break;
                        case SDL_BUTTON_LEFT:
                        case SDL_BUTTON_RIGHT:
                            fireWeapon();
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    println("Mouse button released");
                    setFiring(false);
                    break;
                case SDL_MOUSEMOTION:
                    SDL_GetMouseState(&mouse_x,&mouse_y);
                    #if debugMouseMovement
                        printf("Mouse movement detected. X: %d Y: %d\n",mouse_x,mouse_y);
                    #endif
                    break;
                case SDL_KEYDOWN:
                    SDLKey keyPressed = event.key.keysym.sym;
                    switch (keyPressed) {
                        case SDLK_ESCAPE:
                            println("Escape key pressed.");
                        case SDLK_F12: //in case escape doesn't get detected, we can also use F12.
                            setRunning(false);
                            break;
                        case SDLK_F1:
                            setCurrentWeapon(RIFLE);
                            break;
                        case SDLK_F2:
                            setCurrentWeapon(MACHINE_GUN);
                            break;
                        case SDLK_F3:
                            setCurrentWeapon(SHOTGUN);
                            break;
                        case SDLK_F4:
                            setCurrentWeapon(MACHINE_SHOTGUN);
                            break;
                        case SDLK_F5:
                            setCurrentWeapon(FLAMETHROWER);
                            break;
                        case SDLK_HOME:
                            toggleAlphaVisible();
                            println("Home key pressed. Toggling alpha test visibility.");
                            break;
                        case SDLK_DELETE:
                            println("Delete key pressed. Removing all damage, and resetting the game");
                            resetGame();
                            break;
                        case SDLK_F11:
                            println("F11 pressed. Toggling fullscreen...");
                            toggleFullscreen();
                            break;
                        default:
                            printf("An unhandled key was pressed: ");
                            displayKey(&event.key);
                        }
                    break;
                }
            }
        } catch(...) {
            println("Exception occurred handling input...");
        }
    }
}

void startInputListener() {
    std::thread t(inputHandler);
    t.detach();
}

std::vector<Point> getPointsOnCircle(Point center, int radius, int numPoints) {
    //Reference: http://en.wikipedia.org/wiki/Circle#Cartesian_coordinates
    double alpha = (6.283) / numPoints;
    double theta;
    std::vector<Point> points;
    for (int i = 0; i < numPoints; i++) {
        theta = alpha * i;
        points.push_back((Point) {
            (int)floor(center.x + (cos(theta) * radius)), // X
            (int)floor(center.y + (sin(theta) * radius))}); // Y
    }
    return points;
}

int main(int argc, char* argv[]) {
    getScreenshot(std::string(SCRSHOT_NAME));
    std::this_thread::sleep_for(std::chrono::microseconds(10000)); //give the screenshot script time to run
    int h=0;
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        printf("Unable to initialize SDL");
        return 1;
    }
    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL))) {
        SDL_Quit();
        return 1;
    }
#if startFullscreen
    toggleFullscreen();
#endif
    //set up SDL's OpenGL defaults
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,            8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,          8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,           8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,          8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,          16);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,         32);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,      8);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);
    setScreenSize(getImageSize(SCRSHOT_NAME));
    WIDTH = getScreenSize().w;    HEIGHT = getScreenSize().h;
    printf("Screen size detected as: (%d,%d)\n",getScreenSize().w,getScreenSize().h);
    loadGame();
    running = true;
    startFPSCounter();
    startInputListener();
    startAutomaticFiring();
    while(running) {
        drawScreen(screen,h++);
    }
    SDL_Quit();
    return 0;
}
