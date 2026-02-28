# HxMonitor — 代码库详细参考

> **层级：** 细节参考层  
> **上级文档：** [architecture.md](architecture.md)（架构概述）

本文件是类/成员/信号的完整 API 参考，供需要了解具体实现细节时查阅。

---

## `main.cpp` — 全局状态与启动逻辑

### 全局变量

| `g_debugMode` | `bool` | `--debug` 参数触发，启用终端日志 |
| `g_logFile` | `QFile*` | 日志文件句柄（始终 Append 写入） |

### 函数

**`customMessageHandler(QtMsgType, QMessageLogContext, QString)`**  
Qt 自定义消息处理器（通过 `qInstallMessageHandler` 注册）：
- `g_debugMode` 时写 stderr（`[DEBUG]`/`[WARN]`/`[ERROR]`/`[FATAL]` 前缀）
- 始终写 `g_logFile`

**`restartWithAdminPrivileges(appPath) → bool`**  
若当前非管理员，`ShellExecuteEx(runas)` 重启自身，返回 false（当前进程退出）。已是管理员返回 true。

---

## `Widget` 类

### 成员变量

#### 硬件监控相关

| 成员 | 类型 | 说明 |
|------|------|------|
| `process` | `QProcess*` | 运行 `CmdMonitor.exe` 的子进程 |
| `recivedata` | `QVector<QString>*` | 当前帧的数据行缓冲 |
| `widgetisinit` | `bool` | `MonitorLable` 组件是否已首次创建 |
| `widgetobj` | `QVector<MonitorLable*>*` | 所有监控组件指针列表 |
| `logbtn` | `QPushButton*` | 启动 CmdMonitor LogMode 的按钮 |
| `combox` | `QComboBox*` | 串口选择下拉框 |
| `comlabel` | `QLabel*` | 串口号标签 |
| `connectbtn` | `QPushButton*` | 串口连接/断开按钮 |
| `ports` | `QVector<QString>` | 可用串口列表缓存（用于避免重复刷新） |
| `serialPort` | `QSerialPort*` | 串口通信对象（115200 8N1） |

#### 媒体控制相关

| 成员 | 类型 | 说明 |
|------|------|------|
| `ipcHost` | `LocalServerHost*` | IPC 通讯核心服务 |
| `mediaListWidget` | `QListWidget*` | 媒体列表（item 的 `Qt::UserRole` 存 tabId） |
| `playPauseBtn` | `QPushButton*` | 播放/暂停按钮 |
| `nextBtn` | `QPushButton*` | 下一首（+10s） |
| `prevBtn` | `QPushButton*` | 上一首（-10s） |
| `pipBtn` | `QPushButton*` | 画中画切换 |
| `mediaTabsRaw` | `QByteArray` | 当前 tabs 状态（`payload.tabs` 的原始 JSON） |
| `activeTabId` | `int` | 当前活跃标签页 ID（-1 表示无） |

#### 诊断面板相关

| 成员 | 类型 | 说明 |
|------|------|------|
| `nmStatusLabel` | `QLabel*` | 🟢/🔴 状态 + 收发计数 |
| `nmLogView` | `QTextEdit*` | 只读暗色日志区（Consolas 11px，高 150px） |
| `refreshBtn` | `QPushButton*` | 手动发送 `GetMediaList` |
| `msgSentCount` | `int` | 已发送消息计数（⬆ 方向） |
| `msgRecvCount` | `int` | 已接收消息计数（⬇ 方向） |

#### 系统托盘相关

| 成员 | 类型 | 说明 |
|------|------|------|
| `trayIcon` | `QSystemTrayIcon*` | 系统托盘图标 |
| `trayIconMenu` | `QMenu*` | 托盘右键菜单 |
| `quitAction` | `QAction*` | "退出~" 动作（`qApp->quit()`） |

> `minimizeAction`、`maximizeAction`、`restoreAction` 已声明但未使用。`runAsAdmin()` 已声明但无实现。

### 信号

| 信号 | 触发场景 |
|------|---------|
| `recivedone()` | `readProcessData()` 收到 END 帧 |

### 槽函数（硬件监控）

