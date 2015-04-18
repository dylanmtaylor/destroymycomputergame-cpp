#include "common.h"
#include <mutex>
std::mutex damageMutex;
std::mutex fpsCounterMutex;
std::mutex flamethrowerMutex;
const int maximumFlames = 100;
const int maximumDamage = 3500;
bool npot = false; //whether or not non-power of two sizes are supported
bool fbos = false; //whether or not framebuffer objects are supported
bool alphaTestVisible;
GLuint alphaPNGTestTex;
GLint alphaPNGTestVertices[8];
GLuint currentTex;
GLuint screenTex;
GLuint rbhTex;
GLuint sgbhTex;
GLuint bhTex;
GLuint numberTexture;
GLuint flameTex;
const int cwOffsetX = -24;
const int cwOffsetY = -4;
using namespace std;
//int WIDTH, HEIGHT; //for tracking the size of the screen

#ifdef __linux__
char SCREENSHOT_NAME[] = "/tmp/screen.bmp"; //store the image file to /tmp.
#else
char SCREENSHOT_NAME[] = "screen.bmp";
#endif

const int fpsSpacing = 5;
Size getImageSize(const char* filename);
void setScreenVertices(int width, int height);
void setScreenTexture(const char* filename);
Size scrSize;
int currentFireFrame = 0;
vector<Flame> flames;
vector<Damage> damages;
int framesElapsed = 9999;
int WIDTH, HEIGHT;
const int weaponNamePadding[] = {
    109, //flamethrower
    140, //machine_gun
    75, //machine_shotgun
    192, //shotgun
    240 //rifle
};
SDL_Surface* weaponNameLabel[5];
GLuint weaponNameTextures[5];
GLint weaponNameVertices[5][8];
GLfloat weaponNameTexCoords[5][8];
GLfloat flameAnimationTexCoords[15][8];
GLint fpsVertices[10][8]; //there is no way the FPS counter will hit 10 digits, but _just_ in case...
GLfloat rasterizedNumCoords[12][8];
GLint screenVertices[8];
char framesPerSecond[10];
GLshort texCoords[] = {
    0,1, //upper left
    0,0, //lower left
    1,1, //upper right
    1,0 //lower right
};
GLshort bmpTexCoords[] = { //because bitmaps are stored 'upside-down'
    0,0,
    0,1,
    1,0,
    1,1
};
GLuint BMPToTexture(const char* filename);
GLuint PNGToTexture(const char* filename);
char FIRE_ANIMATION[] = "res/flameanim.png";
char RIFLE_HOLE[] = "res/riflehole.png";
char BULLET_HOLE[] = "res/bullethole.png";
char SHOTGUN_HOLE[] = "res/sghole.png";
char NUMBER_TEXTURE[] = "res/numbers.png"; //"res/numbers.png";

void resetGame() {
    lock_guard<mutex> lk(damageMutex);
    damages.clear();
    lock_guard<mutex> lkf(flamethrowerMutex);
    flames.clear();
}

void initializeGraphics() {
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1); //disable vsync
    setScreenVertices(WIDTH,HEIGHT);
    showGLInfo();
    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    println("Creating textures from bitmaps...");
    setScreenTexture(SCREENSHOT_NAME); //PNGToTexture("screen.png");
    #if testAlphaBlending
    alphaTestVisible = true;
    #endif
//    #if testAlphaBlending
        alphaPNGTestTex = PNGToTexture("res/test.png");
        Size aPNGSize = getImageSize("res/test.png");
        //we want this image centered on the screen. therefore, we need to calculate the vertices for this image.
        alphaPNGTestVertices = {
                (GLint)((WIDTH / 2) - (.5 * aPNGSize.w)), (GLint)((HEIGHT / 2) + (.5 * aPNGSize.h)),
                (GLint)((WIDTH / 2) - (.5 * aPNGSize.w)), (GLint)((HEIGHT / 2) - (.5 * aPNGSize.h)),
                (GLint)((WIDTH / 2) + (.5 * aPNGSize.w)), (GLint)((HEIGHT / 2) + (.5 * aPNGSize.h)),
                (GLint)((WIDTH / 2) + (.5 * aPNGSize.w)), (GLint)((HEIGHT / 2) - (.5 * aPNGSize.h))
        };
