#include "engine.h"
#include "winjudge.h"
#include "fancalc.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <sstream>

namespace TaizhouMahjong {

std::string tileName(uint8_t t) {
    static const char* feng[] = {"", "东","南","西","北"};
    static const char* jian[] = {"", "中","发","白"};
    Suit s = suitOf(t);
    uint8_t v = valueOf(t);
    switch (s) {
        case TONG: return std::to_string(v) + "筒";
        case TIAO: return std::to_string(v) + "条";
        case WAN:  return std::to_string(v) + "万";
        case FENG: return feng[v];
        case JIAN: return jian[v];
        default: return "?";
    }
}

// -------- TileWall --------
TileWall::TileWall() { reset(); }

void TileWall::reset() {
    tiles.clear();
    for (int s = 0; s <= 2; ++s)
        for (int v = 1; v <= 9; ++v)
            for (int i = 0; i < 4; ++i)
                tiles.push_back(makeTile(static_cast<Suit>(s), v));
    for (int v = 1; v <= 4; ++v)
        for (int i = 0; i < 4; ++i)
            tiles.push_back(makeTile(FENG, v));
    for (int v = 1; v <= 3; ++v)
        for (int i = 0; i < 4; ++i)
            tiles.push_back(makeTile(JIAN, v));
    for (int i = 0; i < 8; ++i)
        tiles.push_back(0xF0 + i); // 花牌
    shuffle();
}

void TileWall::shuffle() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(tiles.begin(), tiles.end(), std::mt19937(seed));
}

uint8_t TileWall::draw() {
    if (tiles.empty()) return 0;
    uint8_t t = tiles.back();
    tiles.pop_back();
    if ((t & 0xF0) == 0xF0) {   // 花牌补牌
        if (!tiles.empty()) {
            t = tiles.back();
            tiles.pop_back();
        } else t = 0;
    }
    return t;
}

// -------- Player --------
void Player::sortHand() {
    std::sort(hand.begin(), hand.end());
}

// -------- GameEngine --------
GameEngine::GameEngine() : wall_(std::make_unique<TileWall>()) {}

void GameEngine::addPlayer(std::shared_ptr<Player> p) {
    players_.push_back(p);
    if (players_.size() == 1) p->isDealer = true;
}

void GameEngine::start() {
    if (players_.size() != 4) return;
    dealerIdx = 0;
    currentPlayerIdx = dealerIdx;
    currentPhase = Phase::DEAL;
    dealTiles();
}

void GameEngine::dealTiles() {
    wall_->reset();
    for (int i = 0; i < HAND_SIZE; ++i)
        for (auto& p : players_)
            p->hand.push_back(wall_->draw());
    players_[dealerIdx]->hand.push_back(wall_->draw());
    for (auto& p : players_)
        p->sortHand();
    currentPhase = Phase::DRAW;
    if (onEvent) onEvent(dealerIdx, "deal_done");
}

uint8_t GameEngine::drawTile(int playerIdx) {
    if (playerIdx != currentPlayerIdx || currentPhase != Phase::DRAW) return 0;
    uint8_t t = wall_->draw();
    players_[playerIdx]->hand.push_back(t);
    players_[playerIdx]->sortHand();
    currentPhase = Phase::DISCARD;
    if (onEvent) onEvent(playerIdx, "draw_tile " + std::to_string(t));
    return t;
}

bool GameEngine::discardTile(int playerIdx, uint8_t tile) {
    if (playerIdx != currentPlayerIdx || currentPhase != Phase::DISCARD) return false;
    auto& hand = players_[playerIdx]->hand;
    auto it = std::find(hand.begin(), hand.end(), tile);
    if (it == hand.end()) return false;
    hand.erase(it);
    lastDiscardTile = tile;
    currentPhase = Phase::CHANCE;
    waitForResponse = true;
    if (onEvent) onEvent(playerIdx, "discard " + std::to_string(tile));
    return true;
}

