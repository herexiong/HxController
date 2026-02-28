#include "localserverhost.h"
#include <QDebug>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTime>

LocalServerHost::LocalServerHost(QObject *parent)
    : QObject(parent)
    , m_nextId(1)
{
    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::WorldAccessOption);
    
    QLocalServer::removeServer("MediaControlHubPipe");
    if (!m_server->listen("MediaControlHubPipe")) {
        qWarning() << "[LocalServerHost] Failed to start server:" << m_server->errorString();
        return;
    }

    connect(m_server, &QLocalServer::newConnection, this, &LocalServerHost::onNewConnection);
    qDebug() << "[LocalServerHost] Server listening on MediaControlHubPipe";
}

LocalServerHost::~LocalServerHost()
{
    if (m_server->isListening()) {
        m_server->close();
    }
    qDeleteAll(m_clients);
    m_clients.clear();
    qDebug() << "[LocalServerHost] Server destroyed";
}

void LocalServerHost::onNewConnection()
{
    QLocalSocket *clientSocket = m_server->nextPendingConnection();
    if (!clientSocket) return;

    connect(clientSocket, &QLocalSocket::readyRead, this, &LocalServerHost::onReadyRead);
    connect(clientSocket, &QLocalSocket::disconnected, this, &LocalServerHost::onDisconnected);

    m_clients.append(clientSocket);
    
    QString msg = QString("[LocalServerHost] Client connected. Total clients: %1").arg(m_clients.size());
    qDebug() << msg;
    emit logMessage(QTime::currentTime().toString("HH:mm:ss") + " 🟢 " + msg);
    emit serverStatusChanged();
}

void LocalServerHost::onDisconnected()
{
    QLocalSocket *clientSocket = qobject_cast<QLocalSocket*>(sender());
    if (!clientSocket) return;

    m_clients.removeAll(clientSocket);
    clientSocket->deleteLater();

    QString msg = QString("[LocalServerHost] Client disconnected. Total clients: %1").arg(m_clients.size());
    qDebug() << msg;
    emit logMessage(QTime::currentTime().toString("HH:mm:ss") + " 🔴 " + msg);
    emit serverStatusChanged();
}

void LocalServerHost::onReadyRead()
{
    QLocalSocket *clientSocket = qobject_cast<QLocalSocket*>(sender());
    if (!clientSocket) return;

    while (clientSocket->canReadLine()) {
        QByteArray jsonBytes = clientSocket->readLine().trimmed();
        if (jsonBytes.isEmpty()) continue;

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "[LocalServerHost] JSON parse error:" << parseError.errorString();
            continue;
        }
        if (!doc.isObject()) {
            qWarning() << "[LocalServerHost] Message is not a JSON object";
            continue;
        }

        QJsonObject msgObj = doc.object();
        int displayId = msgObj["id"].toInt(msgObj["replyTo"].toInt());
        QString summary = QString::fromUtf8("⬇ %1/%2 id=%3 (%4 bytes) %5")
            .arg(msgObj["type"].toString(), msgObj["name"].toString())
            .arg(displayId)
            .arg(jsonBytes.size())
            .arg(QString::fromUtf8(jsonBytes));
            
        emit logMessage(QTime::currentTime().toString("HH:mm:ss") + " " + summary);
        qDebug().noquote() << "[LocalServerHost]" << summary;
        
        emit messageReceived(jsonBytes);
    }
}

void LocalServerHost::broadcastMessage(const QByteArray &jsonBytes)
{
    if (m_clients.isEmpty()) return;
    
    QByteArray outBytes = jsonBytes;
    if (!outBytes.endsWith('\n')) {
        outBytes.append('\n');
    }

    for (QLocalSocket *client : m_clients) {
        if (client->state() == QLocalSocket::ConnectedState) {
            client->write(outBytes);
            client->flush();
        }
    }
}

int LocalServerHost::sendCommand(const QString &name, const QByteArray &payloadJson)
{
    int id = m_nextId++;
    QJsonObject msg;
    msg["type"] = "cmd";
    msg["name"] = name;
    msg["id"]   = id;

    QJsonDocument payloadDoc = QJsonDocument::fromJson(payloadJson);
    msg["payload"] = payloadDoc.object();

    QByteArray outBytes = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    broadcastMessage(outBytes);

    QString summary = QString::fromUtf8("⬆ cmd/%1 id=%2 (%3 bytes) %4")
        .arg(name).arg(id).arg(outBytes.size()).arg(QString::fromUtf8(outBytes));
    emit logMessage(QTime::currentTime().toString("HH:mm:ss") + " " + summary);
    qDebug().noquote() << "[LocalServerHost]" << summary;
    
    return id;
}
