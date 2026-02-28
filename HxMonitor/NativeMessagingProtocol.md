# Native Messaging 通讯协议文档

> **层级：** 专项协议文档  
> **上级文档：** [CLAUDE.md](CLAUDE.md)  
> 面向：桌面程序开发者 & 浏览器扩展开发者  
> 版本：1.0.0 | 更新日期：2026-02-17

---

## 1. 概述

浏览器扩展（Media Control Hub）通过 Chrome Native Messaging 与桌面程序双向通信。

- **连接方式**：`chrome.runtime.connectNative()` 长连接
- **Host Name**：`com.media.control.host`
- **传输层**：stdin/stdout，每条消息格式为 `[4 字节小端长度][UTF-8 JSON]`
- **单条消息上限**：建议 ≤ 256KB

> **重要**：Host 的 stdout 只能输出协议消息。日志必须写到 stderr 或文件。

---

## 2. 消息结构（Envelope）

所有消息使用统一信封格式：

```json
{
  "type": "cmd | res | evt",
  "name": "命令/事件名称",
  "id": 1,
  "replyTo": 1,
  "ts": 1740000000000,
  "payload": {}
}
```

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `type` | `"cmd"` \| `"res"` \| `"evt"` | ✅ | 消息类型 |
| `name` | `string` | ✅ | 命令或事件名称 |
| `id` | `number` | `cmd` 必填 | 请求 ID，用于关联响应 |
| `replyTo` | `number` | `res` 必填 | 对应请求的 `id` |
| `ts` | `number` | ❌ | 发送时间戳（ms），可选 |
| `payload` | `object` | ✅ | 业务数据 |

### 消息类型说明

| type | 方向 | 说明 |
|------|------|------|
| `cmd` | Desktop → Extension | 命令请求，扩展必须返回 `res` |
| `res` | Extension → Desktop | 命令响应，必须携带 `replyTo` |
| `evt` | Extension → Desktop | 事件推送，无需回执 |

---

## 3. 命令清单（Desktop → Extension）

### 3.1 PlayPause — 播放/暂停切换

```json
{ "type": "cmd", "name": "PlayPause", "id": 1, "payload": {} }
```

带指定标签页：
```json
{ "type": "cmd", "name": "PlayPause", "id": 1, "payload": { "tabId": 123 } }
```

响应：
```json
{ "type": "res", "name": "PlayPause", "replyTo": 1, "payload": { "ok": true } }
```

### 3.2 Next — 下一首（快进 10s）

```json
{ "type": "cmd", "name": "Next", "id": 2, "payload": {} }
```

### 3.3 Prev — 上一首（后退 10s）

```json
{ "type": "cmd", "name": "Prev", "id": 3, "payload": {} }
```

### 3.4 TogglePip — 切换画中画

```json
{ "type": "cmd", "name": "TogglePip", "id": 4, "payload": {} }
```

### 3.5 GetMediaList — 请求当前媒体列表

```json
{ "type": "cmd", "name": "GetMediaList", "id": 5, "payload": {} }
```

响应（返回完整媒体列表快照）：
```json
{
  "type": "res",
  "name": "GetMediaList",
  "replyTo": 5,
  "payload": {
    "ok": true,
    "activeTabId": 123,
    "tabs": {
      "123": {
        "hasMedia": true,
        "playing": true,
        "title": "Never Gonna Give You Up",
        "artist": "youtube.com",
        "artwork": "https://i.ytimg.com/vi/.../hqdefault.jpg",
        "durationMs": 213000,
        "positionMs": 45000,
        "lastActiveAt": 1740000000000
      },
      "456": {
        "hasMedia": true,
        "playing": false,
        "title": "Some Video",
        "artist": "bilibili.com",
        "durationMs": 600000,
        "positionMs": 120000,
        "lastActiveAt": 1739999990000
      }
    }
  }
}
```

### 命令 Payload 字段参考

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `tabId` | `number` | ❌ | 目标标签页 ID。省略则使用当前 `activeTabId` |

### 响应 Payload 字段参考

| 字段 | 类型 | 说明 |
|------|------|------|
| `ok` | `boolean` | 命令是否执行成功 |
| `error` | `string` | 失败时的错误信息（可选） |

---

