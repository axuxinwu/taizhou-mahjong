#include "gamecontroller.h"
#include <QDebug>

GameController::GameController(QObject* parent) : QObject(parent) {
    setupEngineEvents();
}

void GameController::setupEngineEvents() {
    engine.onEvent = [this](int playerIdx, const std::string& event) {
        Q_UNUSED(playerIdx);
        QString str = QString::fromStdString(event);
        qDebug() << "Engine:" << str;
        if (str == "deal_done") {
            updateHandModel();
            emit turnChanged();
        } else if (str.startsWith("draw_tile") || str.startsWith("discard")) {
            updateHandModel();
            if (str.startsWith("draw_tile")) waiting = true;  // 自己回合，等待出牌
        } else if (str == "chi" || str == "peng" || str == "hu") {
            updateHandModel();
            waiting = false;
        }
    };
}

void GameController::startNewGame() {
    for (int i = 0; i < 4; ++i) {
        auto p = std::make_shared<TaizhouMahjong::Player>();
        p->id = i;
        p->name = "玩家" + std::to_string(i+1);
        engine.addPlayer(p);
    }
    engine.start();
    updateHandModel();
    emit handChanged();
}

void GameController::hostGame(int port) {
    isServer = true;
    myPlayerIdx = 0;
    server = std::make_unique<MahjongServer>(port);
    connect(server.get(), &MahjongServer::clientConnected, this, [this](int id) {
        emit message("玩家加入，ID：" + QString::number(id));
    });
    connect(server.get(), &MahjongServer::dataReceived, this, &GameController::processServerData);
    server->start();
    emit message("服务器已启动，等待其他玩家...");
}

void GameController::joinGame(QString ip, int port) {
    isServer = false;
    client = std::make_unique<MahjongClient>();
    connect(client.get(), &MahjongClient::dataReceived, this, &GameController::processClientData);
    connect(client.get(), &MahjongClient::connected, this, [this]() {
        emit message("已连接到服务器");
    });
    client->connectToHost(ip, port);
}

void GameController::drawTile() {
    if (server || client) {
        sendAction(MahjongProtocol::ActionType::DRAW);
    } else {
        engine.drawTile(myPlayerIdx);
    }
    updateHandModel();
}

void GameController::discardTile(int tileCode) {
    uint8_t tile = static_cast<uint8_t>(tileCode);
    if (server || client) {
        sendAction(MahjongProtocol::ActionType::DISCARD, tile);
    } else {
        engine.discardTile(myPlayerIdx, tile);
    }
    updateHandModel();
    waiting = false;
}

void GameController::chi() {
    // 实际需要选择组合，此处占位
    if (server || client) {
        sendAction(MahjongProtocol::ActionType::CHI);
    }
}

void GameController::peng() {
    if (server || client) {
        sendAction(MahjongProtocol::ActionType::PENG, engine.lastDiscard());
    } else {
        engine.peng(myPlayerIdx, engine.lastDiscard());
    }
    updateHandModel();
}

void GameController::gang() {
    if (server || client) {
        sendAction(MahjongProtocol::ActionType::MING_GANG, engine.lastDiscard());
    } else {
        engine.mingGang(myPlayerIdx, engine.lastDiscard());
    }
    updateHandModel();
}

void GameController::hu() {
    if (server || client) {
        sendAction(MahjongProtocol::ActionType::HU, engine.lastDiscard());
    } else {
        engine.hu(myPlayerIdx, engine.lastDiscard(), false);
    }
}

void GameController::pass() {
    // 过，什么都不做
    sending = false;
}

QString GameController::tileToString(int code) {
    return QString::fromStdString(TaizhouMahjong::tileName(static_cast<uint8_t>(code)));
}

void GameController::updateHandModel() {
    handModel.clear();
    if (engine.players().empty()) return;
    auto& hand = engine.players()[myPlayerIdx]->hand;
    for (uint8_t t : hand)
        handModel.append(static_cast<int>(t));
    emit handChanged();
}

QVariantList GameController::playerHand() const { return handModel; }
QVariantList GameController::discardedTiles() const { return discardModel; }
int GameController::currentPlayerIdx() const { return engine.currentPlayer(); }
bool GameController::waitingForAction() const { return waiting; }

void GameController::sendAction(MahjongProtocol::ActionType act, uint8_t tile,
                                uint8_t tile2, uint8_t tile3) {
    auto msg = MahjongProtocol::actionMsg(myPlayerIdx, act, tile, tile2, tile3);
    QByteArray data = MahjongProtocol::createMessage(msg);
    if (isServer) {
        server->sendToAllExcept(myPlayerIdx, data);
        // 同时本地执行，调用 engine 对应方法
        // ... 可补充
    } else {
        client->send(data);
    }
}

void GameController::processServerData(int clientId, QByteArray data) {
    QJsonObject obj = MahjongProtocol::parseMessage(data);
    QString type = obj["type"].toString();
    if (type == "action") {
        // 转发给其他客户端
        server->sendToAllExcept(clientId, data);
        int player = obj["player"].toInt();
        auto act = static_cast<MahjongProtocol::ActionType>(obj["action"].toInt());
        uint8_t tile = obj["tile"].toInt();
        // 在引擎中执行
        switch (act) {
            case MahjongProtocol::ActionType::DRAW:
                engine.drawTile(player);
                break;
            case MahjongProtocol::ActionType::DISCARD:
                engine.discardTile(player, tile);
                discardModel.append(tile);
                break;
            // ... 其他 case 类似
        }
        updateHandModel();
    }
}

void GameController::processClientData(QByteArray data) {
    QJsonObject obj = MahjongProtocol::parseMessage(data);
    QString type = obj["type"].toString();
    if (type == "deal") {
        QJsonArray arr = obj["hand"].toArray();
        std::vector<uint8_t> hand;
        for (auto v : arr) hand.push_back(static_cast<uint8_t>(v.toInt()));
        myPlayerIdx = obj["myIdx"].toInt();
        engine.players()[myPlayerIdx]->hand = hand;
        engine.players()[myPlayerIdx]->sortHand();
        updateHandModel();
    } else if (type == "action") {
        int player = obj["player"].toInt();
        auto act = static_cast<MahjongProtocol::ActionType>(obj["action"].toInt());
        uint8_t tile = obj["tile"].toInt();
        if (player != myPlayerIdx) {
            // 其他玩家动作，更新本地引擎
            switch (act) {
                case MahjongProtocol::ActionType::DRAW:
                    engine.drawTile(player);
                    break;
                case MahjongProtocol::ActionType::DISCARD:
                    engine.discardTile(player, tile);
                    discardModel.append(tile);
                    emit discardChanged();
                    break;
                // ... 其他
            }
        }
        // 自己回合时显示操作按钮
        if (player == (myPlayerIdx + 1) % 4 && act == MahjongProtocol::ActionType::DISCARD) {
            waiting = true;
            emit actionRequested();
        }
    }
}