//    #else
//        println("The alpha blending test is disabled.");
//    #endif
    rbhTex = PNGToTexture(RIFLE_HOLE);
    bhTex = PNGToTexture(BULLET_HOLE);
    sgbhTex = PNGToTexture(SHOTGUN_HOLE);
    numberTexture = PNGToTexture(NUMBER_TEXTURE);
    println("Creating flame animation texture...");
    flameTex = PNGToTexture(FIRE_ANIMATION);
    println("Loading and mapping current weapon name bitmaps...");
    vector<string> weaps = getDirectoryContents("res/weaps");
    println("weapons directory contents received. beginning loading/mapping.");
    int wtop, wbottom, wleft, wright, wpad;
    for (int weapon = 0; weapon < weaps.size(); weapon++) {
            printf("current weapon: %s (%d of %d)\n",weaps[weapon].c_str(),weapon,weaps.size());
            weaponNameLabel[weapon] = (SDL_Surface*) IMG_Load(weaps[weapon].c_str());
            weaponNameTextures[weapon] = (GLuint) PNGToTexture(weaps[weapon].c_str());
            wtop = (HEIGHT + cwOffsetY);
            wpad = (weaponNamePadding[weapon]);
            wbottom = ((HEIGHT - weaponNameLabel[weapon]->h) + cwOffsetY);
            wleft = (WIDTH - weaponNameLabel[weapon]->w + cwOffsetX) + wpad;
            wright = (WIDTH + cwOffsetX) + wpad;
            printf("Left: %d; Right: %d; Top: %d; Bottom: %d; padding: %d\n",wleft,wright,wtop,wbottom,wpad);
            //printf("bmap size: (%d,%d)\n",bmap->w,bmap->h);
            weaponNameVertices[weapon] = { //in theory, should equal screen vertices
                wleft,wtop,
                wleft,wbottom,
                wright,wtop,
                wright,wbottom
            };
            weaponNameTexCoords[weapon] = {
                (float)0, (float)0,
                (float)0, (float)1,//(float)(weaponNameLabel[weapon]->h / nearestPowTwo(weaponNameLabel[weapon]->h)),
                (float)1,(float)0,//(float)(weaponNameLabel[weapon]->w / nearestPowTwo(weaponNameLabel[weapon]->w)),(float) 0,
                (float)1,(float)1};//(float)(weaponNameLabel[weapon]->w / nearestPowTwo(weaponNameLabel[weapon]->w)), (float)(weaponNameLabel[weapon]->h / nearestPowTwo(weaponNameLabel[weapon]->h))};
    }
    println("Loading weapon name bitmaps complete!");
    println("Mapping rasterized number coordinates...");
    int rev;
    float rleft, rright;
    float rsize = (float)(TRUE_RASTERIZED_NUM_WIDTH / 10);
    for (int n = 9; n >= 0; n--) { //9 to 0
        printf("Mapping coordinates for number %d:\n",n);
        printf("(TRUE WIDTH / 10)[%.0f] * number[%d] : %f\n",rsize,n,(float)(rsize * (n)));
        rleft = (float)((float)(n * ((float)TRUE_RASTERIZED_NUM_WIDTH / 10)) / (float)512);
        rright = (float)((float)((n + 1) * ((float)TRUE_RASTERIZED_NUM_WIDTH / 10)) / (float)512);
        printf("Left: %.05f; Right: %.05f; Top: %d; Bottom: %d\n",(float)rleft,(float)rright,1,0);
        printf("Checking endpoint logic : rleft*512 = %.0f ; rright*512 = %.0f\n", (float)(rleft*512), (float)(rright*512));
        rasterizedNumCoords[(9-n)] = {
            (GLfloat)rleft, (GLfloat)0,        //lower left
            (GLfloat)rleft, (GLfloat)1,        //upper left
            (GLfloat)rright, (GLfloat)0,  //lower right
            (GLfloat)rright, (GLfloat)1   //upper right
        };
    }
    GLint vleft,vright,vtop,vbottom;
    for (int v = 0; v < arraySize(fpsVertices); v++) {
        printf("Mapping FPS vertex %d:\n", v);
        vleft = (GLint) ((2 * fpsSpacing) + ((fpsSpacing + (TRUE_RASTERIZED_NUM_WIDTH / 10)) * (v)));
        vright = (GLint)(vleft + (TRUE_RASTERIZED_NUM_WIDTH / 10));
        vtop = (GLint)(1.5 * fpsSpacing);
        vbottom = (GLint)(vtop + TRUE_RASTERIZED_NUM_HEIGHT);
        printf("Left: %d; Right: %d; Top: %d; Bottom: %d\n",vleft,vright,vtop,vbottom);
        fpsVertices[v] = {
                vleft, vtop,
                vleft, vbottom,
                vright, vtop,
                vright, vbottom};
    }
    println("Number coordinates mapped successfully!");
    println("Mapping flame animation texture coordinates...");
    for (int flame = 0; flame < 15; flame++) {
        flameAnimationTexCoords[flame] = {
            (float)(((float)flame * ((float)TRUE_FLAME_ANIMATION_WIDTH / (float)15)) / 1024), (float)0,
            (float)(((float)flame * ((float)TRUE_FLAME_ANIMATION_WIDTH / (float)15)) / 1024), (float)1,
            (float)(((float)(flame + 1) * ((float)TRUE_FLAME_ANIMATION_WIDTH / (float)15)) / 1024), (float)0,
            (float)(((float)(flame + 1) * ((float)TRUE_FLAME_ANIMATION_WIDTH / (float)15)) / 1024), (float)1
        };
    }
    println("Flame animation coordinates mapped successfully!");
}

