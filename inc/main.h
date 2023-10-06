#ifndef MAIN_H
#define MAIN_H

// Enumeración para las direcciones de movimiento
typedef enum {
    up,
    down,
    left,
    right,
    none
} moveDirection;

// Declaraciones de funciones
void basicInit();
void drawMenuOptions();
void updateCursorPosition();
void updateCursorOptionsPosition();
void moveUp();
void moveDown();
void joyEventHandler(u16 joy, u16 changed, u16 state);
void processStateMenu();
void processStatePlay();
void processStateOptions();
void loadLevel();
void movePlayer(moveDirection Direction);
int getTileAt(u8 X, u8 Y);

#endif /* MAIN_H */