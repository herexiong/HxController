# HxMonitor — 架构概述

> **层级：** 架构层（中层）  
> **上级文档：** [CLAUDE.md](../CLAUDE.md)  
> **下级文档：** [codebase_reference.md](codebase_reference.md) | [known_issues.md](known_issues.md)

---

## 模块划分

```
┌─────────────────────────────────────────────────────┐
│                    HxMonitor.exe                     │
│                                                     │
│  ┌──────────┐   ┌──────────────┐   ┌─────────────┐ │
│  │  main.cpp │   │    Widget    │   │MonitorLable │ │
│  │  启动控制  │──▶│  主窗口控制器 │──▶│ 硬件监控组件 │ │
│  └──────────┘   └──────┬───────┘   └─────────────┘ │
│                         │                            │
│              ┌──────────┴──────────┐                │
│              ▼                     ▼                │
│    ┌──────────────────┐  ┌──────────────────┐      │
│    │ LocalServerHost  │  │   QSerialPort    │      │
│    │     IPC 服务     │  │   串口发送       │      │
│    └──────────────────┘  └──────────────────┘      │
└─────────────────────────────────────────────────────┘
         │  Named Pipe (JSON)             │
         ▼                                ▼
  MediaControl Proxy                  外部硬件设备
  (代理层)                          （单片机/显示屏）
```

### 各模块职责

| 模块 | 文件 | 职责 |
|------|------|------|
| **启动控制** | `main.cpp` | 单实例运行防多开、UAC 提权检测、日志系统初始化 |
| **主窗口** | `widget.h/cpp` | 统筹所有功能：子进程管理、串口通信、媒体控制 UI、诊断面板 |
| **IPC 通讯** | `localserverhost.h/cpp` | `QLocalServer` 命名管道协议读写（直接进行 JSON 行读取） |
| **硬件展示** | `monitorlabel.h/cpp` | 单个监控指标的 UI 组件（标题 + 网格信息标签） |

---

## 启动模式

程序支持两种启动模式，`main.cpp` 在启动时立即检测单实例和挂载：

| 模式 | 触发条件 | UAC 提权 | 终端日志 | 日志文件 |
|------|----------|----------|----------|----------|
| **普通模式** | 无参数 | ✅ 自动 | ❌ | ✅ |
| **Debug 模式** | `--debug` 参数 | ❌ 跳过 | ✅ stderr | ✅ |

> 单实例防多开机制：使用 `QLocalSocket` 尝试连接 `MediaControlHubPipe`，如果连接成功说明已在运行，从而直接退出本次启动。

日志文件路径：`<应用目录>/HxMonitor_debug.log`（Append 模式，所有模式均写入）

---

## 数据流

### 路径 A — 硬件监控

```
CmdMonitor.exe
    │ stdout（BEGIN/END 文本帧）
    ▼
Widget::readProcessData()      ← QProcess::readyReadStandardOutput 触发
    │ 分帧 → emit recivedone()
    ▼
Widget::resolvedata()
    ├──▶ MonitorLable 组件刷新（GUI）
    └──▶ QSerialPort::write(JSON)    ← 仅串口已连接时
```

CmdMonitor 数据帧格式：
```
BEGIN
-> CPU
usage 45% power 35W temp 65°C
-> GPU
...
END
```

### 路径 B — 浏览器媒体控制 (通过 Proxy)

```
MediaControl Proxy
    │ LocalSocket (JSON \\n)
    ▼
LocalServerHost::onReadyRead()
    ├── emit messageReceived(QByteArray)   ──▶ Widget::onNativeMessage()
    │                                              ├──▶ 解析 tabs → updateMediaListUI()
    │                                              └──▶ 刷新诊断面板
    └── emit logMessage(QString)           ──▶ Widget::onNmLog() → nmLogView

用户点击控制按钮
    ▼
Widget::onXxxClicked()
    ▼
LocalServerHost::sendCommand(name, payloadJson)
    │ broadcast (JSON \\n 向所有 Socket 广播)
    ▼
MediaControl Proxy
    │ Socket (res 响应 + evt/MediaList 推送)
    ▼
（再次进入路径 B 顶部）
```

### 诊断面板信号流

```
LocalServerHost
    ├── logMessage(QString) ──▶ Widget::onNmLog()
    │                              ├── 更新 msgSentCount / msgRecvCount
    │                              ├── 追加到 nmLogView（暗色日志区）
    │                              └── updateNmStatus()
    └── serverStatusChanged()──▶ Widget::updateNmStatus()（根据新连接数量改变颜色）
```

---

## 关键设计决策

**QByteArray 替代 QJsonObject 作为跨组件接口**  
Qt 6.9.0 MinGW 构建的 `Qt6Core.dll` 中 `comparesEqual(QJsonObject)` 未导出，若 `QJsonObject` 出现在 moc 处理的头文件 signal/slot 参数中会引发运行时崩溃。所有跨模块传递均使用 `QByteArray`（raw JSON），`QJsonObject` 仅在 `.cpp` 内部使用。  
→ 详见 [known_issues.md](known_issues.md#qt-690-compareequalqjsonobject-符号缺失)

---

## 详细参考

- 每个类的完整成员列表：[codebase_reference.md](codebase_reference.md)
- Native Messaging 消息格式与命令清单：[../NativeMessagingProtocol.md](../NativeMessagingProtocol.md)
- 编译与部署步骤：[../BUILD.md](../BUILD.md)
- 已知问题与陷阱：[known_issues.md](known_issues.md)
