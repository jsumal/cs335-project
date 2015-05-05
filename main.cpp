#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <sys/time.h>
#include <typeinfo>
#include <vector>
#include "structs.h"
#include "gameObject.h"
#include "hero.h"
#include "basicEnemy.h"
#include "platform.h"
#include "Level.h"
#include "game.h"
#include "ppm.h"
#include<time.h>
#include <chrono>  //Library used to count the microseconds when rendering sprites
//#include "bpm.h"
#include "collisions.h"
//#include "initializeLevels.h"

#include <fstream>

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 700

#define GRAVITY -0.35
#define MAXBUTTONS 4

using namespace std;

void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void set_title(void);
void init_MainMenuButtons(void);
void render_MainMenu(void);
void check_menu_button(XEvent *e);
void render_game(game* game);
void renderBackground(GLuint);
void renderTexture(GLuint imageTexture, float x1,float x2,float y1, float y2, int width, int height);
Level*** initializeLevels();

void check_game_input(XEvent *e, game * game);
void physics(game * game);

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;
//Following Declarations are for the Image importing...
unsigned char *buildAlphaData(Ppmimage *img);
void renderHero(GLuint heroTexture,game* game  ,Coordinates* heroSprite,int index,int w, int h);
GLuint getBMP(const char *path);
Ppmimage *guiBackgroundImage = NULL;
Ppmimage *rockImage = NULL;
Ppmimage *heroImage = NULL;
Ppmimage *backgroundImage = NULL;
Ppmimage *mainMenuButtonsImage = NULL;
Ppmimage *mainMenuButtonsExitImage = NULL;
GLuint guiBackgroundTexture;
GLuint rockTexture;
GLuint heroTexture;
GLuint forestTexture;
GLuint mainMenuButtonsTexture;
GLuint mainMenuButtonsExitTexture;
bool forestBackgroundSet=true;
CharacterState prevPosition;
int numAnimation = 0;
auto start = std::chrono::high_resolution_clock::now();
//End

Button button[MAXBUTTONS];
int nbuttons=0;

enum GameState {MAIN_MENU, PLAYING, EXIT_GAME};
GameState g_gamestate = MAIN_MENU;

int numCollisions;

