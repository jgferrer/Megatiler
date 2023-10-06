#include <genesis.h>
#include <resources.h>
#include <font.h>
#include "main.h"

#define ELEMENTCOUNT(x)  (sizeof(x) / sizeof(x[0]))

typedef struct
{
    u16 x;
    u16 y;
    char label[10];
} Option;

#define NUM_MAIN_MENU_OPTIONS 2
Option mainmenu_options[NUM_MAIN_MENU_OPTIONS] = {
    {8, 8, "START"},
    {8, 9, "OPTIONS"}
};

#define NUM_MENU_OPTIONS 3
Option menu_options[NUM_MENU_OPTIONS] = {
    {10, 9, "Easy"},
    {10, 10, "Medium"},
    {10, 11, "Hard"}
};

enum GAME_STATE {
    STATE_MENU,
    STATE_PLAY,
    STATE_OPTIONS
};
enum GAME_STATE currentState;

u8 currentIndex = 0;
u8 currentOptionsIndex = 0;
u8 difficulty = 0; // 0-easy, 1-medium, 2-hard
Sprite *cursor;

u8 level1[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 4, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

typedef struct{
    u8 x;
    u8 y;
} Point;

typedef struct
{
    Point pos;
    Point tilePos;
    int w;
    int h;
    int health;
    bool moving;
    moveDirection dir;
    Sprite *sprite;
    char name[6];
} Entity;

#define SPAWN_TILE 4
#define TILESIZE 8
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define SOLID_TILE 1
#define ANIM_DOWN 0
#define ANIM_UP 1
#define ANIM_SIDE 2

Entity player = {{0, 0}, {0, 0}, 8, 8, 0, FALSE, none, NULL, "PLAYER"};
s16 cont_pulsacion;

int main()
{    
    SYS_disableInts();
    basicInit();
    SYS_enableInts();
    while(1)
    {
        SYS_disableInts();
        switch(currentState){
            case STATE_MENU:{
                processStateMenu();
                break;
            }
            case STATE_PLAY:{
                processStatePlay();
                break;
            }
            case STATE_OPTIONS:{
                processStateOptions();
            }
        }
    }
    return (0);
}

void basicInit(){
    JOY_init();
    JOY_setEventHandler(&joyEventHandler);
    SPR_init();
    currentState = STATE_MENU;
    //Cleanup
    VDP_clearPlane(BG_A, TRUE);
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x000000));
    cursor = SPR_addSprite(&gfx_cursor, 0, 0, 0);
    //Draw options
    u16 i = 0;
    for (; i < ELEMENTCOUNT(mainmenu_options); i++) {
        Option o = mainmenu_options[i];
        VDP_drawText(o.label,o.x,o.y);
    }
}

void drawMenuOptions(){
    SPR_reset();
    currentState = STATE_OPTIONS;
    //Cleanup
    VDP_clearPlane(BG_A, TRUE);
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x000000));
    cursor = SPR_addSprite(&gfx_cursor, 0, 0, 0);
    //Draw options
    u16 i = 0;
    for (; i < ELEMENTCOUNT(menu_options); i++) {
        Option o = menu_options[i];
        VDP_drawText(o.label,o.x,o.y);
    }
}

void updateCursorPosition(){
    SPR_setPosition(cursor, mainmenu_options[currentIndex].x*8-12, mainmenu_options[currentIndex].y*8);
}

void updateCursorOptionsPosition(){
    currentOptionsIndex = difficulty;
    SPR_setPosition(cursor, menu_options[currentOptionsIndex].x*8-12, menu_options[currentOptionsIndex].y*8);
}

void moveUp(){
    if(currentIndex > 0 && currentState == STATE_MENU){
        currentIndex--;
        updateCursorPosition();
    } else if (currentOptionsIndex > 0 && currentState == STATE_OPTIONS) {
        currentOptionsIndex--;
        difficulty--;
        updateCursorOptionsPosition();
    }
}

void moveDown(){
    if(currentIndex < ELEMENTCOUNT(mainmenu_options)-1 && currentState == STATE_MENU){
        currentIndex++;
        updateCursorPosition();
    } else if (currentOptionsIndex < ELEMENTCOUNT(menu_options)-1 && currentState == STATE_OPTIONS) {
        currentOptionsIndex++;
        difficulty++;
        updateCursorOptionsPosition();
    }
}

static void handleInput()
{
    if (currentState == STATE_PLAY) {
        u16 value = JOY_readJoypad(JOY_1);
        if (value & BUTTON_UP)
        {
            movePlayer(up);
            cont_pulsacion++;
        }
        else if (value & BUTTON_DOWN)
        {
            movePlayer(down);
            cont_pulsacion++;
        }
        else if (value & BUTTON_LEFT)
        {
            movePlayer(left);
            cont_pulsacion++;
        }
        else if (value & BUTTON_RIGHT)
        {
            movePlayer(right);
            cont_pulsacion++;
        } 
    }
}

