#include "winjudge.h"
#include "engine.h"
#include <algorithm>
#include <functional>

namespace TaizhouMahjong {

static bool canDecompose(std::vector<uint8_t> cards);

bool canWinRegular(const std::vector<uint8_t>& hand) {
    if (hand.size() % 3 != 2) return false;
    std::vector<uint8_t> cards = hand;
    std::sort(cards.begin(), cards.end());
    for (size_t i = 0; i < cards.size() - 1; ++i) {
        if (cards[i] == cards[i+1]) {
            std::vector<uint8_t> rest;
            for (size_t j = 0; j < i; ++j) rest.push_back(cards[j]);
            for (size_t j = i+2; j < cards.size(); ++j) rest.push_back(cards[j]);
            if (canDecompose(rest)) return true;
            while (i+1 < cards.size() && cards[i] == cards[i+1]) ++i;
        }
    }
    return false;
}

static bool canDecompose(std::vector<uint8_t> cards) {
    if (cards.empty()) return true;
    uint8_t first = cards[0];
    if (cards.size() >= 3 && cards[1] == first && cards[2] == first) {
        std::vector<uint8_t> rest(cards.begin()+3, cards.end());
        if (canDecompose(rest)) return true;
    }
    Suit s = suitOf(first);
    uint8_t v = valueOf(first);
    if (s <= Suit::WAN && v <= 7) {
        auto it2 = std::find(cards.begin(), cards.end(), makeTile(s, v+1));
        auto it3 = std::find(cards.begin(), cards.end(), makeTile(s, v+2));
        if (it2 != cards.end() && it3 != cards.end()) {
            std::vector<uint8_t> rest;
            for (auto it = cards.begin(); it != cards.end(); ++it)
                if (it != it2 && it != it3) rest.push_back(*it);
            if (canDecompose(rest)) return true;
        }
    }
    return false;
}

bool canWinWithLaizi(const std::vector<uint8_t>& hand, uint8_t laiziTile) {
    // 简化：直接遍历所有可能替换值
    int laiziCount = 0;
    std::vector<uint8_t> nonLaizi;
    for (auto t : hand) {
        if (t == laiziTile) laiziCount++;
        else nonLaizi.push_back(t);
    }
    std::sort(nonLaizi.begin(), nonLaizi.end());
    std::function<bool(std::vector<uint8_t>, int)> dfs = [&](std::vector<uint8_t> cur, int lcnt) -> bool {
        if (lcnt == 0) return canWinRegular(cur);
        for (int s = 0; s <= 4; ++s) {
            int maxV = (s <= 2) ? 9 : (s == 3 ? 4 : 3);
            for (int v = 1; v <= maxV; ++v) {
                uint8_t t = makeTile(static_cast<Suit>(s), v);
                cur.push_back(t);
                std::sort(cur.begin(), cur.end());
                if (dfs(cur, lcnt-1)) return true;
                cur.erase(std::find(cur.begin(), cur.end(), t));
            }
        }
        return false;
    };
    return dfs(nonLaizi, laiziCount);
}

} // namespace