int main()
{
    initXWindows();
    init_opengl();
    init_MainMenuButtons();
    Level*** levels = initializeLevels();
    game newgame(levels);
    newgame.hero = new Hero();
    prevPosition = newgame.hero->state;
    while(g_gamestate != EXIT_GAME) {
        switch (g_gamestate) {
            case MAIN_MENU:
                while(XPending(dpy)) {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    check_menu_button(&e);
                    //If they go back to main menu the previous position will be Standing.
                    prevPosition = STANDING;
                }
                render_MainMenu();
                break;
            case PLAYING:
                while(XPending(dpy)) {
                    XEvent e;
                    XNextEvent(dpy, &e);
                    check_game_input(&e, &newgame);
                }
                physics(&newgame);
                render_game(&newgame);
                break;
            case EXIT_GAME:
                break;
            default:
                break;
        }
        glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void initXWindows(void) {
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
}

void init_opengl(void) {
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    
    //Importing the images 
    heroImage = ppm6GetImage("./images/HeroSpriteSheet.ppm");
    backgroundImage = ppm6GetImage("./images/Background1.ppm");
    rockImage = ppm6GetImage("./images/Rock.ppm");
    mainMenuButtonsImage = ppm6GetImage("./images/start.ppm");
    guiBackgroundImage = ppm6GetImage("./images/GuiBackground.ppm");
    mainMenuButtonsExitImage = ppm6GetImage("./images/exit.ppm");
    glGenTextures(1, &heroTexture);
    glGenTextures(1, &forestTexture);
    glGenTextures(1, &rockTexture);
    glGenTextures(1, &mainMenuButtonsTexture);
    glGenTextures(1, &guiBackgroundTexture);
    glGenTextures(1, &mainMenuButtonsExitTexture);
    
    int w = heroImage->width;
    int y = heroImage->height;
    //Setting up the hero textures
    glBindTexture(GL_TEXTURE_2D,heroTexture);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w,y, 0,
               GL_RGB, GL_UNSIGNED_BYTE, heroImage->data);

    unsigned char *silhouetteData = buildAlphaData(heroImage);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, y, 0,
	    GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
    delete [] silhouetteData;


    //Setting up the Main menu buttons texture...
    w = mainMenuButtonsImage->width;
    y = mainMenuButtonsImage->height;
    glBindTexture(GL_TEXTURE_2D,mainMenuButtonsTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, y, 0,
               GL_RGB, GL_UNSIGNED_BYTE, mainMenuButtonsImage->data);
    
    //Setting up the exit button image texture....
    w = mainMenuButtonsExitImage->width;
    y = mainMenuButtonsExitImage->height;
    glBindTexture(GL_TEXTURE_2D,mainMenuButtonsExitTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, y, 0,
               GL_RGB, GL_UNSIGNED_BYTE, mainMenuButtonsExitImage->data);


    //Setting up the Rock Platforms Texture....
    w = rockImage->width;
    y = rockImage->height;
    glBindTexture(GL_TEXTURE_2D,rockTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, y, 0,
               GL_RGB, GL_UNSIGNED_BYTE, rockImage->data);

    //Setting up the background image
    glBindTexture(GL_TEXTURE_2D,forestTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, backgroundImage->width, backgroundImage->height, 0,
               GL_RGB, GL_UNSIGNED_BYTE, backgroundImage->data);
    
    //Setting up the Gui Background image.
    glBindTexture(GL_TEXTURE_2D,guiBackgroundTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, guiBackgroundImage->width, guiBackgroundImage->height, 0,
               GL_RGB, GL_UNSIGNED_BYTE, guiBackgroundImage->data);
}

void cleanupXWindows(void) {
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

void init_MainMenuButtons(void) {
	//initialize buttons...
	nbuttons=0;
	//size and position
	button[nbuttons].r.width = 100;
	button[nbuttons].r.height = 40;
	button[nbuttons].r.left = 290;
	button[nbuttons].r.bot = 320;
	button[nbuttons].r.right = button[nbuttons].r.left + button[nbuttons].r.width;
	button[nbuttons].r.top = button[nbuttons].r.bot + button[nbuttons].r.height;
	button[nbuttons].r.centerx = (button[nbuttons].r.left + button[nbuttons].r.right) / 2;
	button[nbuttons].r.centery = (button[nbuttons].r.bot + button[nbuttons].r.top) / 2;
	strcpy(button[nbuttons].text, "Reset");
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
	button[nbuttons].r.bot = 240;
	button[nbuttons].r.right = button[nbuttons].r.left + button[nbuttons].r.width;
	button[nbuttons].r.top = button[nbuttons].r.bot + button[nbuttons].r.height;
	button[nbuttons].r.centerx = (button[nbuttons].r.left + button[nbuttons].r.right) / 2;
	button[nbuttons].r.centery = (button[nbuttons].r.bot + button[nbuttons].r.top) / 2;
	strcpy(button[nbuttons].text, "Quit");
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


//Add the Main Menu background to this function....
void render_MainMenu(void) {
    //Rect r;
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0,1.0,1.0);
    
    //Rendering the main menu background
    glEnable(GL_TEXTURE_2D);
    renderBackground(guiBackgroundTexture);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    for (int i=0; i<nbuttons; i++) {
		if (button[i].over) {
			int w=2;
			glColor3f(1.0f, 1.0f, 0.0f);
			//draw a highlight around button
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
        glPopMatrix();    
        
        //glTranslatef(button[i].r.centerx,button[i].r.centery, 0.0f);

        //Rendering the menu items
        //Need to clean up my code after it works
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
        }
            //Working on the function below to reduce code reusability..
            //renderTexture(mainMenuButtonsTexture,0.1,0.9,0.1,0.9, 200, 30);

        if (i == 1) {
            //This will be a call to the function to render.
            //renderTexture(mainMenuButtonsExitTexture,0.1,0.9,0.1,0.9,200,30);
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
	}
}

void check_menu_button(XEvent *e) {
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
							break;
						case 1:
							g_gamestate = EXIT_GAME;
							break;
					}
				}
			}
		}
	}
	return;
}

