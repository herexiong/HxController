#include "widget.h"
#include "ui_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QTimer>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#define FRAME_BEGIN QString::fromLocal8Bit("BEGIN")
#define FRAME_END QString::fromLocal8Bit("END")
#define TARGET_APP_POS QCoreApplication::applicationDirPath()+QString("/CmdMonitor/publish/CmdMonitor.exe")

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , recivedata(new QVector<QString>)
    ,widgetobj(new QVector<MonitorLable *>)
    ,serialPort(new QSerialPort(parent))
    ,ipcHost(nullptr)
    ,activeTabId(-1)
{
    ui->setupUi(this);

    this->logbtn = new QPushButton("CatLog",this);
    this->logbtn->resize(80,25);
    this->logbtn->show();
    connect(logbtn,&QPushButton::clicked,this,[](){
        QProcess::startDetached(TARGET_APP_POS, QStringList() << "LogMode");
    });

    this->combox = new QComboBox(this);
    this->combox->resize(180,25);
    this->combox->move(100,0);
    this->combox->addItem("Com1");
    this->combox->show();

    this->comlabel = new QLabel("串口号",this);
    this->comlabel->resize(80,25);
    this->comlabel->move(285,0);
    this->comlabel->show();

    this->connectbtn = new QPushButton("连接",this);
    this->connectbtn->resize(80,25);
    this->connectbtn->show();
    this->connectbtn->move(340,0);
    connect(connectbtn,&QPushButton::clicked,[&](){
        QString port = QString(this->combox->currentText()).split("-")[0];//获取串口号
        USART(port);
    });

    //执行命令获取
    process = new QProcess(this);
    this->startCmdMonitorProcess("");

    //有可读数据读取
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessData()));
    connect(this,SIGNAL(recivedone()),this,SLOT(resolvedata()));

    //托盘初始化
    QIcon icon = QIcon(QCoreApplication::applicationDirPath() +"/icon.png");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("a trayicon example");
    trayIcon->show(); //必须调用，否则托盘图标不显示

    //创建菜单项动作(以下动作只对windows有效)
    quitAction = new QAction("退出~", this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit())); //关闭应用，qApp对应的是程序全局唯一指针

    //创建托盘菜单(必须先创建动作，后添加菜单项，还可以加入菜单项图标美化)
    trayIconMenu = new QMenu(this);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    //========== 媒体控制区域 ==========
    // 媒体列表
    mediaListWidget = new QListWidget(this);
    mediaListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mediaListWidget->setMinimumHeight(120);
    connect(mediaListWidget, &QListWidget::itemDoubleClicked,
            this, &Widget::onMediaItemDoubleClicked);

    // 控制按钮
    playPauseBtn = new QPushButton("⏯ 播放/暂停", this);
    nextBtn = new QPushButton("⏩ 下一首", this);
    prevBtn = new QPushButton("⏪ 上一首", this);
    pipBtn = new QPushButton("📺 画中画", this);

    connect(playPauseBtn, &QPushButton::clicked, this, &Widget::onPlayPauseClicked);
    connect(nextBtn, &QPushButton::clicked, this, &Widget::onNextClicked);
    connect(prevBtn, &QPushButton::clicked, this, &Widget::onPrevClicked);
    connect(pipBtn, &QPushButton::clicked, this, &Widget::onPipClicked);

    QHBoxLayout *mediaBtnLayout = new QHBoxLayout();
    mediaBtnLayout->addWidget(prevBtn);
    mediaBtnLayout->addWidget(playPauseBtn);
    mediaBtnLayout->addWidget(nextBtn);
    mediaBtnLayout->addWidget(pipBtn);

    QLabel *mediaLabel = new QLabel("\xf0\x9f\x8e\xac \xe6\xb5\x8f\xe8\xa7\x88\xe5\x99\xa8\xe5\xaa\x92\xe4\xbd\x93\xe6\x8e\xa7\xe5\x88\xb6", this);
    mediaLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");

    //========== 诊断面板 ==========
    msgSentCount = 0;
    msgRecvCount = 0;

    nmStatusLabel = new QLabel(this);
    nmStatusLabel->setTextFormat(Qt::RichText);
    nmStatusLabel->setStyleSheet("font-size: 12px; padding: 4px;");

    refreshBtn = new QPushButton("\xf0\x9f\x94\x84 \xe5\x88\xb7\xe6\x96\xb0", this);
    refreshBtn->setFixedWidth(80);
    connect(refreshBtn, &QPushButton::clicked, this, &Widget::onRefreshClicked);

    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(nmStatusLabel, 1);
    statusLayout->addWidget(refreshBtn, 0);

    nmLogView = new QTextEdit(this);
    nmLogView->setReadOnly(true);
    nmLogView->setFixedHeight(150);
    nmLogView->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #d4d4d4; "
        "font-family: Consolas, monospace; font-size: 11px; "
        "border: 1px solid #555; }"
    );

    QLabel *diagLabel = new QLabel("\xf0\x9f\x94\xa7 IPC Server \xe8\xaf\x8a\xe6\x96\xad", this);
    diagLabel->setStyleSheet("font-weight: bold; font-size: 13px; margin-top: 8px; color: #888;");

    QVBoxLayout *mediaSection = new QVBoxLayout();
    mediaSection->addWidget(mediaLabel);
    mediaSection->addWidget(mediaListWidget);
    mediaSection->addLayout(mediaBtnLayout);
    mediaSection->addWidget(diagLabel);
    mediaSection->addLayout(statusLayout);
    mediaSection->addWidget(nmLogView);

    // 将媒体控制区域添加到主窗口底部
    if (!this->layout()) {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(5, 50, 5, 5);
        mainLayout->addLayout(mediaSection);
    }

    //========== IPC Server 初始化 ==========
    ipcHost = new LocalServerHost(this);
    connect(ipcHost, &LocalServerHost::messageReceived,
            this, &Widget::onNativeMessage);
    connect(ipcHost, &LocalServerHost::logMessage,
            this, &Widget::onNmLog);
    connect(ipcHost, &LocalServerHost::serverStatusChanged,
            this, &Widget::updateNmStatus);

    updateNmStatus();

    // 启动时请求一次媒体列表
    QTimer::singleShot(500, this, [this](){
        ipcHost->sendCommand("GetMediaList");
    });
}

