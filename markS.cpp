//Mark Stevens
//This function handles game input.


#include "structs.h"
#include "gameObject.h"
#include "hero.h"
#include "basicEnemy.h"
#include "platform.h"
#include "spike.h"
#include "room.h"
#include "ppm.h"
#include "game.h"
#include "collisions.h"
#include <chrono>
#include <GL/glx.h>
#include "fernandoV.h"
#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 700

void editorAddPlatform(Game *game, GameObject * mouse);
void editorAddSpike(Game *game, GameObject * mouse);
void editorAddSavePoint(Game *game, GameObject * mouse);
void resizePlatform(Game *game, GameObject * mouse);
void editorRemovePlatform(Game *game, int index);
void editorRemoveSpike(Game *game, int index);
void editorRemoveSavePoint(Game *game, int index);


void check_game_input(XEvent *e, Game *game)
{
    GameObject mouse;
    mouse.body.type = RECTANGLE;
    mouse.body.width = 1;
    mouse.body.height = 1;
    mouse.body.center[0] = e->xbutton.x;
    mouse.body.center[1] = WINDOW_HEIGHT - e->xbutton.y;

    if (e->type == KeyPress){
        //cout << e->xbutton.x << endl;
        int key = XLookupKeysym(&e->xkey,0);
        if (key == XK_Left){
            game->hero->leftPressed = 1;
        }
        if (key == XK_Right){
            game->hero->rightPressed = 1;
        }
        if (key == XK_Escape){
            g_gamestate = MAIN_MENU;
			stopGameMusic();
            playMenuMusic();
        }
        if (key == XK_e){
            if (game->hero->delay == 0) {
                game->heroShoots();
                game->hero->delay++;
            }
        }

        if ((key == XK_Up || key == XK_w || key == XK_space) && game->hero->jumpRelease == 0){
            if (game->hero->state == WALKING || game->hero->state == STANDING){
                game->hero->initialJump = 1;
            }
            if (game->hero->state == JUMPING && game->hero->jumpCount < 2){
                game->hero->secondJump = 1;
            }
        }
        if (game->state == LEVEL_EDITOR){
            if (key == XK_b){
                game->saveRooms();
            }
            if (key == XK_j){
                game->moveRoomLeft();
            }
            if (key == XK_l){
                game->moveRoomRight();
            }
            if (key == XK_k){
                game->moveRoomDown();
            }
            if (key == XK_i){
                game->moveRoomUp();
            }
            if (key == XK_5){
                game->hero->body.center[0] = e->xbutton.x;
                game->hero->body.center[1] = WINDOW_HEIGHT - e->xbutton.y;
            }
            if (key == XK_Shift_L){
                if (!game->isPlatformMovable &&
                  !game->isPlatformResizable &&
                  !game->isSpikeMovable)
                {
                    editorAddPlatform(game, &mouse);
                }
            }
            if (key == XK_a){
                if (!game->isPlatformMovable &&
                  !game->isPlatformResizable &&
                  !game->isSpikeMovable)
                {
                    editorAddSavePoint(game, &mouse);
                }
            }
            if (key == XK_s){
                if (game->isSpikeMovable){
                    GameObject * curSpike = game->getWorkingSpikePtr();
                    ShapeOrientation spikeFacing = curSpike->body.orientation;

                    if (curSpike->body.orientation == FACING_UP){
                        spikeFacing = FACING_LEFT;
                    }
                    else if (curSpike->body.orientation == FACING_LEFT){
                        spikeFacing = FACING_DOWN;
                    }
                    else if (curSpike->body.orientation == FACING_DOWN){
                        spikeFacing = FACING_RIGHT;
                    }
                    else if (curSpike->body.orientation == FACING_RIGHT){
                        spikeFacing = FACING_UP;
                    }
                    curSpike->body.orientation = spikeFacing;
                }
                if (!game->isSpikeMovable &&
                  !game->isPlatformMovable &&
                  !game->isPlatformMovable)
                {
                    editorAddSpike(game, &mouse);
                }
            }
            if (key == XK_x){
                if (game->isPlatformMovable){
                    game->isPlatformMovable = false;
                }
                if (game->isPlatformResizable){
                    game->isPlatformResizable = false;
                }
                if (game->isSpikeMovable){
                    game->isSpikeMovable = false;
                }
                if (game->isSavePointMovable){
                    game->isSavePointMovable = false;
                }
            }
            if (key == XK_z){
                if (!game->isPlatformMovable &&
                  !game->isPlatformResizable &&
                  !game->isSpikeMovable &&
                  !game->isSavePointMovable)
                {
                    Room * room = game->getRoomPtr();
                    for (unsigned int k = 0; k < room->savePoints.size(); k++){
                        if (collisionRectRect(&mouse.body, &room->savePoints[k]->body)){
                            game->movableSavePointIndex = k;
                            game->isSavePointMovable = true;
                            break;
                        }
                    }
                    if (!game->isSavePointMovable){
                        vector<GameObject*> * spikesV = game->getSpikesVPtr();
                        for (unsigned int k = 0; k < spikesV->size(); k++){
                            if (collisionRectTri(&mouse.body, &spikesV->at(k)->body)){
                                game->movableSpikeIndex = k;
                                game->isSpikeMovable = true;
                                break;
                            }
                        }
                    }
                    if (!game->isSpikeMovable && !game->isSavePointMovable){
                        vector<GameObject*> * platformsV = game->getPlatformsVPtr();
                        for (unsigned int k = 0; k < platformsV->size(); k++){
                            if (collisionRectRect(&mouse.body,&platformsV->at(k)->body)){
                                game->movablePlatformIndex = k;
                                game->isPlatformMovable = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (key == XK_c &&
              !game->isPlatformMovable &&
              !game->isSpikeMovable)
            {
                vector<GameObject*> * platformsV = game->getPlatformsVPtr();
                for (unsigned int k = 0; k < platformsV->size(); k++){
                    if (collisionRectRect(&mouse.body,&platformsV->at(k)->body)){
                        game->resizablePlatformIndex = k;
                        game->isPlatformResizable = true;
                        game->resizablePlatformX = platformsV->at(k)->body.center[0];
                        game->resizablePlatformY = platformsV->at(k)->body.center[1];
                    }
                }
            }
            if (key == XK_d &&
              !game->isPlatformMovable &&
              !game->isPlatformResizable &&
              !game->isSpikeMovable &&
              !game->isSavePointMovable)
            {
                Room * room = game->getRoomPtr();
                for (unsigned int L = 0; L < room->savePoints.size(); L++){
                    if (collisionRectRect(
                      &mouse.body,
                      &room->savePoints[L]->body))
                    {
                        editorRemoveSavePoint(game, L);
                        return;
                    }
                }
                vector<GameObject*> * spikesV = game->getSpikesVPtr();
                for (unsigned int k = 0; k < spikesV->size(); k++){
                    if (collisionRectTri(
                      &mouse.body,
                      &spikesV->at(k)->body))
                    {
                        editorRemoveSpike(game, k);
                        return;
                    }
                }
                vector<GameObject*> * platformsV = game->getPlatformsVPtr();
                for (unsigned int p = 0; p < platformsV->size(); p++){
                    if (collisionRectRect(
                      &mouse.body,
                      &platformsV->at(p)->body))
                    {
                        editorRemovePlatform(game, p);
                        return;
                    }
                }
            }
        }
    }
    if (e->type == KeyRelease){
        int key = XLookupKeysym(&e->xkey,0);
        if ( key == XK_Left){
            game->hero->leftPressed = 0;
        }
        if ( key == XK_Right){
            game->hero->rightPressed = 0;
        }
        if ( key == XK_w || key == XK_Up || key == XK_space){
            game->hero->jumpRelease = 4;
        }
    }
    if (game->isPlatformResizable){
        resizePlatform(game, &mouse);
    }
}

void editorAddPlatform(Game * game, GameObject * mouse)
{
    Room * room = game->getRoomPtr();
    room->platforms.push_back(
      new Platform(
        game->platformTextureWidth,
        game->platformTextureHeight,
        mouse->body.center[0],
        mouse->body.center[1]));
    room->numPlatforms++;
    game->movablePlatformIndex = room->platforms.size() - 1;
    game->isPlatformMovable = true;
}

void editorAddSpike(Game * game, GameObject * mouse)
{
    Room * room = game->getRoomPtr();
    Vec spike[3];
    spike[0][0] = mouse->body.center[0];
    spike[0][1] = mouse->body.center[1];
    spike[0][2] = 0;
    spike[1][0] = mouse->body.center[0] + 30;
    spike[1][1] = mouse->body.center[1];
    spike[1][2] = 0;
    spike[2][0] = mouse->body.center[0] + 15;
    // (sqrt(3) / 2 ) * width  = height for equilateral triangle
    spike[2][1] = mouse->body.center[1] + 25.981;
    spike[2][2] = 0;

    room->spikes.push_back(
      new Spike(spike,FACING_UP));
    room->numSpikes++;
    game->movableSpikeIndex = room->spikes.size() - 1;
    game->isSpikeMovable = true;
}

void editorAddSavePoint(Game * game, GameObject * mouse)
{
    Room * room = game->getRoomPtr();
    room->savePoints.push_back(
      new SavePoint(10,10,mouse->body.center[0],mouse->body.center[1]));
    room->numSavePoints++;
    game->isSavePointMovable = true;
    game->movableSavePointIndex = room->savePoints.size() - 1;
}

void resizePlatform(Game * game, GameObject * mouse)
{
    GameObject * platform = game->getWorkingPlatformPtr();
    int mouseX = (int)mouse->body.center[0];
    int mouseY = (int)mouse->body.center[1];
    int height = (( ( mouseY - game->resizablePlatformY) / ( platform->textureHeight)) * platform->textureHeight + platform->textureHeight);
    int width = (( ( mouseX - game->resizablePlatformX) / ( platform->textureWidth)) * platform->textureWidth + platform->textureWidth);

    if ( width <= 0){
        platform->body.width = platform->textureWidth;
    }
    else{
        platform->body.width = width;
        platform->horizontalTiles = platform->body.width / platform->textureWidth;
    }
    if ( height <= 0){
        platform->body.height = platform->textureHeight;
    }
    else{
        platform->body.height = height;
        platform->verticalTiles = platform->body.height / platform->textureHeight;
    }
}

void editorRemovePlatform(Game * game, int index)
{
    Room * room = game->getRoomPtr();
    room->platforms.erase(room->platforms.begin() + index);
    room->numPlatforms--;
}

void editorRemoveSpike(Game * game, int index)
{
    Room * room = game->getRoomPtr();
    room->spikes.erase(room->spikes.begin() + index);
    room->numSpikes--;
}

void editorRemoveSavePoint(Game * game, int index)
{
    Room * room = game->getRoomPtr();
    room->savePoints.erase(room->savePoints.begin() + index);
    room->numSavePoints--;
}