void check_game_input(XEvent *e, game *game){

    if (e->type == KeyPress){
        int key = XLookupKeysym(&e->xkey,0);
        if (key == XK_Left){
            game->hero->leftPressed = 1;
        }
        if (key == XK_Right){
            game->hero->rightPressed = 1;
        }
        if (key == XK_Escape){
            g_gamestate = MAIN_MENU;
        }
        
        if ((key == XK_w || key == XK_space) && game->hero->jumpRelease == 0){
            if (game->hero->state == WALKING || game->hero->state == STANDING){
                game->hero->initialJump = 1;
            }
            if (game->hero->state == JUMPING && game->hero->jumpCount < 2){
                game->hero->secondJump = 1;
            }
        }
        if (key == XK_j){
            game->currentHorizontalLevel--;
        }
        if (key == XK_l){
            game->currentHorizontalLevel++;
        }
        if (key == XK_k){
            game->currentVerticalLevel--;
        }
        if (key == XK_i){
            game->currentVerticalLevel++;
        }
        if (key == XK_5){
            game->hero->body.center[0] = e->xbutton.x;
            game->hero->body.center[1] = WINDOW_HEIGHT - e->xbutton.y;
        }
        /*
        if (key == XK_e && shootPressed == 0){
            game->hero->shootPressed = 5;
        }*/

    }
    if (e->type == KeyRelease){
        int key = XLookupKeysym(&e->xkey,0);
        if ( key == XK_Left){
            game->hero->leftPressed = 0;
        }
        if ( key == XK_Right){
            game->hero->rightPressed = 0;
        }
        if ( key == XK_w ){
            game->hero->jumpRelease = 4;
            numAnimation = 0;
        }
    }

}

void physics(game * game){

    bool isCollision = false;
    Level * room = game->level[game->currentHorizontalLevel][game->currentVerticalLevel];
    game->hero->movement();
    for (int i = 0; i < room->numPlatforms; i++){
        isCollision = collision(game->hero, room->objects[i]);
        if (isCollision == true){
            game->hero->onCollision(room->objects[i]);
        }
    }
    game->checkRoom();
}

