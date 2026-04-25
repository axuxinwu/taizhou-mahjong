#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <cstdint>

namespace TaizhouMahjong {

enum Suit : uint8_t { TONG = 0, TIAO = 1, WAN = 2, FENG = 3, JIAN = 4 };
inline uint8_t makeTile(Suit s, uint8_t v) { return (static_cast<uint8_t>(s) << 4) | (v & 0x0F); }
inline Suit suitOf(uint8_t t) { return static_cast<Suit>(t >> 4); }
inline uint8_t valueOf(uint8_t t) { return t & 0x0F; }
std::string tileName(uint8_t tile);

constexpr int HAND_SIZE = 13;

class TileWall {
public:
    TileWall();
    void   shuffle();
    uint8_t draw();
    size_t remaining() const { return tiles.size(); }
    bool   empty() const { return tiles.empty(); }
    void   reset();
private:
    std::vector<uint8_t> tiles;
};

struct Player {
    int id;
    std::string name;
    std::vector<uint8_t> hand;
    std::vector<uint8_t> melds;   // 简化存储碰/杠/吃牌
    bool isDealer = false;
    void sortHand();
};

enum class Phase {
    DEAL,
    DRAW,
    DISCARD,
    CHANCE,
    SETTLE,
    END
};

class GameEngine {
public:
    GameEngine();
    void addPlayer(std::shared_ptr<Player> p);
    void start();

    uint8_t drawTile(int playerIdx);
    bool    discardTile(int playerIdx, uint8_t tile);
    bool    chi(int playerIdx, uint8_t targetTile, uint8_t with1, uint8_t with2);
    bool    peng(int playerIdx, uint8_t tile);
    bool    mingGang(int playerIdx, uint8_t tile);
    bool    anGang(int playerIdx, uint8_t tile);
    bool    hu(int playerIdx, uint8_t tile, bool isSelfDraw);

    Phase   phase() const { return currentPhase; }
    int     currentPlayer() const { return currentPlayerIdx; }
    int     dealer() const { return dealerIdx; }
    std::vector<std::shared_ptr<Player>> players() const { return players_; }
    uint8_t lastDiscard() const { return lastDiscardTile; }

    std::function<void(int playerIdx, const std::string& event)> onEvent;

private:
    void dealTiles();
    void nextTurn();
    bool canHu(int playerIdx, uint8_t newTile, bool isSelfDraw);
    void settleRound(int winnerIdx, int loserIdx, bool isSelfDraw);

    std::unique_ptr<TileWall> wall_;
    std::vector<std::shared_ptr<Player>> players_;
    Phase currentPhase = Phase::DEAL;
    int   dealerIdx = 0;
    int   currentPlayerIdx = 0;
    uint8_t lastDiscardTile = 0;
    bool   waitForResponse = false;
};

}

#endif