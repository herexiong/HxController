#ifndef LOCALSERVERHOST_H
#define LOCALSERVERHOST_H

#include <QObject>
#include <QByteArray>
#include <QLocalServer>
#include <QLocalSocket>
#include <QList>

class LocalServerHost : public QObject
{
    Q_OBJECT

public:
    explicit LocalServerHost(QObject *parent = nullptr);
    ~LocalServerHost();

    // 发送命令到代理层（自动分配 id）
    int sendCommand(const QString &name, const QByteArray &payloadJson = "{}");
    bool isConnected() const { return !m_clients.isEmpty(); }
    bool isListening() const { return m_server && m_server->isListening(); }
    int clientCount() const { return m_clients.size(); }

signals:
    void messageReceived(const QByteArray &rawJson);
    void logMessage(const QString &log);
    void serverStatusChanged(); // emits when connected state changes

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void broadcastMessage(const QByteArray &jsonBytes);

    QLocalServer *m_server;
    QList<QLocalSocket*> m_clients;
    int m_nextId;
};

#endif // LOCALSERVERHOST_H