void joyEventHandler(u16 joy, u16 changed, u16 state){
    if (currentState == STATE_MENU || currentState == STATE_OPTIONS){
        if (changed & state & BUTTON_UP)
        {
            moveUp();
        }
        else if(changed & state & BUTTON_DOWN){
            moveDown();
        }
        if((changed & state & BUTTON_A) && currentState == STATE_MENU){
            if(currentIndex == 0){
                currentState = STATE_PLAY;
            }else if (currentIndex == 1){
                currentState = STATE_OPTIONS;
            }
        }
        else if((changed & state & BUTTON_A) && currentState == STATE_OPTIONS)
        {
            currentState = STATE_MENU;
        }
    } 
}

void processStateMenu(){
    //Update
    while(currentState == STATE_MENU){
        updateCursorPosition();
        SPR_update();
        SYS_enableInts();
        SYS_doVBlankProcess();
    }
}

void processStatePlay(){
    VDP_clearPlane(BG_A, TRUE);
    SPR_clear();

    VDP_loadTileSet(floortiles.tileset, 1, DMA);
    PAL_setPalette(PAL1, floortiles.palette->data, DMA);
    PAL_setPalette(PAL2, spr_player.palette->data, DMA);
    loadLevel();
    cont_pulsacion = 0;
    while (currentState == STATE_PLAY)
    {
        if(!cont_pulsacion) handleInput();
        else cont_pulsacion++;
        if(cont_pulsacion>30) cont_pulsacion = 0;

        if(player.moving == TRUE){
            switch(player.dir){
                case up:
                    player.pos.y -= 1;
                    break;
                case down:
                    player.pos.y += 1;
                    break;
                case left:
                    player.pos.x -= 1;
                    break;
                case right:
                    player.pos.x += 1;
                    break;
                default:
                    break;
            }
        }
        if (player.pos.x % TILESIZE == 0 && player.pos.y % TILESIZE == 0){
            player.moving = FALSE;
        }
        SPR_setPosition(player.sprite, player.pos.x, player.pos.y);
        SPR_update();
        SYS_enableInts();
        SYS_doVBlankProcess();
    }
}

void processStateOptions(){
    drawMenuOptions();
    VDP_drawText("DIFFICULTY", 8, 8);
    VDP_drawText("Select difficulty and press 'A'", 1, 22);
    //Update
    while(currentState == STATE_OPTIONS){
        updateCursorOptionsPosition();
        SPR_update();
        SYS_enableInts();
        SYS_doVBlankProcess();
    }
    basicInit();
}

void loadLevel(){
    //Load the level
    u8 x = 0;
    u8 y = 0;
    u8 t = 0;

    SPR_reset();
    for(y = 0; y < 8; y++){
        for (x = 0; x < 8; x++){
            t = level1[y][x];
            if (t == SPAWN_TILE){
                //Spawn the player
                player.tilePos.x = x;
                player.tilePos.y = y;
                player.pos.x = player.tilePos.x * TILESIZE;
                player.pos.y = player.tilePos.y * TILESIZE;
                player.sprite = SPR_addSprite(&spr_player, player.pos.x, player.pos.y, TILE_ATTR(PAL2, 0, FALSE, FALSE));
                VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), x, y);
            } else{
                VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, t + 1), x, y);
            }
        }
    }
}

void movePlayer(moveDirection Direction){
    //Move the player
    if (player.moving == FALSE)
    {
        //Do movement things
        switch( Direction ){
        case up:
            if(player.tilePos.y > 0 && getTileAt(player.tilePos.x,player.tilePos.y-1) != SOLID_TILE){
                player.tilePos.y -= 1;
                player.moving = TRUE;
                player.dir = Direction;
                SPR_setAnim(player.sprite,ANIM_UP);
            }
            break;
        case down:
            if(player.tilePos.y < MAP_HEIGHT - 1 && getTileAt(player.tilePos.x,player.tilePos.y+1) != SOLID_TILE){
                player.tilePos.y += 1;
                player.moving = TRUE;
                player.dir = Direction;
                SPR_setAnim(player.sprite,ANIM_DOWN);
            }
            break;
        case left:
            if(player.tilePos.x > 0 && getTileAt(player.tilePos.x-1, player.tilePos.y) != SOLID_TILE){
                player.tilePos.x -= 1;
                player.moving = TRUE;
                player.dir = Direction;
                SPR_setAnim(player.sprite,ANIM_SIDE);
                SPR_setHFlip(player.sprite, TRUE);
            }
            break;
        case right:
            if(player.tilePos.x < MAP_WIDTH - 1 && getTileAt(player.tilePos.x+1, player.tilePos.y) != SOLID_TILE){
                player.tilePos.x += 1;
                player.moving = TRUE;
                player.dir = Direction;
                SPR_setAnim(player.sprite,ANIM_SIDE);
                SPR_setHFlip(player.sprite, FALSE);
            }
            break;
        default:
            break;
        }
    }
}

int getTileAt(u8 X, u8 Y)
{
    return *(&level1[0][0] + (Y * MAP_HEIGHT + X));
}