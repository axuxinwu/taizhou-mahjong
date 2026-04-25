#include "fancalc.h"
#include <algorithm>
#include <set>

namespace TaizhouMahjong {

void analyzePlayer(const Player& player, PlayerState& state) {
    std::map<uint8_t, int> counter;
    for (auto t : player.hand) counter[t]++;
    for (auto t : player.melds) counter[t]++;

    for (auto& pair : counter) {
        int cnt = pair.second;
        if (cnt >= 4) {
            // 简化：如果 melds 中包含此牌则可能是杠，这里粗略统计
            bool inMelds = std::count(player.melds.begin(), player.melds.end(), pair.first) >= 4;
            if (inMelds) state.mingGangCount++;
            else state.anGangCount++;
            cnt -= 4;
        }
        if (cnt == 3) {
            bool inHand = std::count(player.hand.begin(), player.hand.end(), pair.first) >= 3;
            if (inHand) state.anKeCount++;
            else state.mingKeCount++;
        }
    }
}

int baseHuPoints(const Player& player, const PlayerState& state) {
    int pts = 0;
    pts += state.mingKeCount * 2;
    pts += state.anKeCount * 4;
    pts += state.mingGangCount * 8;
    pts += state.anGangCount * 16;

    auto countTile = [&](uint8_t t) -> int {
        int cnt = std::count(player.hand.begin(), player.hand.end(), t)
                + std::count(player.melds.begin(), player.melds.end(), t);
        return cnt;
    };
    // 中发白对子2胡
    if (countTile(0x51) >= 2) pts += 2;
    if (countTile(0x52) >= 2) pts += 2;
    if (countTile(0x53) >= 2) pts += 2;
    // 门风
    if (countTile(0x31 + state.menFeng) >= 2) pts += 2;
    // 圈风（假设东）对子额外2胡
    if (countTile(0x31) >= 2) pts += 2;

    return pts;
}

int totalFans(const Player& player, const PlayerState& state, bool hasLaizi) {
    int fans = 0;
    std::set<Suit> suits;
    for (auto t : player.hand) {
        Suit s = suitOf(t);
        suits.insert(s);
    }
    for (auto t : player.melds) {
        Suit s = suitOf(t);
        suits.insert(s);
    }
    if (suits.size() == 1 && *suits.begin() <= Suit::WAN) {
        fans += 3; // 清一色
    } else if (suits.size() == 2) {
        bool hasNum = false, hasHonor = false;
        for (auto s : suits) {
            if (s <= Suit::WAN) hasNum = true;
            else hasHonor = true;
        }
        if (hasNum && hasHonor) fans += 1; // 混一色
    }
    if (!hasLaizi) fans += 1; // 无“得”
    return fans;
}

int finalHuPoints(const Player& player, bool isWinner, bool hasLaizi) {
    PlayerState state;
    analyzePlayer(player, state);
    int base = baseHuPoints(player, state);
    if (isWinner) base += 10;
    int fans = totalFans(player, state, hasLaizi);
    int raw = base * (1 << fans);
    return std::min(raw, 100); // 封顶100胡
}

} // namespace