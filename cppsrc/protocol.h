#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QJsonObject>
#include <QByteArray>
#include <vector>
#include <cstdint>

namespace MahjongProtocol {

enum class ActionType {
    DRAW = 0,
    DISCARD,
    CHI,
    PENG,
    MING_GANG,
    AN_GANG,
    HU,
    PASS,
    DEAL
};

QByteArray createMessage(const QJsonObject& obj);
QJsonObject parseMessage(const QByteArray& data);

QJsonObject actionMsg(int playerId, ActionType act, uint8_t tile = 0,
                      uint8_t tile2 = 0, uint8_t tile3 = 0);
QJsonObject dealMsg(const std::vector<uint8_t>& hand, int dealerPos, int myIdx);
QJsonObject syncMsg(const std::vector<uint8_t>& hand, int currentPlayer,
                    uint8_t lastDiscard, int remaining, int myIdx);

} // namespace

#endif