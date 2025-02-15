#include "structs.h"
#include "gameObject.h"
#include "hero.h"
#include "basicEnemy.h"
#include "platform.h"
#include "elevator.h"
#include "spike.h"
#include "room.h"
#include "ppm.h"
#include "game.h"
#include "collisions.h"
#include "jasonC.h"
#include "fernandoV.h"
//=======
#include "markS.h"
#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 700

#include "fonts.h"

#define GRAVITY -0.35
#define MAXBUTTONS 4

using namespace std;

void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void cleanupImages(void);

void set_title(void);
void renderBackground(GLuint);
void init_MainMenuButtons(void);
void render_MainMenu(void);
void check_menu_button(XEvent *e, Game * game);
void check_game_input(XEvent *e, Game * game);
void check_death_input(XEvent *e, Game *newgame);

void moveHero(XEvent *e, Game * game);
void movePlatform(XEvent *e, Game * game);
void moveSavePoint(XEvent *e, Game * game);
void moveSpike(XEvent *e, Game * game);
void moveElevator(XEvent *e, Game * game);
void moveEnemy(XEvent *e, Game * game);
void physics(Game * game);

void render_game(Game* game);
void renderEnemy(GameObject *);
void renderBullet(GameObject *, int);
void renderSpike(GameObject *);
void renderPlatform(GameObject *);
void renderSavePoint(GameObject *, int);
//void renderElevator(GameObject *);
void renderElevatorShadow(Elevator *, int[3], float);

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

Button button[MAXBUTTONS];
int nbuttons=0;

GameState g_gamestate = MAIN_MENU;

int numCollisions;

struct timeval Gthrottle;
long long microseconds;
long long afterDeath = 0;
int GoldMilliSec = 0;
int GtimeLapse = 0;
int Gthreshold = 15000;
//int Gthreshold = 15;
//Variable that is used count the number of renders...
int renderNum = 0;
//Following Declarations are for  Image importing...
unsigned char *buildAlphaData(Ppmimage *img);
void renderHero(GLuint heroTexture,Game* game  ,Coordinates* heroSprite,int index,int w, int h, int mod);
void setUpImage (GLuint texture, Ppmimage *picture);
void convertToRGBA(Ppmimage *picture); 
void renderTexture(GLuint imageTexture, float x1,float x2,float y1, float y2, int width, int height);
GLuint getBMP(const char *path);
Ppmimage *spikeDeathImage = NULL;
Ppmimage *eShootingRightImage = NULL;
Ppmimage *eShootingLeftImage = NULL;
Ppmimage *bulletImage = NULL;
Ppmimage *keyImage = NULL;
Ppmimage *spikeEnemyRightImage = NULL;
Ppmimage *spikeEnemyLeftImage = NULL;
Ppmimage *heroDeathImage = NULL;
Ppmimage *checkPointImage = NULL;
Ppmimage *idleLeftImage = NULL;
Ppmimage *guiBackgroundImage = NULL;
Ppmimage *jumpRightImage = NULL;
Ppmimage *jumpLeftImage = NULL;
Ppmimage *walkLeftImage = NULL;
Ppmimage *walkRightImage = NULL;
Ppmimage *rockImage = NULL;
Ppmimage *mainMenuButtonsEditImage = NULL;
Ppmimage *idleRightImage = NULL;
Ppmimage *backgroundImage = NULL;
Ppmimage *mainMenuButtonsImage = NULL;
Ppmimage *mainMenuButtonsExitImage = NULL;
Ppmimage *spikeImage = NULL;
Ppmimage *deadMessageImage = NULL;
Ppmimage *elevatorImage = NULL;
Ppmimage *coalImage = NULL;
Ppmimage *woodImage = NULL;
Ppmimage *shooterDeathImage = NULL;
Ppmimage *dirtImage = NULL;

//Creating the Textures
GLuint shooterDeathTexture;
GLuint spikeDeathTexture;
GLuint eShootingRightTexture;
GLuint eShootingLeftTexture;
GLuint spikeTexture;
GLuint keyTexture;
GLuint bulletTexture;
GLuint checkPointTexture;
GLuint spikeEnemyRightTexture;
GLuint spikeEnemyLeftTexture;
GLuint deadMessageTexture;
GLuint heroDeathTexture;
GLuint idleLeftTexture;
GLuint guiBackgroundTexture;
GLuint mainMenuButtonsEditTexture;
GLuint platformTextures[5];
GLuint idleRightTexture;
GLuint jumpRightTexture;
GLuint jumpLeftTexture;
GLuint walkRightTexture;
GLuint walkLeftTexture;
GLuint forestTexture;
GLuint mainMenuButtonsTexture;
GLuint mainMenuButtonsExitTexture;
bool forestBackgroundSet=true;
CharacterState prevPosition;
int numAnimation = 0;
int spikeAnimation = 0;
int bulletAnimation = 0;
int shooterAnimation = 0;
Room *savePointRoom;
int currentSavePoint;
auto start = std::chrono::high_resolution_clock::now();
//End
//value for save point sound
bool Scollision = false;

GameObject mouse;

