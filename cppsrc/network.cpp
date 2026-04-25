#include "network.h"
#include <QDataStream>

MahjongServer::MahjongServer(quint16 port, QObject* parent)
    : QObject(parent), server(new QTcpServer(this))
{
    connect(server, &QTcpServer::newConnection, this, &MahjongServer::onNewConnection);
}

bool MahjongServer::start() {
    return server->listen(QHostAddress::Any, port);
}

void MahjongServer::stop() {
    server->close();
    for (auto* sock : clients)
        sock->disconnectFromHost();
    clients.clear();
    clientIds.clear();
}

void MahjongServer::sendToAll(const QByteArray& data) {
    for (auto* sock : clients)
        sock->write(data);
}

void MahjongServer::sendTo(int clientId, const QByteArray& data) {
    auto it = clientIds.key(clientId);
    if (it) it->write(data);
}

void MahjongServer::sendToAllExcept(int exceptId, const QByteArray& data) {
    for (auto* sock : clients) {
        if (clientIds.value(sock) != exceptId)
            sock->write(data);
    }
}

void MahjongServer::onNewConnection() {
    while (server->hasPendingConnections()) {
        QTcpSocket* sock = server->nextPendingConnection();
        int id = nextId++;
        clients.append(sock);
        clientIds.insert(sock, id);
        connect(sock, &QTcpSocket::readyRead, this, &MahjongServer::onReadyRead);
        connect(sock, &QTcpSocket::disconnected, this, &MahjongServer::onDisconnected);
        emit clientConnected(id);
    }
}

void MahjongServer::onReadyRead() {
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;
    int id = clientIds.value(sock, -1);
    emit dataReceived(id, sock->readAll());
}

void MahjongServer::onDisconnected() {
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;
    int id = clientIds.value(sock);
    clients.removeOne(sock);
    clientIds.remove(sock);
    sock->deleteLater();
    emit clientDisconnected(id);
}

// -------- Client --------
MahjongClient::MahjongClient(QObject* parent)
    : QObject(parent), socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::readyRead, this, &MahjongClient::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &MahjongClient::connected);
    connect(socket, &QTcpSocket::disconnected, this, &MahjongClient::disconnected);
}

bool MahjongClient::connectToHost(const QString& ip, quint16 port) {
    socket->connectToHost(ip, port);
    return socket->waitForConnected(3000);
}

void MahjongClient::disconnect() {
    socket->disconnectFromHost();
}

void MahjongClient::send(const QByteArray& data) {
    socket->write(data);
}

void MahjongClient::onReadyRead() {
    emit dataReceived(socket->readAll());
}