## 4. 事件清单（Extension → Desktop）

### 4.1 MediaList — 媒体列表变更推送

当浏览器中的媒体状态发生变化（播放/暂停/标签页关闭/新媒体检测到）时，扩展会**主动推送**完整的媒体列表。推送有 100ms 的 debounce 节流。

```json
{
  "type": "evt",
  "name": "MediaList",
  "ts": 1740000000000,
  "payload": {
    "activeTabId": 123,
    "tabs": {
      "123": {
        "hasMedia": true,
        "playing": true,
        "title": "Never Gonna Give You Up",
        "artist": "youtube.com",
        "artwork": "https://i.ytimg.com/vi/.../hqdefault.jpg",
        "durationMs": 213000,
        "positionMs": 45000,
        "lastActiveAt": 1740000000000
      }
    }
  }
}
```

> **注意**：`tabs` 为空对象 `{}` 表示当前没有任何标签页在播放媒体。`activeTabId` 为 `null` 表示没有活跃的媒体标签。

---

## 5. MediaState 字段参考

每个标签页的媒体状态包含以下字段：

| 字段 | 类型 | 说明 |
|------|------|------|
| `hasMedia` | `boolean` | 是否检测到媒体元素 |
| `playing` | `boolean` | 是否正在播放 |
| `title` | `string` | 媒体标题（来源优先级：Media Session > 站点适配 > document.title） |
| `artist` | `string` | 艺术家/站点名（来源：Media Session > hostname） |
| `artwork` | `string` | 封面图 URL（来源优先级：Media Session > og:image > video.poster > favicon） |
| `album` | `string` | 专辑名（可选，来自 Media Session） |
| `durationMs` | `number` | 总时长（毫秒） |
| `positionMs` | `number` | 当前播放位置（毫秒） |
| `lastActiveAt` | `number` | 最后活跃时间戳（Unix ms） |
| `windowId` | `number` | 浏览器窗口 ID |

---

## 6. 错误处理

### 命令执行失败

当命令无法执行时，响应 `ok: false` 并附带错误信息：

```json
{
  "type": "res",
  "name": "PlayPause",
  "replyTo": 1,
  "payload": { "ok": false, "error": "No active media tab found" }
}
```

常见失败原因：
- `activeTabId` 为 `null`（没有检测到任何媒体）
- 指定的 `tabId` 对应的标签页已关闭
- 目标标签页未注入 Content Script

### 连接断开

Native Messaging 连接断开后，扩展侧会记录错误并更新内部状态。桌面程序可通过检测 stdin/stdout EOF 来感知断连。

---

## 7. 交互流程示例

### 7.1 桌面程序发送播放/暂停命令

```
Desktop                    Extension (Hub Core)           Content Script
   |                            |                              |
   |--- cmd/PlayPause (id=1) -->|                              |
   |                            |--- execute PlayPause ------->|
   |                            |                   (toggle)   |
   |                            |<-- stateUpdate (playing) ----|
   |<-- res/PlayPause (ok) -----|                              |
   |<-- evt/MediaList ----------|                              |
```

### 7.2 用户在网页播放视频

```
Content Script              Extension (Hub Core)           Desktop
   |                            |                              |
   |--- stateUpdate (playing) ->|                              |
   |                            |--- evt/MediaList ----------->|
   |                            |    (100ms debounce)          |
```

### 7.3 桌面程序请求媒体列表快照

```
Desktop                    Extension (Hub Core)
   |                            |
   |--- cmd/GetMediaList (id=5)->|
   |<-- res/GetMediaList --------|
   |   { ok, activeTabId, tabs } |
```

---

## 8. 快速参考

| 命令名 | 方向 | 类型 | 说明 |
|--------|------|------|------|
| `PlayPause` | Desktop → Extension | `cmd` | 播放/暂停切换 |
| `Next` | Desktop → Extension | `cmd` | 下一首（快进 10s） |
| `Prev` | Desktop → Extension | `cmd` | 上一首（后退 10s） |
| `TogglePip` | Desktop → Extension | `cmd` | 切换画中画 |
| `GetMediaList` | Desktop → Extension | `cmd` | 获取当前媒体列表 |
| `MediaList` | Extension → Desktop | `evt` | 媒体状态变更推送 |