int main()
{
    initXWindows();
    init_opengl();
    init_MainMenuButtons();
    //Game newgame();  //says newgame is non-class type 'Game()'
    Game newgame;
    newgame.hero = new Hero();
//    newgame.respawnAtSavePoint();

	//initialize openAL and menu music
	init_openal();
	playMenuMusic();

    bool render = true;
    bool doPhysics = true;

    while(g_gamestate != EXIT_GAME) {
        gettimeofday(&Gthrottle, NULL);
        GtimeLapse = (Gthrottle.tv_usec >= GoldMilliSec) ? Gthrottle.tv_usec - GoldMilliSec :
            (1000000 - GoldMilliSec) + Gthrottle.tv_usec;
        if (GtimeLapse >= Gthreshold){
            GoldMilliSec = Gthrottle.tv_usec;
            render = true;
        }
        else{
            render = false;
        }
 
        switch (g_gamestate) {
            case MAIN_MENU:
	//initialize menu music
	//playMenuMusic();
                while(XPending(dpy)) {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    check_menu_button(&e,&newgame);
                }
                if (render == true){
                    render_MainMenu();
                    glXSwapBuffers(dpy, win);
                }
                break;
            case PLAYING:
                while(XPending(dpy)) {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    if ( newgame.hero->state != DEATH) {
                        check_game_input(&e, &newgame);
                    }
                    else {
                        check_death_input(&e, &newgame);
                    }
                }
                if (doPhysics == true){
                    physics(&newgame);
                    doPhysics = false;
                }
                if (render == true){
                    doPhysics = true;
                    render_game(&newgame);
                    glXSwapBuffers(dpy, win);
                }
                break;
            case LEVEL_EDITOR:
                while(XPending(dpy)) {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    if(!(newgame.hero->state == DEATH) ) {
                        check_game_input(&e, &newgame);
                    }
                    else {
                        check_death_input(&e, &newgame);
                    }
                    if (newgame.isPlatformMovable == true)
                        movePlatform(&e, &newgame);
                    if (newgame.isSpikeMovable == true)
                        moveSpike(&e, &newgame);
                    if (newgame.isSavePointMovable == true)
                        moveSavePoint(&e, &newgame);
                    if (newgame.isElevatorMovable == true)
                        moveElevator(&e, &newgame);
                    if (newgame.isEnemyMovable == true)
                        moveEnemy(&e, &newgame);
                    if (newgame.isHeroMovable == true)
                        moveHero(&e, &newgame);
                }
                if (doPhysics == true){
                    if (!newgame.isHeroMovable)
                        physics(&newgame);
                    doPhysics = false;
                }
                if (render == true){
                    doPhysics = true;
                    render_game(&newgame);
                    glXSwapBuffers(dpy, win);
                }
                break;
            case MAP:
                while(XPending(dpy)) {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    newgame.checkMapInput(&e);
                }
                if (render == true){
                    newgame.renderMap(dpy, &win);
                    glXSwapBuffers(dpy, win);
                }

                break;
            case CREDITS:
                while(XPending(dpy))
                {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    newgame.checkCreditsInput(&e);
                }
                if (render == true)
                {
                    newgame.playCredits();
                    glXSwapBuffers(dpy, win);
                }
                break;
    
            case EXIT_GAME:
                break;
            default:
                break;
        }
    }
    cleanupImages();
    #ifdef NVIDIA
    glXDestroyContext(dpy, glc);
    #else
    glXMakeCurrent(dpy, win, NULL);
    glXDestroyContext(dpy, glc);
    #endif
    cleanupXWindows();
	//close audio devices
	closeDevices();
    return 0;
}

void initXWindows(void)
{
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
    XFree(vi);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    initialize_fonts();
    //Importing Images
    spikeDeathImage = ppm6GetImage("./images/spikeDead.ppm");
    eShootingRightImage = ppm6GetImage("./images/mega_walkR.ppm");
    eShootingLeftImage = ppm6GetImage("./images/mega_walkL.ppm");
    keyImage = ppm6GetImage("./images/key.ppm");
    spikeEnemyRightImage = ppm6GetImage("./images/enemy_spike_right.ppm");
    spikeEnemyLeftImage = ppm6GetImage("./images/enemy_spike_left.ppm");
    deadMessageImage = ppm6GetImage("./images/dieStatement.ppm");
    checkPointImage = ppm6GetImage("./images/checkpoint.ppm");
    idleRightImage = ppm6GetImage("./images/IdleR.ppm");
    idleLeftImage = ppm6GetImage("./images/IdleL.ppm");
    heroDeathImage = ppm6GetImage("./images/dying1.ppm");
    mainMenuButtonsEditImage = ppm6GetImage("./images/Leveleditor.ppm");
    backgroundImage = ppm6GetImage("./images/Background1.ppm");
    rockImage = ppm6GetImage("./images/Rock.ppm");
    mainMenuButtonsImage = ppm6GetImage("./images/start.ppm");
    guiBackgroundImage = ppm6GetImage("./images/GuiBackground.ppm");
    mainMenuButtonsExitImage = ppm6GetImage("./images/exit.ppm");
    jumpRightImage = ppm6GetImage("./images/jumpingGun.ppm");
    jumpLeftImage = ppm6GetImage("./images/jumpingLeft.ppm");
    walkRightImage = ppm6GetImage("./images/HeroWalkRight.ppm");
    walkLeftImage = ppm6GetImage("./images/heroWalkLeft.ppm");
    spikeImage = ppm6GetImage("./images/spike2.ppm");
    bulletImage = ppm6GetImage("./images/bullet.ppm");
    elevatorImage = ppm6GetImage("./images/elevator.ppm");
    woodImage = ppm6GetImage("./images/wood.ppm");
    coalImage = ppm6GetImage("./images/coal.ppm");
    dirtImage = ppm6GetImage("./images/dirt.ppm");
    shooterDeathImage = ppm6GetImage("./images/shooterDeath.ppm");

    //Binding the textures... 
    glGenTextures(1, &keyTexture); 
    glGenTextures(1, &jumpLeftTexture); 
    glGenTextures(1, &jumpRightTexture);
    glGenTextures(1, &walkRightTexture);
    glGenTextures(1, &walkLeftTexture);
    glGenTextures(1, &idleRightTexture);
    glGenTextures(1, &mainMenuButtonsEditTexture);
    glGenTextures(1, &forestTexture);
    //The following is an array of platforms
    glGenTextures(1, &platformTextures[0]);
    glGenTextures(1, &platformTextures[1]);
    glGenTextures(1, &platformTextures[2]);
    glGenTextures(1, &platformTextures[3]);
    glGenTextures(1, &platformTextures[4]);
    glGenTextures(1, &mainMenuButtonsTexture);
    glGenTextures(1, &guiBackgroundTexture);
    glGenTextures(1, &mainMenuButtonsExitTexture);
    glGenTextures(1, &idleLeftTexture);
    glGenTextures(1, &heroDeathTexture);
    glGenTextures(1, &spikeTexture);
    glGenTextures(1, &deadMessageTexture);
    glGenTextures(1, &checkPointTexture);
    glGenTextures(1, &spikeEnemyRightTexture);
    glGenTextures(1, &spikeEnemyLeftTexture);
    glGenTextures(1, &bulletTexture);
    glGenTextures(1, &eShootingRightTexture);
    glGenTextures(1, &eShootingLeftTexture);
    glGenTextures(1, &spikeDeathTexture);
    glGenTextures(1, &shooterDeathTexture);
    

    //Setting up the shooter death texture
    setUpImage(shooterDeathTexture,shooterDeathImage);
    convertToRGBA(shooterDeathImage);
    
    setUpImage(spikeDeathTexture,spikeDeathImage);
    convertToRGBA(spikeDeathImage);
    
    //Settinf up the sprite sheets for the shooter enemy.
    setUpImage(eShootingRightTexture,eShootingRightImage);
    convertToRGBA(eShootingRightImage);
    
    setUpImage(eShootingLeftTexture,eShootingLeftImage);
    convertToRGBA(eShootingLeftImage);
    
    //Setting up the key texture for the save Points
    setUpImage(keyTexture,keyImage);
    convertToRGBA(keyImage);
    
    //Setting up the bullet image/texture
    setUpImage(bulletTexture,bulletImage);
    convertToRGBA(bulletImage);
    //Setting up the spike enemy left image
    setUpImage(spikeEnemyLeftTexture,spikeEnemyLeftImage);
    convertToRGBA(spikeEnemyLeftImage);

    //Setting up the spike enemy right image
    setUpImage(spikeEnemyRightTexture,spikeEnemyRightImage);
    convertToRGBA(spikeEnemyRightImage);

    //Setting up the checkpoint texture
    setUpImage(checkPointTexture,checkPointImage);
    convertToRGBA(checkPointImage);

    //Setting up the hero textures
    setUpImage(idleRightTexture,idleRightImage);
    convertToRGBA(idleRightImage);

    //Setting up the dead Mssage texture
    setUpImage(deadMessageTexture,deadMessageImage);
    convertToRGBA(deadMessageImage);
    //Setting up the spike texture
    setUpImage(spikeTexture, spikeImage);
    convertToRGBA(spikeImage);

    //Settiing up the Death texture
    setUpImage(heroDeathTexture, heroDeathImage);
    convertToRGBA(heroDeathImage);

    //Setting up the Idle left texture
    setUpImage(idleLeftTexture,idleLeftImage);
    convertToRGBA(idleLeftImage);

    //Setting up the jumpRightImage
    setUpImage(jumpRightTexture,jumpRightImage);
    convertToRGBA(jumpRightImage);
    //Setting up the jumpLeft image
    setUpImage(jumpLeftTexture,jumpLeftImage);
    convertToRGBA(jumpLeftImage);
    //Setting up the walking Right texture
    setUpImage(walkRightTexture,walkRightImage);
    convertToRGBA(walkRightImage);
    //Setting up the walking Left texture
    setUpImage(walkLeftTexture,walkLeftImage);
    convertToRGBA(walkLeftImage);
    //Setting Up the Start button image
    setUpImage(mainMenuButtonsTexture,mainMenuButtonsImage);
    //Setting up the Level Editor button image texture....
    setUpImage(mainMenuButtonsEditTexture,mainMenuButtonsEditImage);
    //Setting up the ExitButton texture..
    setUpImage(mainMenuButtonsExitTexture,mainMenuButtonsExitImage);
    //Setting up the Rock Platforms Texture....
    setUpImage(platformTextures[0],rockImage);
    //Setting up the elevator texture
    setUpImage(platformTextures[1],elevatorImage);
    //Setting up the wood image 
    setUpImage(platformTextures[2],woodImage);
    convertToRGBA(woodImage);
    //Setting up the coal Image
    setUpImage(platformTextures[3],coalImage);
    //Setting up the dirt image
    setUpImage(platformTextures[4],dirtImage);
    //Setting up the background image
    setUpImage(forestTexture,backgroundImage);
    //Setting up the Gui Background image.
    setUpImage(guiBackgroundTexture,guiBackgroundImage);
}

