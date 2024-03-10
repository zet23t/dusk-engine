#ifndef __GAME_STATE_H__
#define __GAME_STATE_H__
#include <inttypes.h>

typedef struct GameStateNode {
    uint32_t id;
    struct GameStateFSM *fsm;
    void (*onEnter)(struct GameStateNode *self);
    void (*onExit)(struct GameStateNode *self);
    void (*onUpdate)(struct GameStateNode *self);
    void (*onDraw)(struct GameStateNode *self);
    void (*onDestroy)(struct GameStateNode *self);
} GameStateNode;

typedef struct GameStateFSM {
    GameStateNode *states;
    GameStateNode *currentState;
    GameStateNode *nextState;
} GameStateFSM;

void GameStateFSMInit(GameStateFSM *self, GameStateNode *states);
void GameStateFSMUpdate(GameStateFSM *self);
void GameStateFSMChangeState(GameStateFSM *self, uint32_t id);
void GameStateFSMDestroy(GameStateFSM *self);

#endif