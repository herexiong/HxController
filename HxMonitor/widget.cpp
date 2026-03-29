#include "widget.h"

#include <QAction>
#include <QApplication>
#include <QQmlEngine>
#include <algorithm>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QHideEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMenu>
#include <QMessageBox>
#include <QQmlContext>
#include <QRegularExpression>
#include <QSerialPortInfo>
#include <QTimer>
#include <QVBoxLayout>

#define FRAME_BEGIN QString::fromLatin1("BEGIN")
#define FRAME_END QString::fromLatin1("END")
#define TARGET_APP_POS (QCoreApplication::applicationDirPath() + QStringLiteral("/CmdMonitor/publish/CmdMonitor.exe"))

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , m_quickWidget(new QQuickWidget(this))
    , m_process(new QProcess(this))
    , m_serialConnected(false)
    , m_serialPort(new QSerialPort(this))
    , m_ipcHost(nullptr)
    , m_activeTabId(-1)
    , m_msgSentCount(0)
    , m_msgRecvCount(0)
    , trayIcon(nullptr)
    , trayIconMenu(nullptr)
    , quitAction(nullptr)
{
    setMinimumSize(1180, 780);
    resize(1280, 860);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowIcon(QIcon(QCoreApplication::applicationDirPath() + QStringLiteral("/icon.png")));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setClearColor(Qt::transparent);
    m_quickWidget->rootContext()->setContextProperty(QStringLiteral("backend"), this);
    m_quickWidget->engine()->addImportPath(QDir::cleanPath(QCoreApplication::applicationDirPath() + QStringLiteral("/../../third_party/FluentUI/dist")));
    m_quickWidget->engine()->addImportPath(QDir::cleanPath(QCoreApplication::applicationDirPath() + QStringLiteral("/../../../third_party/FluentUI/dist")));
    m_quickWidget->setSource(QUrl(QStringLiteral("qrc:/qml/MainView.qml")));
    layout->addWidget(m_quickWidget);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &Widget::readProcessData);
    connect(this, &Widget::recivedone, this, &Widget::resolvedata);

    m_ipcHost = new LocalServerHost(this);
    connect(m_ipcHost, &LocalServerHost::messageReceived, this, &Widget::onNativeMessage);
    connect(m_ipcHost, &LocalServerHost::logMessage, this, &Widget::onNmLog);
    connect(m_ipcHost, &LocalServerHost::serverStatusChanged, this, &Widget::onServerStatusChanged);

    QIcon icon(QCoreApplication::applicationDirPath() + QStringLiteral("/icon.png"));
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(QStringLiteral("HxMonitor"));
    trayIcon->show();

    quitAction = new QAction(QStringLiteral("Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Widget::iconActivated);

    startCmdMonitorProcess(QString());
    refreshPorts();
    updateNmStatus();

    QTimer::singleShot(500, this, [this]() {
        refreshMediaList();
    });
}

Widget::~Widget() = default;

QVariantMap Widget::performanceOverview() const
{
    return m_performanceOverview;
}

QVariantList Widget::componentCards() const
{
    return m_componentCards;
}

QVariantList Widget::cpuHistory() const
{
    return m_cpuHistory;
}

QVariantList Widget::gpuHistory() const
{
    return m_gpuHistory;
}

QVariantList Widget::memoryHistory() const
{
    return m_memoryHistory;
}

QVariantList Widget::networkHistory() const
{
    return m_networkHistory;
}

bool Widget::debugMode() const
{
    return QCoreApplication::arguments().contains(QStringLiteral("--debug"));
}

QStringList Widget::serialPorts() const
{
    return m_serialPorts;
}

QString Widget::selectedPort() const
{
    return m_selectedPort;
}

bool Widget::serialConnected() const
{
    return m_serialConnected;
}

QVariantList Widget::mediaItems() const
{
    return m_mediaItems;
}

int Widget::activeTabIdProperty() const
{
    return m_activeTabId;
}

QString Widget::nmStatusText() const
{
    return m_nmStatusText;
}

QString Widget::nmStatusTone() const
{
    return m_nmStatusTone;
}

int Widget::msgSentCountProperty() const
{
    return m_msgSentCount;
}

int Widget::msgRecvCountProperty() const
{
    return m_msgRecvCount;
}

QStringList Widget::nmLogs() const
{
    return m_nmLogs;
}

void Widget::openLogViewer()
{
    QProcess::startDetached(TARGET_APP_POS, QStringList() << QStringLiteral("LogMode"));
}

void Widget::refreshPorts()
{
    QStringList ports;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        ports.append(info.portName() + QStringLiteral(" -- ") + info.description());
    }
    std::sort(ports.begin(), ports.end());
    if (ports == m_serialPorts) {
        return;
    }

    m_serialPorts = ports;
    if (!m_serialPorts.contains(m_selectedPort)) {
        setSelectedPortInternal(m_serialPorts.isEmpty() ? QString() : m_serialPorts.first(), true);
    }
    emit serialPortsChanged();
}