// Need to clean up PPM images
void cleanupImages(void) {
    ppm6CleanupImage(keyImage);
    ppm6CleanupImage(spikeEnemyRightImage);
    ppm6CleanupImage(spikeEnemyLeftImage);
    ppm6CleanupImage(deadMessageImage);
    ppm6CleanupImage(checkPointImage);
    ppm6CleanupImage(idleRightImage);
    ppm6CleanupImage(idleLeftImage);
    ppm6CleanupImage(heroDeathImage);
    ppm6CleanupImage(mainMenuButtonsEditImage);
    ppm6CleanupImage(backgroundImage);
    ppm6CleanupImage(rockImage);
    ppm6CleanupImage(mainMenuButtonsImage);
    ppm6CleanupImage(guiBackgroundImage);
    ppm6CleanupImage(mainMenuButtonsExitImage);
    ppm6CleanupImage(jumpRightImage);
    ppm6CleanupImage(jumpLeftImage);
    ppm6CleanupImage(walkRightImage);
    ppm6CleanupImage(walkLeftImage);
    ppm6CleanupImage(spikeImage);
    ppm6CleanupImage(bulletImage);
    ppm6CleanupImage(eShootingRightImage);
    ppm6CleanupImage(eShootingLeftImage);
    ppm6CleanupImage(spikeDeathImage);
    ppm6CleanupImage(shooterDeathImage);
    ppm6CleanupImage(elevatorImage);
    ppm6CleanupImage(dirtImage);
    ppm6CleanupImage(woodImage);
    ppm6CleanupImage(coalImage);
}

void cleanupXWindows(void)
{
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "CSUB Professor Smash");
}

