---
title: 通讯协议（Native Messaging）— 概览
status: implemented
updated: 2026-02-23
---

> 本文档为**协议概览层**，速览架构要点与消息清单。  
> 完整 payload 结构、字段说明和交互流程图请见 [08-NativeMessaging协议规范.md](./08-NativeMessaging协议规范.md)。

---

# 1. 基本参数

| 项 | 值 |
|---|---|
| Host Name | `com.hxmonitor.proxy` |
| 连接方式 | `chrome.runtime.connectNative()` 长连接 |
| 传输层 | stdin/stdout，`[4字节小端uint32长度][UTF-8 JSON]` |
| 消息上限 | ≤ 256KB |
| stdin 读取 | `QTimer` 10ms 轮询 + `PeekNamedPipe`（Windows pipe 兼容） |

# 2. 消息信封（Envelope）

```json
{ "type": "cmd|res|evt", "name": "...", "id": 1, "replyTo": 1, "ts": 0, "payload": {} }
```

| type | 方向 | 说明 |
|---|---|---|
| `cmd` | Desktop → Extension | 请求，Extension 必须回 `res` |
| `res` | Extension → Desktop | 响应，携带 `replyTo` |
| `evt` | Extension → Desktop | 推送，无需回执 |

# 3. 命令清单（Desktop → Extension）

| 命令 | 说明 |
|---|---|
| `PlayPause` | 播放/暂停切换，可选 `tabId` |
| `Next` | 下一首（快进 10s） |
| `Prev` | 上一首（后退 10s） |
| `TogglePip` | 画中画切换 |
| `GetMediaList` | 请求当前所有媒体标签快照 |

所有命令响应格式：`{ "ok": true/false }`

> **GetMediaList 特殊处理**：若 SW 刚重启状态为空，扩展先广播 `queryState` 给所有 tab，600ms 后再回复，确保数据完整。

# 4. 事件清单（Extension → Desktop）

| 事件 | 触发时机 | 说明 |
|---|---|---|
| `MediaList` | 媒体状态变化（100ms debounce） | 推送完整 `{ activeTabId, tabs }` 快照 |

# 5. 连接建立流程

```
SW 启动 → connectNative() → 浏览器启动 HxMonitor.exe
  → NM Host 初始化（QTimer 轮询开始）
  → Desktop 发 GetMediaList → Extension 回 res/GetMediaList
  → Extension 主动推送 evt/MediaList（状态变化时）
```
