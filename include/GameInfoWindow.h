#pragma once
#include <string>
#include "Game.h"

class GameInfoWindow {
public:
    GameInfoWindow();
    ~GameInfoWindow();
    
    void Render();
    void SetGame(const Game* game);
    void ClearGame();
    
private:
    const Game* currentGame;
    bool hasGame;
    
    void RenderGameInfo();
    void RenderNoGameMessage();
    const char* GetFormatString(GameFormat format) const;
}; 