void init_MainMenuButtons(void)
{
  //initialize buttons...
  nbuttons=0;
  //size and position
    button[nbuttons].r.width = 100;
    button[nbuttons].r.height = 40;
    //button[nbuttons].r.left = 290;
    button[nbuttons].r.left = 290;
    button[nbuttons].r.bot = 450;
    //button[nbuttons].r.bot = 480;
    button[nbuttons].r.right = button[nbuttons].r.left + button[nbuttons].r.width;
    button[nbuttons].r.top = button[nbuttons].r.bot + button[nbuttons].r.height;
    button[nbuttons].r.centerx = (button[nbuttons].r.left + button[nbuttons].r.right) / 2;
    button[nbuttons].r.centery = (button[nbuttons].r.bot + button[nbuttons].r.top) / 2;
    //strcpy(button[nbuttons].text, "Reset");
    button[nbuttons].down = 0;
    button[nbuttons].click = 0;
    button[nbuttons].color[0] = 0.4f;
    button[nbuttons].color[1] = 0.4f;
    button[nbuttons].color[2] = 0.7f;
    button[nbuttons].dcolor[0] = button[nbuttons].color[0] * 0.5f;
    button[nbuttons].dcolor[1] = button[nbuttons].color[1] * 0.5f;
    button[nbuttons].dcolor[2] = button[nbuttons].color[2] * 0.5f;
    button[nbuttons].text_color = 0x00ffffff;
    nbuttons++;
    button[nbuttons].r.width = 100;
    button[nbuttons].r.height = 40;
    button[nbuttons].r.left = 290;
    button[nbuttons].r.bot = 380;
    button[nbuttons].r.right = button[nbuttons].r.left + button[nbuttons].r.width;
    button[nbuttons].r.top = button[nbuttons].r.bot + button[nbuttons].r.height;
    button[nbuttons].r.centerx = (button[nbuttons].r.left + button[nbuttons].r.right) / 2;
    button[nbuttons].r.centery = (button[nbuttons].r.bot + button[nbuttons].r.top) / 2;
    //strcpy(button[nbuttons].text, "Level Editor");
    button[nbuttons].down = 0;
    button[nbuttons].click = 0;
    button[nbuttons].color[0] = 0.3f;
    button[nbuttons].color[1] = 0.3f;
    button[nbuttons].color[2] = 0.6f;
    button[nbuttons].dcolor[0] = button[nbuttons].color[0] * 0.5f;
    button[nbuttons].dcolor[1] = button[nbuttons].color[1] * 0.5f;
    button[nbuttons].dcolor[2] = button[nbuttons].color[2] * 0.5f;
    button[nbuttons].text_color = 0x00ffffff;
    nbuttons++;
    button[nbuttons].r.width = 100;
    button[nbuttons].r.height = 40;
    button[nbuttons].r.left = 290;
    button[nbuttons].r.bot = 310;
    button[nbuttons].r.right = button[nbuttons].r.left + button[nbuttons].r.width;
    button[nbuttons].r.top = button[nbuttons].r.bot + button[nbuttons].r.height;
    button[nbuttons].r.centerx = (button[nbuttons].r.left + button[nbuttons].r.right) / 2;
    button[nbuttons].r.centery = (button[nbuttons].r.bot + button[nbuttons].r.top) / 2;
    //strcpy(button[nbuttons].text, "Quit");
    button[nbuttons].down = 0;
    button[nbuttons].click = 0;
    button[nbuttons].color[0] = 0.3f;
    button[nbuttons].color[1] = 0.3f;
    button[nbuttons].color[2] = 0.6f;
    button[nbuttons].dcolor[0] = button[nbuttons].color[0] * 0.5f;
    button[nbuttons].dcolor[1] = button[nbuttons].color[1] * 0.5f;
    button[nbuttons].dcolor[2] = button[nbuttons].color[2] * 0.5f;
    button[nbuttons].text_color = 0x00ffffff;
    nbuttons++;
}