void render_game(game* game)
{
    glClear(GL_COLOR_BUFFER_BIT);
    float w, h;
    
    Level* current_level = game->level[game->currentHorizontalLevel][game->currentVerticalLevel];
    
    //Rendering the first back ground image if the flag is true.
    glColor3f(1.0,1.0,1.0);
    if(forestBackgroundSet==true){
        renderBackground(forestTexture);
    }


    // Draw the Hero to the screen
    //glColor3ub(200,200,200);
    //glPushMatrix();
    //glTranslatef(game->hero->body.center[0], game->hero->body.center[1], game->hero->body.center[2]);
    w = game->hero->body.width;
    h = game->hero->body.height;
    //glBegin(GL_QUADS);
    //glVertex2i(-w,-h);
    //glVertex2i(-w,h);
    //glVertex2i(w,-h);
    //glVertex2i(w,h);
    //glEnd();
    //glPopMatrix();

    //Draws the Hero to the Screen
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    if(microseconds > 80000) {
        if (game->hero->state == WALKING && game->hero->rightPressed && game->hero->leftPressed == 0) { 
            renderHero(heroTexture,game  ,game->hero->heroWalkingR,numAnimation,w, h);
            std::cout<<"Num: "<<numAnimation<<endl;
        }
        else if (game->hero->state == WALKING && game->hero->leftPressed && game -> hero->rightPressed == 0) { 
            renderHero(heroTexture,game  ,game->hero->heroWalkingL,numAnimation,w, h);
        }
        else if (game->hero->state == JUMPING) { 
            renderHero(heroTexture,game  ,game->hero->heroJump,numAnimation,w, h);
        }
        else {
            renderHero(heroTexture,game  ,game->hero->heroIdleR,numAnimation,w, h);
        }
    /*glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    glPushMatrix();
    glTranslatef(game->hero->body.center[0], game->hero->body.center[1], game->hero->body.center[2]);
    //renderTexture(heroTexture, game->hero->heroJump[i].x1,game->hero->heroJump[i].x2,game->hero->heroJump[i].y1,game->hero->heroJump[i].y2,w,h);
    glBindTexture(GL_TEXTURE_2D,heroTexture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x1,game->hero->heroWalkingR[numAnimation%10].y2); glVertex2i(-w-5,-h-5);
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x1,game->hero->heroWalkingR[numAnimation%10].y1); glVertex2i(-w-5,h+5); //here
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x2,game->hero->heroWalkingR[numAnimation%10].y1); glVertex2i(w+5,h+5);
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x2,game->hero->heroWalkingR[numAnimation%10].y2); glVertex2i(w+5,-h-5);

    //glTexCoord2f(0.8f,.4f); glVertex2i(-w-5,-h-5);
    glTexCoord2f(0.8f,.2f); glVertex2i(-w-5,h+5); //here
    glTexCoord2f(.9f,0.2f); glVertex2i(w+5,h+5);
    glTexCoord2f(.9f,0.4f); glVertex2i(w+5,-h-5);
    glEnd();//

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
    glPopMatrix();*/    
    numAnimation = (numAnimation + 1) % 10;
    start = std::chrono::high_resolution_clock::now();
    }
    else { 
        if (game->hero->state == WALKING && game->hero->rightPressed && game->hero->leftPressed == 0) { 
            renderHero(heroTexture,game  ,game->hero->heroWalkingR,numAnimation,w, h);
            std::cout<<"Num: "<<numAnimation<<endl;
        }
        else if (game->hero->state == WALKING && game->hero->leftPressed && game -> hero->rightPressed == 0) { 
            renderHero(heroTexture,game  ,game->hero->heroWalkingL,numAnimation,w, h);
        }
        else if (game->hero->state == JUMPING) { 
            renderHero(heroTexture,game  ,game->hero->heroJump,numAnimation,w, h);
        }
        else {
            renderHero(heroTexture,game  ,game->hero->heroIdleR,numAnimation,w, h);
        }
    /*glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    glPushMatrix();
    glTranslatef(game->hero->body.center[0], game->hero->body.center[1], game->hero->body.center[2]);
    //renderTexture(heroTexture, game->hero->heroJump[i].x1,game->hero->heroJump[i].x2,game->hero->heroJump[i].y1,game->hero->heroJump[i].y2,w,h);
    glBindTexture(GL_TEXTURE_2D,heroTexture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x1,game->hero->heroWalkingR[numAnimation%10].y2); glVertex2i(-w-5,-h-5);
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x1,game->hero->heroWalkingR[numAnimation%10].y1); glVertex2i(-w-5,h+5); //here
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x2,game->hero->heroWalkingR[numAnimation%10].y1); glVertex2i(w+5,h+5);
    glTexCoord2f(game->hero->heroWalkingR[numAnimation%10].x2,game->hero->heroWalkingR[numAnimation%10].y2); glVertex2i(w+5,-h-5);
    glEnd();

    //glTexCoord2f(0.344f,0.4f); glVertex2i(-60,-90);
    //glTexCoord2f(0.344f,0.2f); glVertex2i(-60,90); //here
    //glTexCoord2f(.430f,.2f); glVertex2i(60,90);
    //glTexCoord2f(.430f,0.4f); glVertex2i(60,-90);
    //glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
    glPopMatrix();*/    
    } 
    for(auto &entity : current_level->enemies) {
        glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);
        glPushMatrix();
        glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
        glBegin(GL_QUADS);
        glVertex2i(-entity->body.width,-entity->body.height);
        glVertex2i(-entity->body.width,entity->body.height);
        glVertex2i(entity->body.width,-entity->body.height);
        glVertex2i(entity->body.width,entity->body.height);
        glEnd();
        glPopMatrix();
    }

    for(auto entity : current_level->bullet) {
        glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);
        glPushMatrix();
        glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
        glBegin(GL_QUADS);
        glVertex2i(-entity->body.width,-entity->body.height);
        glVertex2i(-entity->body.width,entity->body.height);
        glVertex2i(entity->body.width,entity->body.height);
        glVertex2i(entity->body.width,-entity->body.height);
        glEnd();
        glPopMatrix();
    }

    for(auto entity : current_level->objects) {
        glColor3ub(entity->rgb[0], entity->rgb[1], entity->rgb[2]);
        glPushMatrix();
        glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
        glBegin(GL_QUADS);
        glVertex2i(-entity->body.width,-entity->body.height);
        glVertex2i(-entity->body.width,entity->body.height);
        glVertex2i(entity->body.width,entity->body.height);
        glVertex2i(entity->body.width,-entity->body.height);
        glEnd();
        glPopMatrix();

        w = entity->body.width;
        h = entity->body.height;

        //The following code is to draw the platforms
        glEnable(GL_TEXTURE_2D);
        glColor4ub(255,255,255,255);
        glPushMatrix();
        glTranslatef(entity->body.center[0], entity->body.center[1], entity->body.center[2]);
        glBindTexture(GL_TEXTURE_2D, rockTexture);
        glBegin(GL_QUADS);
        glTexCoord2f(0.1f,1.0f); glVertex2i(-w,-h);
        glTexCoord2f(0.1f,0.0f); glVertex2i(-w,h);
        glTexCoord2f(1.0f,0.0f); glVertex2i(w,h);
        glTexCoord2f(1.0f,1.0f); glVertex2i(w,-h);
        glEnd();
        glPopMatrix();    


    }

}