void Widget::startCmdMonitorProcess(const QString& cmd)
{
    if (process->state() == QProcess::NotRunning) {
        // qDebug()<<"strat process";
        process->start(TARGET_APP_POS, QStringList()<<cmd);
    }
    if (!process->waitForStarted()) {
        qDebug() << "Error:" << process->errorString();
    }
}

void Widget::readProcessData(void)
{
    // 读取子进程标准输出
    while (process->canReadLine()) {
        QString data = QString::fromLocal8Bit(process->readLine());

        data.replace("\r", "");
        data.replace("\n", "");
        data.replace("\"", "");

        if(data.compare(FRAME_BEGIN) == 0){
            this->recivedata->clear();//清空容器准备接收
        }else if(data.compare(FRAME_END) == 0){
            emit recivedone();//触发信号，解析内容
        }else{
            this->recivedata->append(data);
        }
    }
}

void Widget::resolvedata(void)
{
    QVector<MonitorLableNode> result;
    MonitorLableNode currentNode;

    for (const QString& line : *(this->recivedata)) {
        if (line.startsWith("->")) {
            // 如果当前节点有数据，将其加入结果
            if (!currentNode.title.isEmpty() || !currentNode.infolist.isEmpty()) {
                result.append(currentNode);
                currentNode = MonitorLableNode(); // 重置当前节点
            }
            // 设置新节点的标题
            currentNode.title = line.mid(2).trimmed();
        } else {
            // 按空格分割并添加到当前节点的 infolist
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            currentNode.infolist.append(parts);
        }
    }
    // 添加最后一个节点（如果有）
    if (!currentNode.title.isEmpty() || !currentNode.infolist.isEmpty()) {
        result.append(currentNode);
    }

    if(!widgetisinit){
        // 获取或创建垂直布局
        QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
        if (!layout) {
            layout = new QVBoxLayout(this);
            layout->setContentsMargins(0, 50, 0, 0);
        }
        layout->setSpacing(10);

        // 在媒体控制区域之前插入监控组件
        for(int i = 0;i<result.size();i++)
        {
            widgetobj->append(new MonitorLable(result[i],this));
            layout->insertWidget(i, (*widgetobj)[i]); // 在前面插入
        }
        widgetisinit = true;
    }else{
        for(int i = 0;i<result.size() && i<widgetobj->size();i++)
        {
            (*widgetobj)[i] ->RefreshMonitorLable(result[i]);
        }
        RefreshPort();
    }

    // 构建 JSON 对象
    QJsonObject json;
    for(MonitorLableNode node: result)
    {
        QJsonObject temp_json = QJsonObject();
        int type = -1;//0->CPU, 1->GPU, 2->RAM, 3->Net
        for(int i = 0;i<node.infolist.size();i++)
        {
            if(node.infolist[0].startsWith("CPU")) type = 0;
            else if(node.infolist[0].startsWith("GPU")) type = 1;
            else if(node.infolist[0].startsWith("内存")) type = 2;
            else if(node.infolist[0].startsWith("网络")) type = 3;
            if(type != -1)
            {
                switch (type) {
                case 0://CPU
                    switch (i) {
                    case 0:
                        temp_json.insert("usage",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    case 1:
                        temp_json.insert("power",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    case 2:
                        temp_json.insert("temp",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    }
                    break;
                case 1://GPU
                    switch (i) {
                    case 0:
                        temp_json.insert("usage",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    case 1:
                        temp_json.insert("power",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    case 2:
                        temp_json.insert("temp",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    case 3:
                        temp_json.insert("usedRAM",node.infolist[i].split(':',Qt::SkipEmptyParts)[1].split('/')[0]);
                        temp_json.insert("totalRAM",node.infolist[i].split(':',Qt::SkipEmptyParts)[1].split('/')[1]);
                        break;
                    }
                    break;
                case 2://RAM
                    switch (i) {
                    case 0:
                        // temp_json.insert("us",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        temp_json.insert("usedRAM",node.infolist[i].split(':',Qt::SkipEmptyParts)[1].split('/')[0]);
                        temp_json.insert("totalRAM",node.infolist[i].split(':',Qt::SkipEmptyParts)[1].split('/')[1]);
                        break;
                    case 1:
                        temp_json.insert("usage",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    }
                    break;
                case 3://NetWork
                    switch (i) {
                    case 0:
                        temp_json.insert("upload",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    case 1:
                        temp_json.insert("download",node.infolist[i].split(':',Qt::SkipEmptyParts)[1]);
                        break;
                    }
                    break;
                }
            }
        }
        switch (type) {
        case 0:
            temp_json.insert("title",node.title.split(':',Qt::SkipEmptyParts)[1]);
            json.insert("CPU", QJsonValue(temp_json));
            break;
        case 1:
            temp_json.insert("title",node.title.split(':',Qt::SkipEmptyParts)[1]);
            json.insert("GPU", QJsonValue(temp_json));
            break;
        case 2:
            temp_json.insert("title",node.title);
            json.insert("RAM", QJsonValue(temp_json));
            break;
        case 3:
            temp_json.insert("title",node.title.split(':',Qt::SkipEmptyParts)[1]);
            json.insert("NET", QJsonValue(temp_json));
            break;
        }
    }
    // qDebug()<<json;//验证json数据
    // //串口发送数据
    if(serialPort) serialPort->write(QJsonDocument(json).toJson().data());
    // qDebug()<<QJsonDocument(json).toJson().data();//验证json数据
}

//刷新可用串口
void Widget::RefreshPort(void) {
    QVector<QString>temp;
    //获取当前可用串口号
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
        temp.push_back(info.portName()+"--"+info.description());
    }
    //排序现有的串口号,用于比较和原有的差距
    std::sort(temp.begin(), temp.end(), [](const auto &a, const auto &b) {
        return a < b; // 升序
    });
    if (temp != this->ports) {  //如果可用串口号有变化
        this->combox->clear();  //清除原有列表
        this->ports = temp;         //更新串口列表
        for (auto& a : ports) {     //更新新串口
            this->combox->addItem(a);
        }
    }
}

//串口通信核心
void Widget::USART(QString port) {
    static bool connect_status = false;
    QSerialPort::BaudRate Baud = QSerialPort::Baud115200;//波特率
    QSerialPort::DataBits Data = QSerialPort::Data8;     //数据位
    QSerialPort::StopBits Stop = QSerialPort::OneStop;     //停止位
    QSerialPort::Parity Check = QSerialPort::NoParity;      //校验位

    // serialPort = new QSerialPort(this);
    //为串口设置配置
    serialPort->setBaudRate(Baud);
    serialPort->setPortName(port);
    serialPort->setDataBits(Data);
    serialPort->setParity(Check);
    serialPort->setStopBits(Stop);
    if(!connect_status){
        //打开串口
        if (serialPort->open(QSerialPort::ReadWrite)) {
            qDebug()<<"串口打开";
            connectbtn->setText("关闭连接");
            connect_status = true;
        }else {
            QMessageBox::critical(this, "串口打开失败","请确认串口是否正确连接");
        }
    }else{
        serialPort->close();
        qDebug()<<"串口关闭";
        connectbtn->setText("连接");
        connect_status = false;
    }
}

void Widget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        // trayIcon->showMessage("title","你单击了"); //后面两个默认参数
        this->showNormal();
        break;
    case QSystemTrayIcon::DoubleClick:
        // trayIcon->showMessage("title","你双击了");
        break;
    case QSystemTrayIcon::MiddleClick:
        // trayIcon->showMessage("title","你中键了");
        break;
    default:
        break;
    }

}

void Widget::closeEvent(QCloseEvent *event)
{
    if(trayIcon->isVisible())
    {
        hide(); //隐藏窗口
        event->ignore(); //忽略事件
    }
}

void Widget::hideEvent(QHideEvent *event)
{
    if(trayIcon->isVisible())
    {
        hide(); //隐藏窗口
        // trayIcon->showMessage("title","隐藏到托盘图标了"); //提示用户隐藏到了托盘
        event->ignore(); //忽略事件
    }
}


Widget::~Widget()
{
    delete this->ui;
    delete this->recivedata;
    delete this->widgetobj;
}

//========== IPC Server 处理 ==========

void Widget::onNativeMessage(const QByteArray &rawJson)
{
    qDebug() << "[Widget] onNativeMessage raw:" << rawJson;

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(rawJson, &parseErr);
    if (parseErr.error != QJsonParseError::NoError) {
        qWarning() << "[Widget] JSON parse error:" << parseErr.errorString();
        return;
    }
    QJsonObject msg = doc.object();

    QString type = msg["type"].toString();
    QString name = msg["name"].toString();
    qDebug() << "[Widget] type=" << type << "name=" << name;

    if ((type == "evt" && name == "MediaList") ||
        (type == "res" && name == "GetMediaList")) {
        QJsonObject payload = msg["payload"].toObject();
        qDebug() << "[Widget] payload keys:" << payload.keys();

        if (payload["activeTabId"].isNull()) {
            activeTabId = -1;
        } else {
            activeTabId = payload["activeTabId"].toInt();
        }
        qDebug() << "[Widget] activeTabId=" << activeTabId;

        QJsonObject tabsObj = payload["tabs"].toObject();
        qDebug() << "[Widget] tabs keys:" << tabsObj.keys() << "count=" << tabsObj.size();
        mediaTabsRaw = QJsonDocument(tabsObj).toJson(QJsonDocument::Compact);

        updateMediaListUI();
    } else {
        qDebug() << "[Widget] unhandled message type/name, ignoring";
    }
}

void Widget::updateMediaListUI()
{
    mediaListWidget->clear();

    QJsonDocument tabsDoc = QJsonDocument::fromJson(mediaTabsRaw);
    QJsonObject mediaTabs = tabsDoc.object();
    qDebug() << "[Widget] updateMediaListUI: mediaTabsRaw=" << mediaTabsRaw
             << "mediaTabs.size()=" << mediaTabs.size();

    if (mediaTabs.isEmpty()) {
        QListWidgetItem *emptyItem = new QListWidgetItem("暂无媒体播放");
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
        mediaListWidget->addItem(emptyItem);
        return;
    }

    QStringList keys = mediaTabs.keys();
    for (const QString &key : keys) {
        int tabId = key.toInt();
        QJsonObject state = mediaTabs[key].toObject();

        QString title = state["title"].toString("未知");
        QString artist = state["artist"].toString("");
        bool playing = state["playing"].toBool(false);
        qint64 durationMs = static_cast<qint64>(state["durationMs"].toDouble(0));
        qint64 positionMs = static_cast<qint64>(state["positionMs"].toDouble(0));

        // 格式化时间 mm:ss / mm:ss
        QString posStr = QString("%1:%2")
            .arg(positionMs / 60000, 2, 10, QChar('0'))
            .arg((positionMs / 1000) % 60, 2, 10, QChar('0'));
        QString durStr = QString("%1:%2")
            .arg(durationMs / 60000, 2, 10, QChar('0'))
            .arg((durationMs / 1000) % 60, 2, 10, QChar('0'));

        QString statusIcon = playing ? "\u25b6" : "\u23f8";
        QString activeMarker = (tabId == activeTabId) ? " \u2605" : "";

        QString displayText = QString("%1 %2 - %3  [%4 / %5]%6")
            .arg(statusIcon, title, artist, posStr, durStr, activeMarker);

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, tabId);  // 存储 tabId

        if (tabId == activeTabId) {
            item->setSelected(true);
        }

        mediaListWidget->addItem(item);
    }
}

int Widget::getSelectedTabId()
{
    QList<QListWidgetItem*> selected = mediaListWidget->selectedItems();
    if (!selected.isEmpty()) {
        return selected.first()->data(Qt::UserRole).toInt();
    }
    return activeTabId;  // 默认使用 activeTabId
}

void Widget::onPlayPauseClicked()
{
    int tabId = getSelectedTabId();
    if (tabId > 0) {
        QByteArray payload = QByteArray("{\"tabId\":") + QByteArray::number(tabId) + "}";
        ipcHost->sendCommand("PlayPause", payload);
    } else {
        ipcHost->sendCommand("PlayPause");
    }
}

void Widget::onNextClicked()
{
    int tabId = getSelectedTabId();
    if (tabId > 0) {
        QByteArray payload = QByteArray("{\"tabId\":") + QByteArray::number(tabId) + "}";
        ipcHost->sendCommand("Next", payload);
    } else {
        ipcHost->sendCommand("Next");
    }
}

void Widget::onPrevClicked()
{
    int tabId = getSelectedTabId();
    if (tabId > 0) {
        QByteArray payload = QByteArray("{\"tabId\":") + QByteArray::number(tabId) + "}";
        ipcHost->sendCommand("Prev", payload);
    } else {
        ipcHost->sendCommand("Prev");
    }
}

void Widget::onPipClicked()
{
    int tabId = getSelectedTabId();
    if (tabId > 0) {
        QByteArray payload = QByteArray("{\"tabId\":") + QByteArray::number(tabId) + "}";
        ipcHost->sendCommand("TogglePip", payload);
    } else {
        ipcHost->sendCommand("TogglePip");
    }
}

void Widget::onMediaItemDoubleClicked(QListWidgetItem *item)
{
    int tabId = item->data(Qt::UserRole).toInt();
    if (tabId > 0) {
        QByteArray payload = QByteArray("{\"tabId\":") + QByteArray::number(tabId) + "}";
        ipcHost->sendCommand("PlayPause", payload);
    }
}

//========== 诊断面板实现 ==========

void Widget::updateNmStatus()
{
    QString statusIcon;
    QString statusText;
    if (ipcHost) {
        int clients = ipcHost->clientCount();
        if (clients > 0) {
            statusIcon = "\xf0\x9f\x9f\xa2"; // Green circle
            statusText = QString("\xe5\xb7\xb2\xe8\xbf\x9e\xe6\x8e\xa5 (%1 \xe5\xae\xa2\xe6\x88\xb7\xe7\xab\xaf)").arg(clients); // 已连接 (x 客户端)
        } else if (ipcHost->isListening()) {
            statusIcon = "\xf0\x9f\x9f\xa1"; // Yellow circle
            statusText = "\xe7\xad\x89\xe5\xbe\x85\xe8\xbf\x9e\xe6\x8e\xa5..."; // 等待连接...
        } else {
            statusIcon = "\xf0\x9f\x94\xb4"; // Red circle
            statusText = "\xe6\x9c\x8d\xe5\x8a\xa1\xe5\xbc\x82\xe5\xb8\xb8"; // 服务异常
        }
    } else {
        statusIcon = "\xf0\x9f\x94\xb4";
        statusText = "\xe5\xb7\xb2\xe6\x96\xad\xe5\xbc\x80"; // 已断开
    }
    nmStatusLabel->setText(QString("%1 %2 | \xe5\x8f\x91\xe9\x80\x81: %3  \xe6\x8e\xa5\xe6\x94\xb6: %4")
        .arg(statusIcon, statusText)
        .arg(msgSentCount)
        .arg(msgRecvCount));
}

void Widget::onNmLog(const QString &log)
{
    // 根据方向更新计数
    if (log.contains("\xe2\xac\x86")) {
        msgSentCount++;
    } else if (log.contains("\xe2\xac\x87")) {
        msgRecvCount++;
    }
    updateNmStatus();

    // 追加日志并自动滚动到底部
    nmLogView->append(log);
    QTextCursor cursor = nmLogView->textCursor();
    cursor.movePosition(QTextCursor::End);
    nmLogView->setTextCursor(cursor);
}

void Widget::onNmDisconnected()
{
    updateNmStatus();
}

void Widget::onRefreshClicked()
{
    ipcHost->sendCommand("GetMediaList");
}