void render_MainMenu(void)
{
    //Rect r;
    //glClear(GL_COLOR_BUFFER_BIT);
    //glColor3ub(200,200,200);
    glColor3f(1.0,1.0,1.0);

  glEnable(GL_TEXTURE_2D);
  renderBackground(guiBackgroundTexture);
  glDisable(GL_TEXTURE_2D);
  
  glPushMatrix();
  for (int i=0; i<nbuttons; i++) {
    
    if (button[i].over) {
      int w=2;
      glColor3f(1.0f, 1.0f, 0.0f);
      //Ba highlight around button
      glLineWidth(3);
      glBegin(GL_LINE_LOOP);
        glVertex2i(button[i].r.left-w,  button[i].r.bot-w);
        glVertex2i(button[i].r.left-w,  button[i].r.top+w);
        glVertex2i(button[i].r.right+w, button[i].r.top+w);
        glVertex2i(button[i].r.right+w, button[i].r.bot-w);
        glVertex2i(button[i].r.left-w,  button[i].r.bot-w);
      glEnd();
      glLineWidth(1);
    }
    
    if (button[i].down) {
      glColor3fv(button[i].dcolor);
    } else {
      glColor3fv(button[i].color);
    }
    glBegin(GL_QUADS);
      glVertex2i(button[i].r.left,  button[i].r.bot);
      glVertex2i(button[i].r.left,  button[i].r.top);
      glVertex2i(button[i].r.right, button[i].r.top);
      glVertex2i(button[i].r.right, button[i].r.bot);
    glEnd();
    //r.left = button[i].r.centerx;
    //r.bot  = button[i].r.centery-8;
    //r.center = 1;
    /*
    if (button[i].down) {
      ggprint16(&r, 0, button[i].text_color, "Pressed!");
    } else {
      ggprint16(&r, 0, button[i].text_color, button[i].text);
    }
    */
    glPopMatrix();
        //Rendering the menu Items
        if (i == 0) {
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();

            glTranslatef(button[i].r.centerx,button[i].r.centery, 0.0f);
            glBindTexture(GL_TEXTURE_2D,mainMenuButtonsTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(0.1f,.9f); glVertex2i(-50,-20);
            glTexCoord2f(0.1f,0.1f); glVertex2i(-50,20); //smenu
            glTexCoord2f(.9f,0.1f); glVertex2i(50,20);
            glTexCoord2f(.9f,.9f); glVertex2i(50,-20);
            glEnd();

            glPopMatrix();
            //BmainMenuButtonsTexture,0.1,0.9,0.1,0.9, 200, 30);
        }
        if(i == 1) {
            //This will be a call to the function to render.
            //BmainMenuButtonsExitTexture,0.1,0.9,0.1,0.9,200,30);
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();

            glTranslatef(button[i].r.centerx,button[i].r.centery, 0.0f);
            glBindTexture(GL_TEXTURE_2D,mainMenuButtonsEditTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(0.1f,.9f); glVertex2i(-50,-20);
            glTexCoord2f(0.1f,0.1f); glVertex2i(-50,20); //smenu
            glTexCoord2f(.9f,0.1f); glVertex2i(50,20);
            glTexCoord2f(.9f,.9f); glVertex2i(50,-20);
            glEnd();

            glPopMatrix();
        }
        if(i == 2) {
            //This will be a call to the function to render.
            //BmainMenuButtonsExitTexture,0.1,0.9,0.1,0.9,200,30);
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();

            glTranslatef(button[i].r.centerx,button[i].r.centery, 0.0f);
            glBindTexture(GL_TEXTURE_2D,mainMenuButtonsExitTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(0.1f,.9f); glVertex2i(-50,-20);
            glTexCoord2f(0.1f,0.1f); glVertex2i(-50,20); //smenu
            glTexCoord2f(.9f,0.1f); glVertex2i(50,20);
            glTexCoord2f(.9f,.9f); glVertex2i(50,-20);
            glEnd();

            glPopMatrix();
        }
    }
}

void movePlatform(XEvent *e, Game *game)
{
    Room * room = game->getRoomPtr();
    mouse.body.center[0] = e->xbutton.x;
    mouse.body.center[1] = WINDOW_HEIGHT - e->xbutton.y;
    mouse.body.width = room->platforms[game->movablePlatformIndex]->body.width;
    mouse.body.height = room->platforms[game->movablePlatformIndex]->body.height;
    bool isCollision = false;
    int collisionCount = 0;
    if (game->isPlatformMovable){
        for (int i = 0; i < room->numPlatforms; i++){
            if (i != game->movablePlatformIndex){
                isCollision = collisionRectRect(&mouse.body , &room->platforms[i]->body);
                if (isCollision == true) {
                    collisionCount++;
                    movablePlatformCollision(&mouse, room->platforms[i]);
                    room->platforms[game->movablePlatformIndex]->body.center[0] = mouse.body.center[0];
                    room->platforms[game->movablePlatformIndex]->body.center[1] = mouse.body.center[1];
                }
            }
        }
        if (collisionCount <= 0){

            room->platforms[game->movablePlatformIndex]->body.center[0] = e->xbutton.x;
            room->platforms[game->movablePlatformIndex]->body.center[1] = WINDOW_HEIGHT - e->xbutton.y;
        }
    }
}

void moveSavePoint(XEvent *e, Game *game)
{
    Room * currentRoom = game->getRoomPtr();
    currentRoom->savePoints[game->movableSavePointIndex]->body.center[0] = e->xbutton.x;
    currentRoom->savePoints[game->movableSavePointIndex]->body.center[1] = WINDOW_HEIGHT - e->xbutton.y;
}
void moveEnemy(XEvent *e, Game * game)
{
    Room * currentRoom = game->getRoomPtr();
    currentRoom->enemies[game->movableEnemyIndex]->body.center[0] = e->xbutton.x;
    currentRoom->enemies[game->movableEnemyIndex]->body.center[1] = WINDOW_HEIGHT - e->xbutton.y;
    
}
void moveHero(XEvent *e, Game * game)
{
    game->hero->body.center[0] = e->xbutton.x;
    game->hero->body.center[1] = WINDOW_HEIGHT - e->xbutton.y;
}
void moveSpike(XEvent *e, Game *game){
    Room * currentRoom = game->getRoomPtr();
    GameObject * currentSpike = currentRoom->spikes[game->movableSpikeIndex];
    Vec spike[3];
    spike[0][2] = 0;
    spike[1][2] = 0;
    spike[2][2] = 0;
    spike[0][0] = e->xbutton.x;
    spike[0][1] = WINDOW_HEIGHT - e->xbutton.y;
    if (currentSpike->body.orientation == FACING_UP){
        spike[1][0] = e->xbutton.x + 26;
        spike[1][1] = WINDOW_HEIGHT - e->xbutton.y;
        spike[2][0] = e->xbutton.x + 13;
        spike[2][1] = WINDOW_HEIGHT - e->xbutton.y + 22.981;
    }
    if (currentSpike->body.orientation == FACING_LEFT){
        spike[1][0] = e->xbutton.x;
        spike[1][1] = WINDOW_HEIGHT - e->xbutton.y + 26;
        spike[2][0] = e->xbutton.x - 22.981;
        spike[2][1] = WINDOW_HEIGHT - e->xbutton.y + 13;
    }
    if (currentSpike->body.orientation == FACING_DOWN){
        spike[1][0] = e->xbutton.x - 26;
        spike[1][1] = WINDOW_HEIGHT - e->xbutton.y;
        spike[2][0] = e->xbutton.x - 13;
        spike[2][1] = WINDOW_HEIGHT - e->xbutton.y - 22.981;
    }
    if (currentSpike->body.orientation == FACING_RIGHT){
        spike[1][0] = e->xbutton.x;
        spike[1][1] = WINDOW_HEIGHT - e->xbutton.y - 26;
        spike[2][0] = e->xbutton.x + 22.981;
        spike[2][1] = WINDOW_HEIGHT - e->xbutton.y - 13;
    }
    vecCopy(spike[0], currentRoom->spikes[game->movableSpikeIndex]->body.corners[0]);
    vecCopy(spike[1], currentRoom->spikes[game->movableSpikeIndex]->body.corners[1]);
    vecCopy(spike[2], currentRoom->spikes[game->movableSpikeIndex]->body.corners[2]);
}

void moveElevator(XEvent *e, Game * game)
{
    Room * currentRoom = game->getRoomPtr();
    Elevator * currentElevator = currentRoom->elevators[game->movableElevatorIndex];
    float mouseX = e->xbutton.x;
    float mouseY = WINDOW_HEIGHT - e->xbutton.y;
    currentElevator->setUpperLimit(
      (currentElevator->getUpperLimit() - currentElevator->getLowerLimit()) + mouseY);
    currentElevator->setLowerLimit(mouseY);
    currentElevator->body.center[0] = mouseX;
    currentElevator->body.center[1] = mouseY;
}

void check_menu_button(XEvent *e, Game * game)
{
  static int savex = 0;
  static int savey = 0;
  int i,x,y;
  int lbutton=0;
  //int rbutton=0;
  //
  if (e->type == ButtonRelease)
    return;
  if (e->type == ButtonPress) {
    if (e->xbutton.button==1) {
      //Left button is down
      lbutton=1;
    }
    //if (e->xbutton.button==3) {
      //Right button is down
      //rbutton=1;
    //}
  }
  x = e->xbutton.x;
  y = e->xbutton.y;
  y = WINDOW_HEIGHT - y;
  if (savex != e->xbutton.x || savey != e->xbutton.y) {
    //Mouse moved
    savex = e->xbutton.x;
    savey = e->xbutton.y;
  }
  for (i=0; i<nbuttons; i++) {
    button[i].over=0;
    if (x >= button[i].r.left &&
      x <= button[i].r.right &&
      y >= button[i].r.bot &&
      y <= button[i].r.top) {
      button[i].over=1;
      if (button[i].over) {
        if (lbutton) {
          switch(i) {
            case 0:
              g_gamestate = PLAYING;
              game->state = PLAYING;
			  stopMenuMusic();
              playGameMusic();
              break;
            case 1:
              g_gamestate = LEVEL_EDITOR;
              game->state = LEVEL_EDITOR;
			  stopMenuMusic();
              playGameMusic();
              break;
            case 2:
              g_gamestate = EXIT_GAME;
              break;
          }
        }
      }
    }
  }
  return;
}
/*
void check_death_input(XEvent *e,Game *game)
{
    if (e -> type ==KeyPress ) { 
        int key = XLookupKeysym(&e->xkey,0);
        if (key == XK_Escape){
            g_gamestate = MAIN_MENU;
        }
        if (key == XK_Return || key == XK_space) { 
            game->respawnAtSavePoint();
            renderNum = 0;
        }
    }
}*/

void physics(Game * game)
{
    bool isCollision = false;
    Room * room = game->getRoomPtr();
	CharacterState previous = game->hero->state;
    game->hero->movement();
    if (game->hero->delay != 0) {
        game->hero->delay = (game->hero->delay + 1) % 20;
    }
    for (auto entity : room->elevators) {
        entity->movement();
    }

    for (int i = 0; i < room->numPlatforms; i++) {
        isCollision = collisionRectRect(&game->hero->body, &room->platforms[i]->body);
        if (isCollision == true) {
            game->hero->onCollision(room->platforms[i]);
        }
    }

    isCollision = false;
    for(int i = 0; i < room->numElevators; i++) {
        isCollision = collisionRectRect(&game->hero->body, &room->elevators[i]->body);
        if (isCollision == true) {
            game->hero->onCollision(room->elevators[i]);
        }
    }

    isCollision = false;
    for (int i = 0; i < room->numSavePoints; i++) {
        isCollision = collisionRectRect(&game->hero->body, &room->savePoints[i]->body);
		//save point collision 
        if (isCollision == true) {
			if(Scollision == true) {
				playSavePoint();
				Scollision = false;
			}
            game->setSavePoint(i);
            currentSavePoint = i;
            savePointRoom = room;
            break;
        }
    }
	if(isCollision == false) {

		Scollision = true;

	}
    //if (isCollision == false) {  BUG HERE...

    isCollision = false;
    for (int i = 0; i < room->numSpikes; i++) {
        isCollision = collisionRectTri(&game->hero->body, &room->spikes[i]->body);
        if (isCollision == true) {
            game->hero->onCollision(room->spikes[i]);
        }
    }
    enemyPhysics(game);
    bulletPhysics(game);
    
    isCollision = false;
    for (int i = 0; i < room->numBasicEnemies; i++) {
        isCollision = collisionRectRect(&game->hero->body, &room->enemies[i]->body);
        if (isCollision == true) {
            game->hero->onCollision(room->enemies[i]);
        }
    }

    isCollision = false;
    for (int i = room->numBullet - 1 ; i >= 0; i--) {
        isCollision = collisionRectRect(&game->hero->body, &room->bullet[i]->body);
        if (isCollision == true) {
            if (room->bullet[i]->id != HBULLET) {
                game->hero->onCollision(room->bullet[i]);
                delete room->bullet[i];
                room->bullet.erase(room->bullet.begin() + i);
                room->numBullet--;
            }
        }
    }

    if (game->hero->state == DEATH) {
        // TEMPORARY: return hero to start
        game->hero->jumpInitiated = 0;
        game->hero->initialJump = 0;
        game->hero->secondJump = 0;
        game->hero->jumpCount = 0;
        game->hero->jumpRelease = 1;
        game->hero->jumpFinished = 0;
        game->hero->velocity[0] = 0;
        //game->hero->velocity[1] = 0;
    }
	//play grunt sound
	if(game->hero->state == DEATH && previous != DEATH) {
	//////////initilize openAL
	cout<<"grunt sound\n";
	playDeath();

	}



    game->checkRoom();
}

void render_game(Game* game)
{
    static bool firstEditorRun = true;
    static int shadowColor[3];
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    Room* current_level = game->getRoomPtr();

    //glClear(GL_COLOR_BUFFER_BIT);
    float w, h;

    if(firstEditorRun) {
        if(game->state == LEVEL_EDITOR) {
            firstEditorRun = false;
            shadowColor[0] = 100;
            shadowColor[1] = 100;
            shadowColor[2] = 100;
        }
    }
    
    glColor3f(1.0,1.0,1.0);
    if( forestBackgroundSet == true ) {
        renderBackground(forestTexture);
    }

    //int bulletAnimation = 0;
    for(auto entity : current_level->bullet) {
        renderBullet(entity, bulletAnimation);
        bulletAnimation = (bulletAnimation + 1)%10;
    }
    for(auto entity : current_level->spikes) {
        renderSpike(entity);
    }
    int enemyItr = 0;
    for(auto &entity : current_level->enemies) {
        renderEnemy(entity);
        enemyItr = (enemyItr + 1)%10;
    }
    int savePointCounter= 0;
    for(auto entity : current_level->savePoints) {
        if (current_level == savePointRoom) {
            renderSavePoint(entity, savePointCounter);
        }
        else {
            renderSavePoint(entity, -1);
        }
        savePointCounter++;
    }

    if(game->isElevatorResizable) {
        renderElevatorShadow(current_level->elevators[game->resizableElevatorIndex], current_level->elevators[game->resizableElevatorIndex]->rgb, 0.5);
    }
    for(int i = 0; i < current_level->numElevators; i++) {
        if(game->state == LEVEL_EDITOR) {
            if(i != game->resizableElevatorIndex) {
                renderElevatorShadow(current_level->elevators[i], shadowColor, 0.5);
            }
            else if(!game->isElevatorResizable) {
                renderElevatorShadow(current_level->elevators[i], shadowColor, 0.5);
            }
        }
        renderPlatform(current_level->elevators[i]);
    }
    for(auto entity : current_level->platforms) {
        renderPlatform(entity);
    }
    if( game->hero->state == DEATH && (renderNum % 40 <= 25)) {
        renderTexture(deadMessageTexture, 0.0,1.0,0.0, 1.0, 400, 100);
    }
    
    // Draw the Hero to the screen
    w = game->hero->body.width;
    h = game->hero->body.height;
 
    if (microseconds > 80000) {
        if (game->hero->state == WALKING &&
          game->hero->rightPressed &&
          game->hero->leftPressed == 0)
        {
            renderHero(walkRightTexture,game,game->hero->heroWalkingR,numAnimation,w, h, 10);
        }
        else if (game->hero->state == WALKING &&
          game->hero->leftPressed &&
          game -> hero->rightPressed == 0)
        {
            renderHero(walkLeftTexture,game  ,game->hero->heroWalkingL,numAnimation,w, h, 10);
        }
        else if (game->hero->state == JUMPING &&
          game->hero->body.orientation == FACING_RIGHT)
        {
            renderHero(jumpRightTexture, game, game->hero->heroJumpR, numAnimation, w, h, 10);
        }
        else if (game->hero->body.orientation == FACING_LEFT &&
          game->hero->state == JUMPING)
        {
            renderHero(jumpLeftTexture, game, game->hero->heroJumpL, numAnimation, w, h, 10);
        }
        else if (game->hero->body.orientation == FACING_LEFT &&
          game->hero->state == STANDING)
        {
            renderHero(idleLeftTexture,game  ,game->hero->heroIdleL,numAnimation,w, h, 10);
        }
        else if (game->hero->state == DEATH) {
            //renderHero(heroDeathTexture,game,game->hero->heroDeath,numAnimation,w,h,10);
            renderHero(heroDeathTexture,game,game->hero->heroDeath,0,w,h,10);
            renderNum = (renderNum + 1)%40;
             
        }
        else {
            renderHero(idleRightTexture,game  ,game->hero->heroIdleR,numAnimation,w, h, 10);
        }
        numAnimation = (numAnimation + 1) % 10;
        start = std::chrono::high_resolution_clock::now();
    }
        //helloworld

    else {
        if (game->hero->state == WALKING &&
          game->hero->rightPressed &&
          game->hero->leftPressed == 0)
        {
            renderHero(walkRightTexture,game  ,game->hero->heroWalkingR,numAnimation,w, h, 10);
        }
        else if (game->hero->state == WALKING &&
          game->hero->leftPressed &&
          game -> hero->rightPressed == 0)
        {
            renderHero(walkLeftTexture,game  ,game->hero->heroWalkingL,numAnimation,w, h, 10);
        }
        else if (game->hero->state == JUMPING &&
          game->hero->body.orientation == FACING_RIGHT)
        {
            renderHero(jumpRightTexture,game  ,game->hero->heroJumpR,numAnimation,w, h, 10);
        }
        else if (game->hero->body.orientation == FACING_LEFT &&
          game->hero->state == JUMPING)
        {
            renderHero(jumpLeftTexture,game  ,game->hero->heroJumpL,numAnimation,w, h, 10);
        }
        else if (game->hero->body.orientation == FACING_LEFT &&
          game->hero->state == STANDING)
        {
            renderHero(idleLeftTexture,game  ,game->hero->heroIdleL,numAnimation,w, h, 10);
        }
        else if (game->hero->state == DEATH) {
            //std::cout<<"DEAD\n";
            //renderHero(heroDeathTexture,game,game->hero->heroDeath,numAnimation,w,h,10);
            renderHero(heroDeathTexture,game,game->hero->heroDeath,0,w,h,10);
            renderNum = (renderNum + 1)%40;
        }
        else {
            renderHero(idleRightTexture,game  ,game->hero->heroIdleR,numAnimation,w, h, 10);
        }
    }
}

void renderEnemy(GameObject * entity)
{
    float w = entity->body.width;
    float h = entity->body.height;
    glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);

    if (entity->id == SHOOTERENEMY) {
        w = entity->body.width+5;
        h = entity->body.height+5;
        if( entity->body.orientation == FACING_RIGHT)
        {
            if ( microseconds > 80000) {
                shooterAnimation = (shooterAnimation + 1) % 10;
            }
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
            glBindTexture(GL_TEXTURE_2D, eShootingRightTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x1,((ShooterEnemy*)entity)->sprite[shooterAnimation].y2); glVertex2i(-w,-h);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x1,((ShooterEnemy*)entity)->sprite[shooterAnimation].y1); glVertex2i(-w,h);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x2,((ShooterEnemy*)entity)->sprite[shooterAnimation].y1); glVertex2i(w,h);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x2,((ShooterEnemy*)entity)->sprite[shooterAnimation].y2); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
        }
        else if( entity->body.orientation == FACING_LEFT)
        { 
            if ( microseconds > 80000) {
                shooterAnimation = (shooterAnimation + 1) % 10;
            }
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
            glBindTexture(GL_TEXTURE_2D, eShootingLeftTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x1,((ShooterEnemy*)entity)->sprite[shooterAnimation].y2); glVertex2i(-w,-h);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x1,((ShooterEnemy*)entity)->sprite[shooterAnimation].y1); glVertex2i(-w,h);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x2,((ShooterEnemy*)entity)->sprite[shooterAnimation].y1); glVertex2i(w,h);
            glTexCoord2f(((ShooterEnemy*)entity)->sprite[shooterAnimation].x2,((ShooterEnemy*)entity)->sprite[shooterAnimation].y2); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
        }
        else {
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
            glBindTexture(GL_TEXTURE_2D, shooterDeathTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,1.0f); glVertex2i(-w,-h);
            glTexCoord2f(0.0f,0.0f); glVertex2i(-w,h);
            glTexCoord2f(1.0f,0.0f); glVertex2i(w,h);
            glTexCoord2f(1.0f,1.0f); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
            if(afterDeath > 75) {
                ((ShooterEnemy*)entity)->state = DEATH;
                afterDeath = 0;
            }
            afterDeath++;
        }
    }
    else {
        if( entity->body.orientation == FACING_RIGHT)
        {
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
            glBindTexture(GL_TEXTURE_2D, spikeEnemyRightTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x1,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y2); glVertex2i(-w,-h);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x1,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y1); glVertex2i(-w,h);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x2,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y1); glVertex2i(w,h);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x2,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y2); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
            spikeAnimation = (spikeAnimation + 1) %10;
        }
        else if( entity->body.orientation == FACING_LEFT)
        {
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
            glBindTexture(GL_TEXTURE_2D, spikeEnemyLeftTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x1,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y2); glVertex2i(-w,-h);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x1,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y1); glVertex2i(-w,h);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x2,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y1); glVertex2i(w,h);
            glTexCoord2f(((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].x2,((BasicEnemy*)entity)->enemyWalkRight[spikeAnimation].y2); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
            spikeAnimation = (spikeAnimation + 1) %10;
        }
        else {
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
            glBindTexture(GL_TEXTURE_2D, spikeDeathTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,1.0f); glVertex2i(-w,-h);
            glTexCoord2f(0.0f,0.0f); glVertex2i(-w,h);
            glTexCoord2f(1.0f,0.0f); glVertex2i(w,h);
            glTexCoord2f(1.0f,1.0f); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
            if(afterDeath > 75) {
                ((BasicEnemy*)entity)->state = DEATH;
                afterDeath = 0;
            }
            afterDeath++;
        }
    }
}

