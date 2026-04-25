#ifndef FANCALC_H
#define FANCALC_H

#include <vector>
#include <cstdint>
#include "engine.h"

namespace TaizhouMahjong {

struct PlayerState {
    int mingKeCount = 0;
    int anKeCount  = 0;
    int mingGangCount = 0;
    int anGangCount = 0;
    int menFeng = 0; // 0东1南2西3北
};

void analyzePlayer(const Player& player, PlayerState& state);
int baseHuPoints(const Player& player, const PlayerState& state);
int totalFans(const Player& player, const PlayerState& state, bool hasLaizi);
int finalHuPoints(const Player& player, bool isWinner, bool hasLaizi);

}

#endif