void getScreenshot(string filename) {
    //probably a better way to do this. temp solution.
    #if defined(__linux__) //if this code is running on a Linux-based operating system
        if (fileExists("/usr/bin/scrot")) { //if 'scrot' is installed
            //use 'scrot' to take a screenshot ... one of the best screenshot programs ever made, in my opinion
            const char* cmdargs = ("/usr/bin/scrot -q 100 -d 0 " + filename).c_str();
            println(cmdargs);        system(cmdargs);
        } else if (fileExists("/bin/import")) {
            //use ImageMagick's 'import' command to get a screenshot.
            //Note: import seems to have problems with transparency and compositioning.
            const char* cmdargs = ("/bin/import " + filename).c_str();
            println(cmdargs);        system(cmdargs);
        } else if (fileExists("/usr/bin/xwd")) {
            //This is just about as reliable than ImageMagick's 'import' command.
            if (fileExists("/usr/bin/convert")) {
                //uses ImageMagick's 'convert' command and output redirection
                const char* cmdargs = ("/usr/bin/xwd -root | convert xwd:- " + filename).c_str();
                println(cmdargs);        system(cmdargs);
            } else if (fileExists("/usr/bin/xwdtopnm") && fileExists("/usr/bin/pnmtopng")) {
                //Attempt to rely on Netpbm. From my experience, this works _TERRIBLY_.
                //At this point, this is practically a last resort
                const char* cmdargs = ("/usr/bin/xwd -root | xwdtopnm | pnmtopng > " + filename).c_str();
                println(cmdargs);        system(cmdargs);
            }
        } else {
            println("No usable screenshot utility was detected. Please install a compatible screen capture program, such as \"scrot\".");
            exit(1);
        }
    #elif defined(_WIN32) //we're on windows.
        ScreenCapture(filename); //handle this in another method
    #else
        println("Incompatible Operating System. Currently only Windows and Linux are supported. Sorry!");
        exit(99999); //we're on an incompatible operating system. not much we can do here.
    #endif
}