void renderBullet(GameObject * entity, int index)
{
    float w = entity->body.width + 4;
    float h = entity->body.height + 4;
    glEnable(GL_TEXTURE_2D);
    glColor4ub(255,255,255,255);
    glPushMatrix();

    glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
    glBindTexture(GL_TEXTURE_2D, bulletTexture);
    glBegin(GL_QUADS);
        glTexCoord2f(((BasicBullet*)entity)->bullet[index].x1,((BasicBullet*)entity)->bullet[index].y2);
        glVertex2i(-w,-h);
        glTexCoord2f(((BasicBullet*)entity)->bullet[index].x1,((BasicBullet*)entity)->bullet[index].y1);
        glVertex2i(-w,h);
        glTexCoord2f(((BasicBullet*)entity)->bullet[index].x2,((BasicBullet*)entity)->bullet[index].y1);
        glVertex2i(w,h);
        glTexCoord2f(((BasicBullet*)entity)->bullet[index].x2,((BasicBullet*)entity)->bullet[index].y2);
        glVertex2i(w,-h);
    glEnd();

    glPopMatrix();
    index = (index + 1)%10;
}

void renderSpike(GameObject * entity)
{
    glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);
    glEnable(GL_TEXTURE_2D);
    glColor4ub(255,255,255,255);
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, spikeTexture);
    glBegin(GL_TRIANGLES);
    //moises
        glTexCoord2f(0.1,.9); glVertex2f(entity->body.corners[0][0],entity->body.corners[0][1]);
        glTexCoord2f(.9,.9); glVertex2f(entity->body.corners[1][0],entity->body.corners[1][1]);
        glTexCoord2f(.5,.5); glVertex2f(entity->body.corners[2][0],entity->body.corners[2][1]);
    glEnd();
    glPopMatrix();
}

