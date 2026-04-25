#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QList>
#include <QMap>

class MahjongServer : public QObject {
    Q_OBJECT
public:
    explicit MahjongServer(quint16 port, QObject* parent = nullptr);
    bool start();
    void stop();
    void sendToAll(const QByteArray& data);
    void sendTo(int clientId, const QByteArray& data);
    void sendToAllExcept(int exceptId, const QByteArray& data);

signals:
    void clientConnected(int id);
    void clientDisconnected(int id);
    void dataReceived(int clientId, QByteArray data);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer* server;
    QList<QTcpSocket*> clients;
    QMap<QTcpSocket*, int> clientIds;
    int nextId = 0;
};

class MahjongClient : public QObject {
    Q_OBJECT
public:
    explicit MahjongClient(QObject* parent = nullptr);
    bool connectToHost(const QString& ip, quint16 port);
    void disconnect();
    void send(const QByteArray& data);

signals:
    void connected();
    void disconnected();
    void dataReceived(QByteArray data);

private slots:
    void onReadyRead();

private:
    QTcpSocket* socket;
};

#endif