void Widget::setSelectedPort(const QString &portDisplay)
{
    setSelectedPortInternal(portDisplay, true);
}

void Widget::toggleSerialConnection()
{
    if (!m_serialConnected) {
        const QString portName = selectedPortName();
        if (portName.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("HxMonitor"), QStringLiteral("Please select a serial port first."));
            return;
        }

        m_serialPort->setBaudRate(QSerialPort::Baud115200);
        m_serialPort->setPortName(portName);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);

        if (!m_serialPort->open(QSerialPort::ReadWrite)) {
            QMessageBox::critical(this, QStringLiteral("Serial Port Error"), QStringLiteral("Please verify the device is connected and the port is available."));
            return;
        }
        m_serialConnected = true;
    } else {
        m_serialPort->close();
        m_serialConnected = false;
    }

    emit serialConnectedChanged();
}

void Widget::refreshMediaList()
{
    sendIpcCommand(QStringLiteral("GetMediaList"));
}

void Widget::playPause(int tabId)
{
    const int resolvedTabId = resolveTabId(tabId);
    if (resolvedTabId > 0) {
        sendIpcCommand(QStringLiteral("PlayPause"), QByteArray("{\"tabId\":") + QByteArray::number(resolvedTabId) + "}");
    } else {
        sendIpcCommand(QStringLiteral("PlayPause"));
    }
}

void Widget::playNext(int tabId)
{
    const int resolvedTabId = resolveTabId(tabId);
    if (resolvedTabId > 0) {
        sendIpcCommand(QStringLiteral("Next"), QByteArray("{\"tabId\":") + QByteArray::number(resolvedTabId) + "}");
    } else {
        sendIpcCommand(QStringLiteral("Next"));
    }
}

void Widget::playPrevious(int tabId)
{
    const int resolvedTabId = resolveTabId(tabId);
    if (resolvedTabId > 0) {
        sendIpcCommand(QStringLiteral("Prev"), QByteArray("{\"tabId\":") + QByteArray::number(resolvedTabId) + "}");
    } else {
        sendIpcCommand(QStringLiteral("Prev"));
    }
}

void Widget::togglePip(int tabId)
{
    const int resolvedTabId = resolveTabId(tabId);
    if (resolvedTabId > 0) {
        sendIpcCommand(QStringLiteral("TogglePip"), QByteArray("{\"tabId\":") + QByteArray::number(resolvedTabId) + "}");
    } else {
        sendIpcCommand(QStringLiteral("TogglePip"));
    }
}

void Widget::activateMedia(int tabId)
{
    playPause(tabId);
}

void Widget::closeEvent(QCloseEvent *event)
{
    if (trayIcon && trayIcon->isVisible()) {
        hide();
        event->ignore();
        return;
    }
    QWidget::closeEvent(event);
}

void Widget::hideEvent(QHideEvent *event)
{
    if (trayIcon && trayIcon->isVisible()) {
        event->ignore();
        return;
    }
    QWidget::hideEvent(event);
}