void renderPlatform(GameObject * entity)
{
    float w = entity -> textureWidth;
    float h = entity -> textureHeight;
    
    int cornerX = entity->body.center[0] - entity->body.width;
    int cornerY = entity->body.center[1] + entity -> body.height;

    glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);
    for (int row = 0; row < entity->verticalTiles; row++){
        int rowOffset = cornerY - ((row * entity->textureHeight * 2) + entity->textureHeight);    

        for (int column = 0; column < entity->horizontalTiles; column++){
            //The follwoing code is to draw the platforms
            int colOffset = cornerX + (column * entity->textureWidth * 2) + entity->textureWidth;
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            //glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
            glTranslatef(colOffset, rowOffset, entity->body.center[2]);
            glBindTexture(GL_TEXTURE_2D, platformTextures[entity->tex_id]);
            glBegin(GL_QUADS);
                glTexCoord2f(0.1f,1.0f); glVertex2i(-w,-h);
                glTexCoord2f(0.1f,0.0f); glVertex2i(-w,h);
                glTexCoord2f(1.0f,0.0f); glVertex2i(w,h);
                glTexCoord2f(1.0f,1.0f); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
        }
    }
}

void renderSavePoint(GameObject * entity, int index)
{
    glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);
    float w = entity->body.width;
    float h = entity->body.height;
    if(index == currentSavePoint) {
        glEnable(GL_TEXTURE_2D);
        glColor4ub(255,255,255,255);
        glPushMatrix();
        glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
        glBindTexture(GL_TEXTURE_2D, keyTexture);
        glBegin(GL_QUADS);
            glTexCoord2f(0.1f,1.0f); glVertex2i(-w,-h);
            glTexCoord2f(0.1f,0.0f); glVertex2i(-w,h);
            glTexCoord2f(1.0f,0.0f); glVertex2i(w,h);
            glTexCoord2f(1.0f,1.0f); glVertex2i(w,-h);
        glEnd();
        glPopMatrix();
    }
    else {    
        glEnable(GL_TEXTURE_2D);
        glColor4ub(255,255,255,255);
        glPushMatrix();
        glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
        glBindTexture(GL_TEXTURE_2D, checkPointTexture);
        glBegin(GL_QUADS);
            glTexCoord2f(0.1f,1.0f); glVertex2i(-w,-h);
            glTexCoord2f(0.1f,0.0f); glVertex2i(-w,h);
            glTexCoord2f(1.0f,0.0f); glVertex2i(w,h);
            glTexCoord2f(1.0f,1.0f); glVertex2i(w,-h);
        glEnd();
        glPopMatrix();
    }
}