void setScreenVertices(int WIDTH, int HEIGHT) {
    screenVertices = {     //calculate screen vertices
    0,HEIGHT,
    0,0,
    WIDTH,HEIGHT,
    WIDTH,0};
}

void setScreenTexture(const char *scrShotName) {
    screenTex = BMPToTexture(scrShotName);
}

void setScreenSize(Size s) {
    scrSize = s;
}

Size getScreenSize() {
    return scrSize;
}

SDL_Surface *loadBMP(const char *filename) {
    //function written by Jeff Molofee
    Uint8 *rowhi, *rowlo;
    Uint8 *tmpbuf, tmpch;
    SDL_Surface *image;
    int i, j;

    image = SDL_LoadBMP(filename);
    if ( image == NULL ) {
        fprintf(stderr, "Unable to load %s: %s\n", filename, SDL_GetError());
        return(NULL);
    }

    /* GL surfaces are upside-down and RGB, not BGR :-) */
    tmpbuf = (Uint8 *)malloc(image->pitch);
    if ( tmpbuf == NULL ) {
        fprintf(stderr, "Out of memory\n");
        return(NULL);
    }
    rowhi = (Uint8 *)image->pixels;
    rowlo = rowhi + (image->h * image->pitch) - image->pitch;
    for ( i=0; i<image->h/2; ++i ) {
        for ( j=0; j<image->w; ++j ) {
            tmpch = rowhi[j*3];
            rowhi[j*3] = rowhi[j*3+2];
            rowhi[j*3+2] = tmpch;
            tmpch = rowlo[j*3];
            rowlo[j*3] = rowlo[j*3+2];
            rowlo[j*3+2] = tmpch;
        }
        memcpy(tmpbuf, rowhi, image->pitch);
        memcpy(rowhi, rowlo, image->pitch);
        memcpy(rowlo, tmpbuf, image->pitch);
        rowhi += image->pitch;
        rowlo -= image->pitch;
    }
    free(tmpbuf);
    return(image);
}

/** Load Bitmaps And Convert To Textures
    Note: BMP files do NOT support alpha transparency.
*/
GLuint BMPToTexture(const char *filename) {

    GLuint texture;
    SDL_Surface *image = loadBMP(filename);     // Load Texture

    if (!image) {
        printf("failed to load bitmap image: %s\n", filename);
        SDL_Quit(); exit(1);
    }

    // Create Texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);   // 2d texture (x and y size)

    //Set up scaling
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image->w, image->h, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
    return texture;
};

Size getImageSize(const char *filename) {
    SDL_Surface *image = IMG_Load(filename);
    Size s = (Size){image->w,image->h};
    SDL_FreeSurface(image);
    return s;
}

GLuint PNGToTexture(const char *filename) {

    GLuint texture;
    SDL_Surface *image = IMG_Load(filename);  // Load Texture
    if (!image) {
        printf("failed to load png image: %s\n", filename);
        SDL_Quit(); exit(1);
    }

    // Create Texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);   // 2d texture (x and y size)

    //Set up scaling
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);


   if (image->format->BytesPerPixel == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
    } else {
        // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,
        // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
    }
    return texture;
}

void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    Uint32 *pixmem32;
    Uint32 colour;
    colour = SDL_MapRGB( screen->format, r, g, b );
    pixmem32 = (Uint32*) screen->pixels  + y + x;
    *pixmem32 = colour;
}

