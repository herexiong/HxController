#ifndef WIDGET_H
#define WIDGET_H

#include <QProcess>
#include <QQuickWidget>
#include <QRegularExpression>
#include <QSerialPort>
#include <QSystemTrayIcon>
#include <QVariantList>
#include <QVector>
#include <QStringList>
#include <QWidget>

#include "localserverhost.h"

class QCloseEvent;
class QHideEvent;
class QMenu;
class QAction;

class Widget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap performanceOverview READ performanceOverview NOTIFY performanceOverviewChanged)
    Q_PROPERTY(QVariantList componentCards READ componentCards NOTIFY componentCardsChanged)
    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY historiesChanged)
    Q_PROPERTY(QVariantList gpuHistory READ gpuHistory NOTIFY historiesChanged)
    Q_PROPERTY(QVariantList memoryHistory READ memoryHistory NOTIFY historiesChanged)
    Q_PROPERTY(QVariantList networkHistory READ networkHistory NOTIFY historiesChanged)
    Q_PROPERTY(bool debugMode READ debugMode CONSTANT)
    Q_PROPERTY(QStringList serialPorts READ serialPorts NOTIFY serialPortsChanged)
    Q_PROPERTY(QString selectedPort READ selectedPort WRITE setSelectedPort NOTIFY selectedPortChanged)
    Q_PROPERTY(bool serialConnected READ serialConnected NOTIFY serialConnectedChanged)
    Q_PROPERTY(QVariantList mediaItems READ mediaItems NOTIFY mediaItemsChanged)
    Q_PROPERTY(int activeTabId READ activeTabIdProperty NOTIFY activeTabIdChanged)
    Q_PROPERTY(QString nmStatusText READ nmStatusText NOTIFY nmStatusChanged)
    Q_PROPERTY(QString nmStatusTone READ nmStatusTone NOTIFY nmStatusChanged)
    Q_PROPERTY(int msgSentCount READ msgSentCountProperty NOTIFY nmStatusChanged)
    Q_PROPERTY(int msgRecvCount READ msgRecvCountProperty NOTIFY nmStatusChanged)
    Q_PROPERTY(QStringList nmLogs READ nmLogs NOTIFY nmLogsChanged)

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    QVariantMap performanceOverview() const;
    QVariantList componentCards() const;
    QVariantList cpuHistory() const;
    QVariantList gpuHistory() const;
    QVariantList memoryHistory() const;
    QVariantList networkHistory() const;
    bool debugMode() const;
    QStringList serialPorts() const;
    QString selectedPort() const;
    bool serialConnected() const;
    QVariantList mediaItems() const;
    int activeTabIdProperty() const;
    QString nmStatusText() const;
    QString nmStatusTone() const;
    int msgSentCountProperty() const;
    int msgRecvCountProperty() const;
    QStringList nmLogs() const;

    Q_INVOKABLE void openLogViewer();
    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void setSelectedPort(const QString &portDisplay);
    Q_INVOKABLE void toggleSerialConnection();
    Q_INVOKABLE void refreshMediaList();
    Q_INVOKABLE void playPause(int tabId = -1);
    Q_INVOKABLE void playNext(int tabId = -1);
    Q_INVOKABLE void playPrevious(int tabId = -1);
    Q_INVOKABLE void togglePip(int tabId = -1);
    Q_INVOKABLE void activateMedia(int tabId);

protected:
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void readProcessData();
    void resolvedata();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onNativeMessage(const QByteArray &rawJson);
    void onNmLog(const QString &log);
    void onServerStatusChanged();

signals:
    void recivedone();
    void performanceOverviewChanged();
    void componentCardsChanged();
    void historiesChanged();
    void serialPortsChanged();
    void selectedPortChanged();
    void serialConnectedChanged();
    void mediaItemsChanged();
    void activeTabIdChanged();
    void nmStatusChanged();
    void nmLogsChanged();

private:
    struct MonitorNode {
        QString title;
        QStringList infolist;
    };

    struct ParsedMetric {
        QString label;
        QString value;
    };

    struct MetricSnapshot {
        double cpuUsage = 0.0;
        double gpuUsage = 0.0;
        double memoryUsage = 0.0;
        double networkLevel = 0.0;
        QString cpuValue;
        QString gpuValue;
        QString memoryValue;
        QString networkValue;
    };

    void startCmdMonitorProcess(const QString &cmd);
    void updatePerformanceModel(const QVector<MonitorNode> &nodes);
    void updateMediaItems();
    void updateNmStatus();
    void appendNmLog(const QString &log);
    void sendIpcCommand(const QString &name, const QByteArray &payloadJson = "{}");
    int resolveTabId(int requestedTabId) const;
    QString selectedPortName() const;
    void setSelectedPortInternal(const QString &portDisplay, bool notify);
    ParsedMetric splitMetric(const QString &raw) const;
    double parsePercentValue(const QString &text) const;
    double parseCapacityUsagePercent(const QString &usedText, const QString &totalText) const;
    double parseNetworkLevel(const QString &text) const;
    QString normalizedTitle(const QString &rawTitle) const;
    QString componentKind(const QString &title, const QStringList &metrics) const;
    QVariantMap buildMetricItem(const QString &label, const QString &value, const QString &emphasis = QStringLiteral("secondary")) const;
    void appendHistorySample(QVariantList &history, double value, int limit = 32);
    QString extractFirstMatch(const QString &text, const QRegularExpression &regex, int group = 1) const;

    QQuickWidget *m_quickWidget;
    QProcess *m_process;
    QVector<QString> m_receivedData;
    QStringList m_serialPorts;
    QString m_selectedPort;
    bool m_serialConnected;
    QSerialPort *m_serialPort;

    LocalServerHost *m_ipcHost;
    QByteArray m_mediaTabsRaw;
    QVariantMap m_performanceOverview;
    QVariantList m_componentCards;
    QVariantList m_cpuHistory;
    QVariantList m_gpuHistory;
    QVariantList m_memoryHistory;
    QVariantList m_networkHistory;
    QVariantList m_mediaItems;
    QStringList m_nmLogs;
    QString m_nmStatusText;
    QString m_nmStatusTone;
    int m_activeTabId;
    int m_msgSentCount;
    int m_msgRecvCount;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *quitAction;
};

#endif // WIDGET_H