| 槽 | 触发 | 功能 |
|----|------|------|
| `readProcessData()` | `QProcess::readyReadStandardOutput` | 按行读取 BEGIN/END 帧，缓冲数据行 |
| `resolvedata()` | `recivedone` 信号 | 解析数据 → UI 刷新 + JSON 串口发送 |
| `startCmdMonitorProcess(args)` | 构造时调用 | 若进程未运行则启动 CmdMonitor.exe |
| `RefreshPort()` | `resolvedata` 内调用 | 更新串口下拉列表（仅列表变化时刷新） |
| `USART(port)` | 连接按钮点击 | 打开或关闭指定串口 |

### 槽函数（媒体控制）

| 槽 | 功能 |
|----|------|
| `onNativeMessage(QByteArray)` | 解析 NM 消息，路由 `evt/MediaList` 和 `res/GetMediaList` |
| `updateMediaListUI()` | 根据 `mediaTabsRaw` 重绘 `mediaListWidget` |
| `getSelectedTabId() → int` | 返回选中项 tabId，未选中时返回 `activeTabId` |
| `onPlayPauseClicked()` | 发送 `cmd/PlayPause` |
| `onNextClicked()` | 发送 `cmd/Next` |
| `onPrevClicked()` | 发送 `cmd/Prev` |
| `onPipClicked()` | 发送 `cmd/TogglePip` |
| `onMediaItemDoubleClicked(item)` | 发送 `cmd/PlayPause`（指定双击项的 tabId） |

### 槽函数（诊断面板）

| 槽 | 功能 |
|----|------|
| `updateNmStatus()` | 更新 `nmStatusLabel`（图标 + 计数） |
| `onNmLog(QString)` | 追加日志到 `nmLogView`，更新计数，调用 `updateNmStatus()` |
| `onNmDisconnected()` | 调用 `updateNmStatus()`（反映断开状态） |
| `onRefreshClicked()` | 调用 `nativeHost->sendCommand("GetMediaList")` |

### 媒体列表项格式

```
▶ / ⏸  标题 - 来源  [mm:ss / mm:ss]  ★
```

- `▶` = 播放中，`⏸` = 暂停
- `★` = 当前 `activeTabId` 对应的标签页
- `Qt::UserRole` 存储整型 `tabId`，用于按钮操作定位目标

### 窗口事件

| 事件 | 行为 |
|------|------|
| `closeEvent` | 隐藏窗口（不退出，系统托盘仍可见） |
| `hideEvent` | 隐藏窗口 |
| 托盘图标单击 | `showNormal()` 恢复 |
| 托盘右键"退出~" | `qApp->quit()` |

---

## `LocalServerHost` 类

### 公共接口

**`sendCommand(name: QString, payloadJson: QByteArray = "{}") → int`**  
封装 `{type:"cmd", name, id, payload}` 信封并广播向所有已连接代理，返回自增的消息 id。

**`isConnected() → bool`**  
返回当前是否有客户端成功连接。

### 信号

| 信号 | 参数 | 触发时机 |
|------|------|---------|
| `messageReceived` | `QByteArray` | 接收到来自代理的完整 JSON 行数据时 |
| `logMessage` | `QString` | 每次收发/连接状态改变 |
| `serverStatusChanged`| — | 服务端有客户端连接或者断开时 |

### 私有成员

| 成员 | 类型 | 说明 |
|------|------|------|
| `m_server` | `QLocalServer*` | IPC 监听服务端 |
| `m_clients` | `QList<QLocalSocket*>` | 已连接的客户端 Socket 列表 |
| `m_nextId` | `int` | 命令 id 自增计数器 |

### 消息帧格式

```
[JSON UTF-8 字节]\n
```

---

## `MonitorLable` 类

### 数据结构

```cpp
struct MonitorLableNode {
    QString title;       // 模块标题（如 "CPU"）
    QStringList infolist; // 指标值列表
};
```

### 布局

```
QVBoxLayout
  ├── QLabel (标题, bold, 16px)
  └── QGridLayout (3列)
        ├── QLabel info[0]  QLabel info[1]  QLabel info[2]
        └── QLabel info[3]  ...
```

### 方法

| 方法 | 说明 |
|------|------|
| `RefreshMonitorLable(node)` | 用新数据更新已有标签文本 |

---

## `HxMonitor.pro` 构建配置

| 配置项 | 值 |
|--------|----|
| Qt 模块 | `core gui widgets serialport` |
| C++ 标准 | `c++17` |
| 构建目标 | `HxMonitor` |
| 构建目录 | `build/Desktop_Qt_6_9_0_MinGW_64_bit-Debug/` |
