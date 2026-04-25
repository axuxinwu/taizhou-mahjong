#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QVariantList>
#include "engine.h"
#include "network.h"
#include "protocol.h"

class GameController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList playerHand READ playerHand NOTIFY handChanged)
    Q_PROPERTY(QVariantList discardedTiles READ discardedTiles NOTIFY discardChanged)
    Q_PROPERTY(int currentPlayerIdx READ currentPlayerIdx NOTIFY turnChanged)
    Q_PROPERTY(bool waitingForAction READ waitingForAction NOTIFY actionRequested)

public:
    explicit GameController(QObject* parent = nullptr);

    Q_INVOKABLE void startNewGame();
    Q_INVOKABLE void hostGame(int port = 12345);
    Q_INVOKABLE void joinGame(QString ip, int port = 12345);
    Q_INVOKABLE void drawTile();
    Q_INVOKABLE void discardTile(int tileCode);
    Q_INVOKABLE void chi();
    Q_INVOKABLE void peng();
    Q_INVOKABLE void gang();
    Q_INVOKABLE void hu();
    Q_INVOKABLE void pass();
    Q_INVOKABLE QString tileToString(int code);

    QVariantList playerHand() const;
    QVariantList discardedTiles() const;
    int currentPlayerIdx() const;
    bool waitingForAction() const;

signals:
    void handChanged();
    void discardChanged();
    void turnChanged();
    void actionRequested();
    void message(QString msg);

private:
    void updateHandModel();
    void sendAction(MahjongProtocol::ActionType act, uint8_t tile = 0,
                    uint8_t tile2 = 0, uint8_t tile3 = 0);
    void processServerData(int clientId, QByteArray data);
    void processClientData(QByteArray data);
    void setupEngineEvents();

    TaizhouMahjong::GameEngine engine;
    std::unique_ptr<MahjongServer> server;
    std::unique_ptr<MahjongClient> client;
    bool isServer = false;
    int myPlayerIdx = 0;
    QVariantList handModel;
    QVariantList discardModel;
    bool waiting = false;
};

#endif