/*void renderElevator(GameObject * entity)
{
    float w = entity -> textureWidth;
    float h = entity -> textureHeight;
    
    int cornerX = entity->body.center[0] - entity->body.width;
    int cornerY = entity->body.center[1] + entity -> body.height;

    glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);
    for (int row = 0; row < entity->verticalTiles; row++){
        int rowOffset = cornerY - ((row * entity->textureHeight * 2) + entity->textureHeight);    

        for (int column = 0; column < entity->horizontalTiles; column++){
            //The follwoing code is to draw the platforms
            int colOffset = cornerX + (column * entity->textureWidth * 2) + entity->textureWidth;
            glEnable(GL_TEXTURE_2D);
            glColor4ub(255,255,255,255);
            glPushMatrix();
            glTranslatef(colOffset, rowOffset, entity->body.center[2]);
            std::cout<<"TEXT ID: "<<entity->tex_id<<"\n";
            glBindTexture(GL_TEXTURE_2D, platformTextures[entity->tex_id]);
            glBegin(GL_QUADS);
                glVertex2f(-w,-h);
                glVertex2f(-w,h);
                glVertex2f(w,h);
                glVertex2f(w,-h);
                glTexCoord2f(0.1f,1.0f); glVertex2i(-w,-h);
                glTexCoord2f(0.1f,0.0f); glVertex2i(-w,h);
                glTexCoord2f(1.0f,0.0f); glVertex2i(w,h);
                glTexCoord2f(1.0f,1.0f); glVertex2i(w,-h);
            glEnd();
            glPopMatrix();
        }
    }
}*/

void renderElevatorShadow(Elevator * entity, int color[3], float alpha)
{
    float w = entity->body.width;
    float h = entity->body.height;
    float leftBound = entity->body.center[0] + w;
    float rightBound = entity->body.center[0] - w;
    float upperBound = entity->getUpperLimit() + h;
    float lowerBound = entity->getLowerLimit() - h;
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(
      ((float)color[0])/255.0,
      ((float)color[1])/255.0,
      ((float)color[2])/255.0,
      alpha);
    glPushMatrix();
    glBegin(GL_QUADS);
        glVertex2f(leftBound, lowerBound);
        glVertex2f(leftBound, upperBound);
        glVertex2f(rightBound, upperBound);
        glVertex2f(rightBound, lowerBound);
    glEnd();
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

