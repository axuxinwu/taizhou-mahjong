#include "protocol.h"
#include <QJsonDocument>
#include <QJsonArray>

namespace MahjongProtocol {

QByteArray createMessage(const QJsonObject& obj) {
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

QJsonObject parseMessage(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    return doc.object();
}

QJsonObject actionMsg(int playerId, ActionType act, uint8_t tile,
                     uint8_t tile2, uint8_t tile3) {
    QJsonObject obj;
    obj["type"] = "action";
    obj["player"] = playerId;
    obj["action"] = static_cast<int>(act);
    if (tile) obj["tile"] = tile;
    if (tile2) obj["tile2"] = tile2;
    if (tile3) obj["tile3"] = tile3;
    return obj;
}

QJsonObject dealMsg(const std::vector<uint8_t>& hand, int dealerPos, int myIdx) {
    QJsonObject obj;
    obj["type"] = "deal";
    QJsonArray arr;
    for (auto t : hand) arr.append(t);
    obj["hand"] = arr;
    obj["dealer"] = dealerPos;
    obj["myIdx"] = myIdx;
    return obj;
}

QJsonObject syncMsg(const std::vector<uint8_t>& hand, int currentPlayer,
                    uint8_t lastDiscard, int remaining, int myIdx) {
    QJsonObject obj;
    obj["type"] = "sync";
    QJsonArray arr;
    for (auto t : hand) arr.append(t);
    obj["hand"] = arr;
    obj["currentPlayer"] = currentPlayer;
    obj["lastDiscard"] = lastDiscard;
    obj["remaining"] = remaining;
    obj["myIdx"] = myIdx;
    return obj;
}

} // namespace