bool GameEngine::chi(int playerIdx, uint8_t targetTile, uint8_t with1, uint8_t with2) {
    if (currentPhase != Phase::CHANCE || playerIdx == currentPlayerIdx) return false;
    if ((currentPlayerIdx + 1) % 4 != playerIdx) return false;

    auto& hand = players_[playerIdx]->hand;
    auto it1 = std::find(hand.begin(), hand.end(), with1);
    auto it2 = std::find(hand.begin(), hand.end(), with2);
    if (it1 == hand.end() || it2 == hand.end() || it1 == it2) return false;

    std::vector<uint8_t> triple = { targetTile, with1, with2 };
    std::sort(triple.begin(), triple.end());
    if (!(triple[0] + 1 == triple[1] && triple[1] + 1 == triple[2])) return false;
    if (suitOf(triple[0]) != suitOf(triple[1]) || suitOf(triple[1]) != suitOf(triple[2])) return false;

    hand.erase(it1);
    hand.erase(std::find(hand.begin(), hand.end(), with2)); // it2 might be invalid, re-search
    for (auto t : triple)
        players_[playerIdx]->melds.push_back(t);

    waitForResponse = false;
    currentPlayerIdx = playerIdx;
    currentPhase = Phase::DRAW;
    if (onEvent) onEvent(playerIdx, "chi");
    return true;
}

bool GameEngine::peng(int playerIdx, uint8_t tile) {
    if (currentPhase != Phase::CHANCE || playerIdx == currentPlayerIdx) return false;
    auto& hand = players_[playerIdx]->hand;
    if (std::count(hand.begin(), hand.end(), tile) < 2) return false;
    for (int i = 0; i < 2; ++i)
        hand.erase(std::find(hand.begin(), hand.end(), tile));
    for (int i = 0; i < 3; ++i)
        players_[playerIdx]->melds.push_back(tile);

    waitForResponse = false;
    currentPlayerIdx = playerIdx;
    currentPhase = Phase::DRAW;
    if (onEvent) onEvent(playerIdx, "peng");
    return true;
}

bool GameEngine::mingGang(int playerIdx, uint8_t tile) {
    if (currentPhase == Phase::CHANCE && playerIdx != currentPlayerIdx) {
        auto& hand = players_[playerIdx]->hand;
        if (std::count(hand.begin(), hand.end(), tile) < 3) return false;
        for (int i = 0; i < 3; ++i)
            hand.erase(std::find(hand.begin(), hand.end(), tile));
        for (int i = 0; i < 4; ++i)
            players_[playerIdx]->melds.push_back(tile);
        waitForResponse = false;
        currentPlayerIdx = playerIdx;
        currentPhase = Phase::DRAW;
        if (onEvent) onEvent(playerIdx, "ming_gang");
        return true;
    }
    // 补杠逻辑可后续完善
    return false;
}

bool GameEngine::anGang(int playerIdx, uint8_t tile) {
    if (currentPhase != Phase::DRAW || playerIdx != currentPlayerIdx) return false;
    auto& hand = players_[playerIdx]->hand;
    if (std::count(hand.begin(), hand.end(), tile) < 4) return false;
    for (int i = 0; i < 4; ++i)
        hand.erase(std::find(hand.begin(), hand.end(), tile));
    for (int i = 0; i < 4; ++i)
        players_[playerIdx]->melds.push_back(tile);
    currentPhase = Phase::DRAW; // 需要补牌，drawTile内处理
    if (onEvent) onEvent(playerIdx, "an_gang");
    return true;
}

bool GameEngine::hu(int playerIdx, uint8_t tile, bool isSelfDraw) {
    if (isSelfDraw && (playerIdx != currentPlayerIdx || currentPhase != Phase::DISCARD)) return false;
    if (!isSelfDraw && (currentPhase != Phase::CHANCE || playerIdx == currentPlayerIdx)) return false;

    if (!canHu(playerIdx, tile, isSelfDraw)) return false;

    settleRound(playerIdx, isSelfDraw ? playerIdx : currentPlayerIdx, isSelfDraw);
    currentPhase = Phase::END;
    if (onEvent) onEvent(playerIdx, "hu");
    return true;
}

bool GameEngine::canHu(int playerIdx, uint8_t newTile, bool isSelfDraw) {
    auto p = players_[playerIdx];
    std::vector<uint8_t> handCopy = p->hand;
    if (!isSelfDraw) handCopy.push_back(newTile);
    // 简化：无赖子
    std::sort(handCopy.begin(), handCopy.end());
    return canWinRegular(handCopy);
}

void GameEngine::settleRound(int winnerIdx, int loserIdx, bool isSelfDraw) {
    // 结算逻辑，使用 fancalc 计算胡数
    // 此处仅通知事件
    if (onEvent) onEvent(winnerIdx, "win");
}

} // namespace