void Widget::readProcessData()
{
    while (m_process->canReadLine()) {
        QString data = QString::fromLocal8Bit(m_process->readLine());
        data.replace(QStringLiteral("\r"), QString());
        data.replace(QStringLiteral("\n"), QString());
        data.replace(QStringLiteral("\""), QString());

        if (data == FRAME_BEGIN) {
            m_receivedData.clear();
        } else if (data == FRAME_END) {
            emit recivedone();
        } else {
            m_receivedData.append(data);
        }
    }
}

void Widget::resolvedata()
{
    QVector<MonitorNode> result;
    MonitorNode currentNode;

    for (const QString &line : std::as_const(m_receivedData)) {
        if (line.startsWith(QStringLiteral("->"))) {
            if (!currentNode.title.isEmpty() || !currentNode.infolist.isEmpty()) {
                result.append(currentNode);
                currentNode = MonitorNode();
            }
            currentNode.title = line.mid(2).trimmed();
        } else if (!line.trimmed().isEmpty()) {
            currentNode.infolist.append(line.trimmed());
        }
    }

    if (!currentNode.title.isEmpty() || !currentNode.infolist.isEmpty()) {
        result.append(currentNode);
    }

    updatePerformanceModel(result);

    QJsonObject json;
    for (const MonitorNode &node : result) {
        QJsonObject section;
        int type = -1;
        for (int i = 0; i < node.infolist.size(); ++i) {
            const QString metric = node.infolist.at(i);
            if (node.infolist.first().startsWith(QStringLiteral("CPU"))) {
                type = 0;
            } else if (node.infolist.first().startsWith(QStringLiteral("GPU"))) {
                type = 1;
            } else if (node.infolist.first().startsWith(QStringLiteral("\u5185\u5b58"))) {
                type = 2;
            } else if (node.infolist.first().startsWith(QStringLiteral("\u7f51\u7edc"))) {
                type = 3;
            }

            QString value;
            if (metric.contains(QLatin1Char(':'))) {
                value = metric.section(QLatin1Char(':'), 1).trimmed();
            } else {
                value = metric.section(QChar(0xff1a), 1).trimmed();
            }

            switch (type) {
            case 0:
                if (i == 0) section.insert(QStringLiteral("usage"), value);
                if (i == 1) section.insert(QStringLiteral("power"), value);
                if (i == 2) section.insert(QStringLiteral("temp"), value);
                break;
            case 1:
                if (i == 0) section.insert(QStringLiteral("usage"), value);
                if (i == 1) section.insert(QStringLiteral("power"), value);
                if (i == 2) section.insert(QStringLiteral("temp"), value);
                if (i == 3) {
                    section.insert(QStringLiteral("usedRAM"), value.section(QLatin1Char('/'), 0, 0).trimmed());
                    section.insert(QStringLiteral("totalRAM"), value.section(QLatin1Char('/'), 1, 1).trimmed());
                }
                break;
            case 2:
                if (i == 0) {
                    section.insert(QStringLiteral("usedRAM"), value.section(QLatin1Char('/'), 0, 0).trimmed());
                    section.insert(QStringLiteral("totalRAM"), value.section(QLatin1Char('/'), 1, 1).trimmed());
                }
                if (i == 1) section.insert(QStringLiteral("usage"), value);
                break;
            case 3:
                if (i == 0) section.insert(QStringLiteral("upload"), value);
                if (i == 1) section.insert(QStringLiteral("download"), value);
                break;
            default:
                break;
            }
        }

        switch (type) {
        case 0:
            section.insert(QStringLiteral("title"), node.title.section(QLatin1Char(':'), 1).trimmed());
            json.insert(QStringLiteral("CPU"), section);
            break;
        case 1:
            section.insert(QStringLiteral("title"), node.title.section(QLatin1Char(':'), 1).trimmed());
            json.insert(QStringLiteral("GPU"), section);
            break;
        case 2:
            section.insert(QStringLiteral("title"), node.title);
            json.insert(QStringLiteral("RAM"), section);
            break;
        case 3:
            section.insert(QStringLiteral("title"), node.title.section(QLatin1Char(':'), 1).trimmed());
            json.insert(QStringLiteral("NET"), section);
            break;
        default:
            break;
        }
    }

    if (m_serialConnected && m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }

    refreshPorts();
}

