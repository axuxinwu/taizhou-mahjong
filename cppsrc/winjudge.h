#ifndef WINJUDGE_H
#define WINJUDGE_H

#include <vector>
#include <cstdint>

namespace TaizhouMahjong {

bool canWinRegular(const std::vector<uint8_t>& hand);
bool canWinWithLaizi(const std::vector<uint8_t>& hand, uint8_t laiziTile);

}

#endif