Level*** initializeLevels()
{
    Level*** room = (Level***)malloc(20 * sizeof(Level**));
    int count = 0;
    while (count < 20)
    {
        room[count] = (Level**)malloc( 5 * sizeof(void*));
        count++;
    }
    //Level* temp;
    //temp = new Level(13,1);
    
    
    ifstream roomFile;
    char num[5];
    int args[4];
    char row1 = '0';
    char row2 = '2';
    char column = '1';
    char fileName[19] = "Rooms/room";
    fileName[18] = 0;
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 4; j++){
            row1 = (char) (i / 10) + 48;
            row2 = (char) (i % 10) + 48;
            column = (char)j + 48;
            fileName[10] = row1;
            fileName[11] = row2;
            fileName[12] = '0';
            fileName[13] = column;
            cout << row1 << " " << row2 << " " << column << endl;
            fileName[14] = '.';
            fileName[15] = 't';
            fileName[16] = 'x';
            fileName[17] = 't';

            cout << fileName << endl;
            roomFile.open(fileName);
            room[row2-48][column-48] = new Level(0,0);
            while (true){
                roomFile >> num;
                if (roomFile.eof())
                    break;
                args[0] = atoi(num);
                roomFile >> num;
                if (roomFile.eof())
                    break;
                args[1] = atoi(num);
                roomFile >> num;
                if (roomFile.eof())
                    break;
                args[2] = atoi(num);
                roomFile >> num;
                if (roomFile.eof())
                    break;
                args[3] = atoi(num);
                room[row2-48][column-48]->objects.push_back(new platform(args[0], args[1], args[2], args[3]));
                room[row2-48][column-48]->numPlatforms++;
            }
            roomFile.close();
        }
    }
    
    return room;
}

//This function takes in a background parameter and will render it to the screen        
void renderBackground(GLuint backgroundTexture)
{        
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D,backgroundTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,1.0f); glVertex2i(0,0);
    glTexCoord2f(0.0,0.0); glVertex2i(0,WINDOW_HEIGHT);
    glTexCoord2f(1.0,0.0); glVertex2i(WINDOW_WIDTH,WINDOW_HEIGHT);
    glTexCoord2f(1.0f,1.0f); glVertex2i(WINDOW_WIDTH,0);
    glPopMatrix();
    glEnd();
}

void renderTexture(GLuint imageTexture, float x1,float x2,float y1, float y2, int width, int height)
{
    //glPushMatrix();
    glBindTexture(GL_TEXTURE_2D,imageTexture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(x1,y1); glVertex2i(width,height);
    glTexCoord2f(x1,y2); glVertex2i(-width,height);
    glTexCoord2f(x2,y2); glVertex2i(width,height);
    glTexCoord2f(x2,y1); glVertex2i(width,-height);
    glBindTexture(GL_TEXTURE_2D,0);
    glPopMatrix();
    glEnd();
}

//Grabbed this code from Gordons rainforest program.
unsigned char *buildAlphaData(Ppmimage *img)
{
    //add 4th component to RGB stream...
    int a,b,c;
    unsigned char *newdata, *ptr;
    unsigned char *data = (unsigned char *)img->data;
    //newdata = (unsigned char *)malloc(img->width * img->height * 4);
    newdata = new unsigned char[img->width * img->height * 4];
    ptr = newdata;
    for (int i=0; i<img->width * img->height * 3; i+=3) {
        a = *(data+0);
        b = *(data+1);
        c = *(data+2);
        *(ptr+0) = a;
        *(ptr+1) = b;
        *(ptr+2) = c;
        //
        //get the alpha value
        //
        //original code
        //get largest color component...
        //*(ptr+3) = (unsigned char)((
        //      (int)*(ptr+0) +
        //      (int)*(ptr+1) +
        //      (int)*(ptr+2)) / 3);
        //d = a;
        //if (b >= a && b >= c) d = b;
        //if (c >= a && c >= b) d = c;
        //*(ptr+3) = d;
        //
        //new code, suggested by Chris Smith, Fall 2013
        *(ptr+3) = (a|b|c);
        //
        ptr += 4;
        data += 3;
    }
    return newdata;
}
void renderHero(GLuint heroTexture,game* game  ,Coordinates* heroSprite,int index,int w, int h)
{
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    glPushMatrix();
    glTranslatef(game->hero->body.center[0], game->hero->body.center[1], game->hero->body.center[2]);
    glBindTexture(GL_TEXTURE_2D,heroTexture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(heroSprite[index%10].x1,heroSprite[index%10].y2); glVertex2i(-w-5,-h-5);
    glTexCoord2f(heroSprite[index%10].x1,heroSprite[index%10].y1); glVertex2i(-w-5,h+5); //here
    glTexCoord2f(heroSprite[index%10].x2,heroSprite[index%10].y1); glVertex2i(w+5,h+5);
    glTexCoord2f(heroSprite[index%10].x2,heroSprite[index%10].y2); glVertex2i(w+5,-h-5);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
    glPopMatrix();    
}