void showGLInfo() {
    printf("Destroy My Computer, ported to SDL and Linux!\n");
    printf("Written by Dylan Taylor and Scott Ketelaar\n");
    printf("Ported to Linux and C++0x by Dylan Taylor\n");
    printf("OpenGL Vendor: %s\n",(char *)glGetString(GL_VENDOR));
    printf("OpenGL Version: %s\n",(char *)glGetString(GL_VERSION));
    printf("OpenGL Renderer: %s\n", (char *)glGetString(GL_RENDERER));
    const char* slv = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (slv != NULL) printf("OpenGL Shading Language Version: %s\n", slv);
    const char* extensions = (char *)glGetString(GL_EXTENSIONS);
    const char* npotExtension = "GL_ARB_texture_non_power_of_two";
    const char* anisoExtension = "texture_filter_anisotropic";
    const char* fboExtension = "GL_EXT_framebuffer_object";
#if showSupportedOpenGLExtensions
    println(extensions);
#endif
#if forcedPowerOfTwoTextureScaling
    println("NOTE: Non-Power of Two Texture Sizes are DISABLED. Textures will need to be scaled.");
#else
    npot = strstr(extensions, npotExtension);
    printf("Non-Power of Two Texture Sizes: %sSupported\n",(npot ? "" : "NOT "));
#endif
    const bool aniso = strstr(extensions, anisoExtension);
    printf("Anisotropic Filtering: %s Supported",(aniso ? "" : "NOT "));
    if (aniso) {
        float maxAnisotropy;
        glGetFloatv(GL_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        printf(" (%dX)\n",(int)maxAnisotropy);
    } else {
        println("");
    }
    fbos = strstr(extensions,fboExtension);
    printf("Frame Buffer Objects (GL_EXT_framebuffer_object): %sSupported\n",(fbos ? "" : "NOT "));
}


void drawFloatCoordinateMapped2DObject(GLint vertices[], GLuint texture, GLfloat coords[]) {
    if (currentTex != texture) {
        glBindTexture(GL_TEXTURE_2D,texture);
        currentTex = texture;
    }
    glVertexPointer(2,GL_INT,0,vertices);
    glTexCoordPointer(2,GL_FLOAT,0,coords);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    int err = glGetError();
    if (err != 0) {
        printf("OpenGL error occurred: %d",err);
    }
}

void drawShortCoordinateMapped2DObject(GLint vertices[], GLuint texture, GLshort coords[]) {
    if (currentTex != texture) {
        glBindTexture(GL_TEXTURE_2D,texture);
        currentTex = texture;
    }
    glVertexPointer(2,GL_INT,0,vertices);
    glTexCoordPointer(2,GL_SHORT,0,coords);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    int err = glGetError();
    if (err != 0) printf("OpenGL error occurred: %d",err);
}

int intlen(float start) {
    int end = 0;
    while(start >= 1) {
        start /= 10;
        end++;
    }
    return end;
}

void drawEntire2DObject(GLint vertices[], GLuint texture) {
    drawShortCoordinateMapped2DObject(vertices, texture, texCoords);
}

void drawDamages() {
    lock_guard<mutex> lk(damageMutex);
    //first draw the damage
    for(vector<Damage>::iterator iter = damages.begin(); iter != damages.end(); ++iter) {
        drawEntire2DObject((iter)->getVertices(),(iter)->getTextureID());
//        printf("drawing damage with texture id %d at l: %d, r: %d, t: %d, b: %d\n",(iter)->getTextureID(), (iter)->getVertices()[0],
//               (iter)->getVertices()[4], (iter)->getVertices()[1], (iter)->getVertices()[3]);
    }
    lock_guard<mutex> lkf(flamethrowerMutex);
    //then draw the flames
    bool flamesDrawn = false; //used to prevent memory leaks by clearing the vector if no flames were drawn. also improves performance.
    for(vector<Flame>::iterator iter = flames.begin(); iter != flames.end(); ++iter) {
//                drawEntire2DObject((iter)->getVertices(),rbhTex);
        if (((iter)->getVertices()[0] > -64) && ((iter)->getVertices()[4] < WIDTH + 64) &&
        ((iter)->getVertices()[1] > -128) && ((iter)->getVertices()[3] < HEIGHT + 128)) {
                    drawFloatCoordinateMapped2DObject((iter)->getVertices(),flameTex,
                                          flameAnimationTexCoords[currentFireFrame]);
                    if (!flamesDrawn) flamesDrawn = true;
                    printf("drawing flame with texture id %d at l: %d, r: %d, t: %d, b: %d\n",rbhTex, (iter)->getVertices()[0],
               (iter)->getVertices()[4], (iter)->getVertices()[1], (iter)->getVertices()[3]);
        }
    }
    if (!flamesDrawn) flames.clear();
}

void drawFPS() {
    lock_guard<mutex> lk(fpsCounterMutex);
    #if testFramesPerSecond
        const char* fps = "0123456789";
    #else
        const char* fps = framesPerSecond;
    #endif
    char cCh;
    int cNum = 0;
    for (int num = 0; num < strlen(fps); num++) {
        cCh = fps[num];
        cNum = atoi(&cCh);
            drawFloatCoordinateMapped2DObject(fpsVertices[num], numberTexture, rasterizedNumCoords[cNum]);
    }
}

void drawScreen(SDL_Surface* screen, int h) {
    glClear(GL_COLOR_BUFFER_BIT);		// Clear The Screen Buffer
    glLoadIdentity();				// Reset The View
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    drawShortCoordinateMapped2DObject(screenVertices,screenTex,bmpTexCoords); //draw screen texture
    #if testAlphaBlending
    if (alphaTestVisible) drawEntire2DObject(alphaPNGTestVertices,alphaPNGTestTex);
//        printf("drawing alpha test with texture id %d at l: %d, r: %d, t: %d, b: %d\n",alphaPNGTestTex, alphaPNGTestVertices[0],
//                alphaPNGTestVertices[4], alphaPNGTestVertices[1], alphaPNGTestVertices[3]);
    #endif
    drawDamages();
    #if showFramesPerSecond
        drawFPS();
    #endif
    int cwIndex = getCurrentWeapon() - 9002;
    drawShortCoordinateMapped2DObject(weaponNameVertices[cwIndex], weaponNameTextures[cwIndex], texCoords);
    framesElapsed++;
    SDL_GL_SwapBuffers();
}

void updateFPS() {
    lock_guard<mutex> lk(fpsCounterMutex);
    sprintf(framesPerSecond,"%d", framesElapsed);
    printf("Frames Per Second: %s\n", framesPerSecond);
    framesElapsed = 0;
}

void fpsCounter() {
    println("Starting FPS counter...");
    while (true) {
        updateFPS();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void startFPSCounter() {
    thread t(fpsCounter);
    t.detach();
}

void toggleFullscreen()// Toggle Fullscreen/Windowed
//works only on *nix right now
{
    SDL_Surface *S; // a surface to point the screen
    S = SDL_GetVideoSurface(); // gets the video surface
    if(!S || (SDL_WM_ToggleFullScreen(S)!=1)) { // If SDL_GetVideoSurface Fails, Or if cant toggle to fullscreen
        printf("Unable to toggle fullscreen: %sn", SDL_GetError() );
        // only reporting the error, not exiting
    }
}

void addDamage(GLuint texture, Point pt, Size s) {
    lock_guard<mutex> lk(damageMutex);
    Damage d(texture, pt, s);
    damages.push_back(d);
//    while (damages.size() > maximumDamage) {
//        damages.erase(damages.begin());
//        damages.shrink_to_fit();
//    }
}

void toggleAlphaVisible() {
    alphaTestVisible = !alphaTestVisible;
}

void fireShotgun(Point c) {
    int sgRangeX = floor((float)WIDTH / (float)4);
    int sgRangeY = floor((float)HEIGHT / (float)4);
    //calculate range
    int left = (c.x - (sgRangeX / 2));
    int right = (c.x + (sgRangeX / 2));
    int top = (c.y - (sgRangeY / 2));
    int btm = (c.y + (sgRangeY / 2));
    for (int shot = 0; shot < sgShots; shot++) {
        printf("Firing shotgun... %d of %d\n",(shot+1),(sgShots));
        addDamage(sgbhTex, (Point) {randomInt(left,right),randomInt(top,btm)}, shotgunBulletHoleSize);
    }
}

void addFlame(Point startingLocation, Size flameSize, int deltaX, int deltaY) {
    printf("Creating flame; Delta X: %d, Delta Y: %d\n",deltaX,deltaY);
    lock_guard<mutex> lk(flamethrowerMutex);
    Flame f(startingLocation, flameSize, deltaX, deltaY);
    flames.push_back(f);
    //because flames can cause a (very) high CPU load, we need to limit how many are on the screen at once.
//    while (flames.size() > maximumFlames) {
//        flames.erase(flames.begin());
//        flames.shrink_to_fit();
//    }
}

void flameOn(Point e) {
    vector<Point> circlePoints = getPointsOnCircle(e, randomInt(ftMinRad,ftMaxRad), ftCirclePts);
    #if useFlamethrowerBurstPoints
       vector<Point> burstPoints;
    #endif
    for(vector<Point>::iterator citer = circlePoints.begin(); citer != circlePoints.end(); ++citer) {
        #if useFlamethrowerBurstPoints
            burstPoints = getPointsOnCircle((Point)*citer, ftBurstRad, ftBurstPts);
            for(vector<Point>::iterator biter = burstPoints.begin(); biter != burstPoints.end(); ++biter) {
                addFlame((Point)*biter, flameSize,
                     (((biter)->x = (citer)->x) ? 0 : (((biter)->x < (citer)->x) ? deltaD : deltaI)), //X delta
                     (((biter)->y = (citer)->y) ? 0 : (((biter)->y < (citer)->y) ? deltaD : deltaI)) //Y delta
                );
            }
        #else
            addFlame((Point)*citer, flameSize,
                 (((citer)->x = e.x) ? 0 : (((citer)->x < e.x) ? deltaD : deltaI)), //X delta
                 (((citer)->y = e.y) ? 0 : (((citer)->y < e.y) ? deltaD : deltaI)) //Y delta
            );
        #endif
    }
}


void advanceFireFlames() {
    while (isRunning()) currentFireFrame = ((currentFireFrame == 14) ? 0 : (currentFireFrame + 1));
}

void automaticWeaponFire() {
    Point e;
    while (isRunning()) {
        if (isFiring()) { //because we don't want to do this if we're not currently firing the weapon
            println("Automatically firing weapon.");
            //printf("Size of damage vector: %d\n", damages.size());
            e = getMouseCoords();
            switch (getCurrentWeapon()) {
                case FLAMETHROWER:
                    flameOn(e);
                    printf("Size of flame vector: %d\n", flames.size());
                    break;
                case MACHINE_SHOTGUN:
                    fireShotgun(e);
                    break;
                case MACHINE_GUN:
                    addDamage(bhTex, e, shotgunBulletHoleSize);
                    break;
                default:
                    println("Attempted to use automatic weapon fire method on an invalid weapon.");
                    return;
            }
            this_thread::sleep_for(chrono::microseconds(autoFiringDelay));
        }
    }
}

void fireWeapon() {
    println("Firing weapon!");
    switch(getCurrentWeapon()) {
        case NONE:
            println("Can't fire weapon when no weapon is selected.");
            return;
        case RIFLE:
            println("Firing rifle...");
            addDamage(rbhTex, getMouseCoords(), rifleBulletHoleSize);
            return;
        case SHOTGUN:
            fireShotgun(getMouseCoords());
        default:
            setFiring(true);
            return;
    }
}

void startAutomaticFiring() {
    thread t(automaticWeaponFire);
    t.detach();
}