void Widget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        showNormal();
        raise();
        activateWindow();
    }
}

void Widget::onNativeMessage(const QByteArray &rawJson)
{
    ++m_msgRecvCount;
    updateNmStatus();

    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(rawJson, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    const QJsonObject msg = doc.object();
    const QString type = msg.value(QStringLiteral("type")).toString();
    const QString name = msg.value(QStringLiteral("name")).toString();

    if ((type == QStringLiteral("evt") && name == QStringLiteral("MediaList"))
        || (type == QStringLiteral("res") && name == QStringLiteral("GetMediaList"))) {
        const QJsonObject payload = msg.value(QStringLiteral("payload")).toObject();
        const int newActiveId = payload.value(QStringLiteral("activeTabId")).isNull()
            ? -1
            : payload.value(QStringLiteral("activeTabId")).toInt();

        if (newActiveId != m_activeTabId) {
            m_activeTabId = newActiveId;
            emit activeTabIdChanged();
        }

        m_mediaTabsRaw = QJsonDocument(payload.value(QStringLiteral("tabs")).toObject()).toJson(QJsonDocument::Compact);
        updateMediaItems();
    }
}

void Widget::onNmLog(const QString &log)
{
    appendNmLog(log);
}

void Widget::onServerStatusChanged()
{
    updateNmStatus();
}

void Widget::startCmdMonitorProcess(const QString &cmd)
{
    if (m_process->state() == QProcess::NotRunning) {
        m_process->start(TARGET_APP_POS, QStringList() << cmd);
    }
    if (!m_process->waitForStarted()) {
        qDebug() << "Error:" << m_process->errorString();
    }
}

void Widget::updatePerformanceModel(const QVector<MonitorNode> &nodes)
{
    QVariantList componentCards;
    QVariantList summaryMetrics;
    QVariantMap overview;
    MetricSnapshot snapshot;

    static const QHash<QString, QString> accentByKind = {
        {QStringLiteral("cpu"), QStringLiteral("#4f8cff")},
        {QStringLiteral("gpu"), QStringLiteral("#13b8a6")},
        {QStringLiteral("memory"), QStringLiteral("#ff8c42")},
        {QStringLiteral("network"), QStringLiteral("#9b7bff")}
    };

    for (const MonitorNode &node : nodes) {
        QVariantList metrics;
        const QString title = normalizedTitle(node.title);
        const QString kind = componentKind(title, node.infolist);

        const QString joined = node.infolist.join(QStringLiteral(" "));
        QString primaryValue;
        QString secondaryValue;
        QString statusText;

        if (kind == QStringLiteral("cpu")) {
            const QString usage = extractFirstMatch(joined, QRegularExpression(QStringLiteral("([0-9]+(?:\\.[0-9]+)?\\s*%)")));
            const QString power = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(?:功率|power)\\s*[:：]?\\s*([0-9]+(?:\\.[0-9]+)?\\s*[Ww])"), QRegularExpression::CaseInsensitiveOption));
            const QString temp = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(?:温度|temp(?:erature)?)\\s*[:：]?\\s*([0-9]+(?:\\.[0-9]+)?\\s*(?:°?C|℃))"), QRegularExpression::CaseInsensitiveOption));

            snapshot.cpuUsage = parsePercentValue(usage);
            snapshot.cpuValue = usage;
            primaryValue = usage;
            secondaryValue = power;
            statusText = temp;

            if (!usage.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Usage"), usage, QStringLiteral("primary")));
            if (!power.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Power"), power, QStringLiteral("secondary")));
            if (!temp.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Temp"), temp, QStringLiteral("accent")));
        } else if (kind == QStringLiteral("gpu")) {
            const QString usage = extractFirstMatch(joined, QRegularExpression(QStringLiteral("([0-9]+(?:\\.[0-9]+)?\\s*%)")));
            const QString power = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(?:功率|power)\\s*[:：]?\\s*([0-9]+(?:\\.[0-9]+)?\\s*[Ww])"), QRegularExpression::CaseInsensitiveOption));
            const QString temp = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(?:温度|temp(?:erature)?)\\s*[:：]?\\s*([0-9]+(?:\\.[0-9]+)?\\s*(?:°?C|℃))"), QRegularExpression::CaseInsensitiveOption));
            const QString vram = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(\\d+(?:\\.\\d+)?\\s*[GMK]B?\\s*/\\s*\\d+(?:\\.\\d+)?\\s*[GMK]B?)"), QRegularExpression::CaseInsensitiveOption));

            snapshot.gpuUsage = parsePercentValue(usage);
            snapshot.gpuValue = usage;
            primaryValue = usage;
            secondaryValue = power;
            statusText = temp;

            if (!usage.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Usage"), usage, QStringLiteral("primary")));
            if (!power.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Power"), power, QStringLiteral("secondary")));
            if (!temp.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Temp"), temp, QStringLiteral("accent")));
            if (!vram.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("VRAM"), vram, QStringLiteral("secondary")));
            if (!vram.isEmpty() && secondaryValue.isEmpty()) {
                secondaryValue = vram;
            }
        } else if (kind == QStringLiteral("memory")) {
            const QString capacity = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(\\d+(?:\\.\\d+)?\\s*[GMK]B?\\s*/\\s*\\d+(?:\\.\\d+)?\\s*[GMK]B?)"), QRegularExpression::CaseInsensitiveOption));
            const QString usage = extractFirstMatch(joined, QRegularExpression(QStringLiteral("([0-9]+(?:\\.[0-9]+)?\\s*%)")));

            if (!capacity.isEmpty()) {
                snapshot.memoryUsage = parseCapacityUsagePercent(capacity.section(QLatin1Char('/'), 0, 0).trimmed(),
                                                                 capacity.section(QLatin1Char('/'), 1, 1).trimmed());
                primaryValue = capacity;
                metrics.append(buildMetricItem(QStringLiteral("Capacity"), capacity, QStringLiteral("primary")));
            }
            if (!usage.isEmpty()) {
                snapshot.memoryUsage = parsePercentValue(usage);
                snapshot.memoryValue = usage;
                statusText = usage;
                metrics.append(buildMetricItem(QStringLiteral("Usage"), usage, QStringLiteral("accent")));
            } else if (snapshot.memoryUsage > 0.0) {
                snapshot.memoryValue = QStringLiteral("%1%").arg(QString::number(snapshot.memoryUsage, 'f', 0));
                statusText = snapshot.memoryValue;
            }
        } else if (kind == QStringLiteral("network")) {
            const QString upload = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(?:上传|up(?:load)?)\\s*[:：]?\\s*([^\\s]+)"), QRegularExpression::CaseInsensitiveOption));
            const QString download = extractFirstMatch(joined, QRegularExpression(QStringLiteral("(?:下载|down(?:load)?)\\s*[:：]?\\s*([^\\s]+)"), QRegularExpression::CaseInsensitiveOption));

            primaryValue = upload;
            secondaryValue = download;
            snapshot.networkLevel = std::max(parseNetworkLevel(upload), parseNetworkLevel(download));
            snapshot.networkValue = download.isEmpty() ? upload : download;

            if (!upload.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Upload"), upload, QStringLiteral("primary")));
            if (!download.isEmpty()) metrics.append(buildMetricItem(QStringLiteral("Download"), download, QStringLiteral("accent")));
        }

        if (metrics.isEmpty()) {
            for (const QString &raw : node.infolist) {
                const ParsedMetric parsed = splitMetric(raw);
                metrics.append(buildMetricItem(parsed.label, parsed.value, QStringLiteral("secondary")));
            }
        }

        QVariantMap card;
        card.insert(QStringLiteral("title"), title);
        card.insert(QStringLiteral("kind"), kind);
        card.insert(QStringLiteral("subtitle"),
                    kind == QStringLiteral("cpu") ? QStringLiteral("Compute and thermal load") :
                    kind == QStringLiteral("gpu") ? QStringLiteral("Graphics processor state") :
                    kind == QStringLiteral("memory") ? QStringLiteral("Capacity pressure and allocation") :
                    kind == QStringLiteral("network") ? QStringLiteral("Current transfer throughput") :
                    QStringLiteral("Live telemetry"));
        card.insert(QStringLiteral("accent"), accentByKind.value(kind, QStringLiteral("#4f8cff")));
        card.insert(QStringLiteral("primaryValue"), primaryValue);
        card.insert(QStringLiteral("secondaryValue"), secondaryValue);
        card.insert(QStringLiteral("statusText"), statusText);
        card.insert(QStringLiteral("metrics"), metrics);
        componentCards.append(card);
    }

    appendHistorySample(m_cpuHistory, snapshot.cpuUsage);
    appendHistorySample(m_gpuHistory, snapshot.gpuUsage);
    appendHistorySample(m_memoryHistory, snapshot.memoryUsage);
    appendHistorySample(m_networkHistory, snapshot.networkLevel);

    summaryMetrics.append(buildMetricItem(QStringLiteral("CPU"), snapshot.cpuValue.isEmpty() ? QStringLiteral("--") : snapshot.cpuValue, QStringLiteral("primary")));
    summaryMetrics.append(buildMetricItem(QStringLiteral("GPU"), snapshot.gpuValue.isEmpty() ? QStringLiteral("--") : snapshot.gpuValue, QStringLiteral("accent")));
    summaryMetrics.append(buildMetricItem(QStringLiteral("Memory"), snapshot.memoryValue.isEmpty() ? QStringLiteral("--") : snapshot.memoryValue, QStringLiteral("accent")));
    summaryMetrics.append(buildMetricItem(QStringLiteral("Network"), QStringLiteral("%1%").arg(QString::number(snapshot.networkLevel, 'f', 0)), QStringLiteral("secondary")));

    overview.insert(QStringLiteral("eyebrow"), QStringLiteral("Performance cockpit"));
    overview.insert(QStringLiteral("title"), QStringLiteral("System performance"));
    overview.insert(QStringLiteral("description"), QStringLiteral("Group critical telemetry by subsystem, highlight the hottest signals, and keep recent trends visible."));
    overview.insert(QStringLiteral("summaryMetrics"), summaryMetrics);
    overview.insert(QStringLiteral("cpuHeadline"), snapshot.cpuValue.isEmpty() ? QStringLiteral("--") : snapshot.cpuValue);
    overview.insert(QStringLiteral("gpuHeadline"), snapshot.gpuValue.isEmpty() ? QStringLiteral("--") : snapshot.gpuValue);
    overview.insert(QStringLiteral("memoryHeadline"), snapshot.memoryValue.isEmpty() ? QStringLiteral("--") : snapshot.memoryValue);
    overview.insert(QStringLiteral("networkHeadline"), QStringLiteral("%1%").arg(QString::number(snapshot.networkLevel, 'f', 0)));

    m_componentCards = componentCards;
    m_performanceOverview = overview;
    emit componentCardsChanged();
    emit performanceOverviewChanged();
    emit historiesChanged();
}

void Widget::updateMediaItems()
{
    QVariantList items;
    const QJsonDocument tabsDoc = QJsonDocument::fromJson(m_mediaTabsRaw);
    const QJsonObject tabs = tabsDoc.object();

    for (const QString &key : tabs.keys()) {
        const int tabId = key.toInt();
        const QJsonObject state = tabs.value(key).toObject();
        const bool playing = state.value(QStringLiteral("playing")).toBool(false);
        const qint64 durationMs = static_cast<qint64>(state.value(QStringLiteral("durationMs")).toDouble(0));
        const qint64 positionMs = static_cast<qint64>(state.value(QStringLiteral("positionMs")).toDouble(0));

        const QString posStr = QStringLiteral("%1:%2")
                                   .arg(positionMs / 60000, 2, 10, QChar('0'))
                                   .arg((positionMs / 1000) % 60, 2, 10, QChar('0'));
        const QString durStr = QStringLiteral("%1:%2")
                                   .arg(durationMs / 60000, 2, 10, QChar('0'))
                                   .arg((durationMs / 1000) % 60, 2, 10, QChar('0'));

        QVariantMap item;
        item.insert(QStringLiteral("tabId"), tabId);
        item.insert(QStringLiteral("title"), state.value(QStringLiteral("title")).toString(QStringLiteral("Unknown media")));
        item.insert(QStringLiteral("artist"), state.value(QStringLiteral("artist")).toString(QStringLiteral("Unknown source")));
        item.insert(QStringLiteral("timeline"), posStr + QStringLiteral(" / ") + durStr);
        item.insert(QStringLiteral("playing"), playing);
        item.insert(QStringLiteral("active"), tabId == m_activeTabId);
        item.insert(QStringLiteral("stateLabel"), playing ? QStringLiteral("Playing") : QStringLiteral("Paused"));
        items.append(item);
    }

    std::sort(items.begin(), items.end(), [](const QVariant &left, const QVariant &right) {
        return left.toMap().value(QStringLiteral("tabId")).toInt() < right.toMap().value(QStringLiteral("tabId")).toInt();
    });

    m_mediaItems = items;
    emit mediaItemsChanged();
}

void Widget::updateNmStatus()
{
    if (!m_ipcHost) {
        m_nmStatusText = QStringLiteral("IPC service not initialized");
        m_nmStatusTone = QStringLiteral("danger");
    } else if (m_ipcHost->clientCount() > 0) {
        m_nmStatusText = QStringLiteral("%1 client(s) connected").arg(m_ipcHost->clientCount());
        m_nmStatusTone = QStringLiteral("success");
    } else if (m_ipcHost->isListening()) {
        m_nmStatusText = QStringLiteral("IPC service listening for browser proxy");
        m_nmStatusTone = QStringLiteral("warning");
    } else {
        m_nmStatusText = QStringLiteral("IPC service failed to start");
        m_nmStatusTone = QStringLiteral("danger");
    }

    emit nmStatusChanged();
}

void Widget::appendNmLog(const QString &log)
{
    m_nmLogs.append(log);
    while (m_nmLogs.size() > 200) {
        m_nmLogs.removeFirst();
    }
    emit nmLogsChanged();
}

void Widget::sendIpcCommand(const QString &name, const QByteArray &payloadJson)
{
    if (!m_ipcHost) {
        return;
    }
    ++m_msgSentCount;
    m_ipcHost->sendCommand(name, payloadJson);
    updateNmStatus();
}

int Widget::resolveTabId(int requestedTabId) const
{
    if (requestedTabId > 0) {
        return requestedTabId;
    }
    return m_activeTabId;
}

QString Widget::selectedPortName() const
{
    return m_selectedPort.section(QStringLiteral(" -- "), 0, 0).trimmed();
}

void Widget::setSelectedPortInternal(const QString &portDisplay, bool notify)
{
    if (m_selectedPort == portDisplay) {
        return;
    }
    m_selectedPort = portDisplay;
    if (notify) {
        emit selectedPortChanged();
    }
}

Widget::ParsedMetric Widget::splitMetric(const QString &raw) const
{
    ParsedMetric metric;
    if (raw.contains(QChar(0xff1a))) {
        metric.label = raw.section(QChar(0xff1a), 0, 0).trimmed();
        metric.value = raw.section(QChar(0xff1a), 1).trimmed();
    } else if (raw.contains(QLatin1Char(':'))) {
        metric.label = raw.section(QLatin1Char(':'), 0, 0).trimmed();
        metric.value = raw.section(QLatin1Char(':'), 1).trimmed();
    } else {
        metric.label = QStringLiteral("Metric");
        metric.value = raw.trimmed();
    }
    return metric;
}

double Widget::parsePercentValue(const QString &text) const
{
    QString normalized = text.trimmed();
    normalized.remove(QLatin1Char('%'));
    normalized.remove(QLatin1Char(' '));
    bool ok = false;
    const double value = normalized.toDouble(&ok);
    return ok ? std::clamp(value, 0.0, 100.0) : 0.0;
}

double Widget::parseCapacityUsagePercent(const QString &usedText, const QString &totalText) const
{
    auto extractNumber = [](QString text) {
        text.remove(QRegularExpression(QStringLiteral("[^0-9\\.]")));
        bool ok = false;
        const double value = text.toDouble(&ok);
        return ok ? value : 0.0;
    };

    const double used = extractNumber(usedText);
    const double total = extractNumber(totalText);
    if (total <= 0.0) {
        return 0.0;
    }
    return std::clamp((used / total) * 100.0, 0.0, 100.0);
}

double Widget::parseNetworkLevel(const QString &text) const
{
    QString normalized = text.trimmed().toUpper();
    double multiplier = 1.0;
    if (normalized.contains(QStringLiteral("GB"))) {
        multiplier = 100.0;
    } else if (normalized.contains(QStringLiteral("MB"))) {
        multiplier = 12.0;
    } else if (normalized.contains(QStringLiteral("KB"))) {
        multiplier = 1.0;
    }

    normalized.remove(QRegularExpression(QStringLiteral("[^0-9\\.]")));
    bool ok = false;
    const double raw = normalized.toDouble(&ok);
    if (!ok) {
        return 0.0;
    }
    return std::clamp(raw * multiplier, 0.0, 100.0);
}

QString Widget::normalizedTitle(const QString &rawTitle) const
{
    QString title = rawTitle;
    if (title.startsWith(QStringLiteral("->"))) {
        title = title.mid(2).trimmed();
    }
    return title.trimmed();
}

QString Widget::componentKind(const QString &title, const QStringList &metrics) const
{
    const QString loweredTitle = title.toLower();
    if (loweredTitle.contains(QStringLiteral("cpu"))) {
        return QStringLiteral("cpu");
    }
    if (loweredTitle.contains(QStringLiteral("gpu"))) {
        return QStringLiteral("gpu");
    }
    if (loweredTitle.contains(QStringLiteral("memory")) || title.contains(QStringLiteral("\u5185\u5b58"))) {
        return QStringLiteral("memory");
    }
    if (loweredTitle.contains(QStringLiteral("network")) || title.contains(QStringLiteral("\u7f51\u7edc"))) {
        return QStringLiteral("network");
    }

    for (const QString &metric : metrics) {
        const QString loweredMetric = metric.toLower();
        if (loweredMetric.contains(QStringLiteral("cpu"))) {
            return QStringLiteral("cpu");
        }
        if (loweredMetric.contains(QStringLiteral("gpu"))) {
            return QStringLiteral("gpu");
        }
        if (loweredMetric.contains(QStringLiteral("upload")) || loweredMetric.contains(QStringLiteral("download"))) {
            return QStringLiteral("network");
        }
        if (metric.contains(QStringLiteral("\u5185\u5b58"))) {
            return QStringLiteral("memory");
        }
        if (metric.contains(QStringLiteral("\u7f51\u7edc"))) {
            return QStringLiteral("network");
        }
    }
    return QStringLiteral("generic");
}

QVariantMap Widget::buildMetricItem(const QString &label, const QString &value, const QString &emphasis) const
{
    QVariantMap metric;
    metric.insert(QStringLiteral("label"), label);
    metric.insert(QStringLiteral("value"), value);
    metric.insert(QStringLiteral("emphasis"), emphasis);
    return metric;
}

void Widget::appendHistorySample(QVariantList &history, double value, int limit)
{
    history.append(std::clamp(value, 0.0, 100.0));
    while (history.size() > limit) {
        history.removeFirst();
    }
}

QString Widget::extractFirstMatch(const QString &text, const QRegularExpression &regex, int group) const
{
    const QRegularExpressionMatch match = regex.match(text);
    if (!match.hasMatch()) {
        return QString();
    }
    return match.captured(group).trimmed();
}
