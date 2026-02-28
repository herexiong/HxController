# Media Control Hub — AI 助手上下文指南 (Claude/LLM Context)

> **建立目的**: 此文件专供 AI 助手（如 Claude, Cursor, Gemini 等）读取，用于快速理解本项目的核心脉络、架构约定和技术规范，避免偏离现有设计跑偏。

## 1. 项目简介
**Media Control Hub** 包含一个浏览器扩展和一个对应的 Windows 桌面 Native Messaging (NM) Host。
用来将浏览器里的视频、音频暴露给桌面操作系统，使桌面程序能够通过 Native Messaging 读取浏览器所有网页的媒体播放状态，并发送控制命令（如 播放、暂停、上一曲、下一曲）。

## 2. 架构模式 (三端协同)
本项目严格按照单一职责原则被划分为三个隔离的上下文：

### 2.1 UI 展示层 (`extension/ui/`)
- **技术栈**: Vue 3 + TypeScript + Vite
- **职责**: 仅负责卡片展示和用户交互。
- **边界**: 绝对不可以包含媒体控制逻辑！所有的指令都必须通过 `chrome.runtime.sendMessage` 发送给 Service Worker。

### 2.2 Hub Core / 中枢层 (`extension/sw/`)
- **技术栈**: MV3 Service Worker (TypeScript)
- **职责**: 全局唯一的“大脑”。用于集中维护所有 Tab 汇报上来的媒体状态（由 `store.ts` 管理）。处理扩展与桌面的长连接，并计算当前的 **Active Tab**。

### 2.3 探针与执行层 (`extension/content/`)
- **技术栈**: Content Script
- **职责**: **唯一允许操作 DOM 的地方**。探测页面媒体，通过 `navigator.mediaSession` 等提取元数据，被动接受 SW 下发的指令控制 `video/audio`。

### 2.4 中间代理通信层 (`proxy/` / `e2c_proxy`)
- **技术栈**: Go / Rust (Headless 无图形界面)
- **架构意义**: **破除 Native Messaging 多实例诅咒**。浏览器 `connectNative` 会**主动创建一个全新的独立进程**，如果直连桌面 GUI 会导致弹出一排托盘。本代理层作为透明的“传声筒”，将浏览器的 `stdin/stdout` 转换为 `Named Pipe (命名管道)` 发送给唯一的桌面端。

### 2.5 桌面控制中枢 (`HxMonitor` Qt App)
- **技术栈**: Qt6 + C++
- **职责**: 建立 `QLocalServer` 监听 Named Pipe。不管连入多少个 Proxy，全盘接受并聚合状态。当用户按下物理快捷键时，**广播 (Broadcast)** JSON 控制命令给所有相连的 Proxy。

## 3. 重要开发规范
1. **统一数据流向**: UI 操作 -> Hub Core -> Content Script 执行。
2. **消息信封**: 通过 Native Messaging 交换的 JSON 必须包括 { type: "cmd|res|evt", name: string, payload: any }。
3. **状态管理**: 不要依赖持久化存储。SW 经常被休眠，在唤醒和连接 NM 时，通过发送 `queryState` 让 Content Script 主动汇报当前状态来“重建状态树”。
4. **节流与防抖**: 
   - Content script 对 `timeupdate` (进度条) 的汇报限制为最高 1Hz。
   - SW 发向 Desktop 的状态变化采用 100ms debounce。

## 4. 关键文件路标
- `doc/08-NativeMessaging协议规范.md`: 桌面到扩展交互的绝对权威 JSON 协议规定！
- `doc/07-功能点索引.md`: 现有功能目录与对应的代码位置参考。
- `doc/01-架构设计.md`: 如果要了解时序图和组件交互流，看这里。
- `doc/05-技术实现要点.md`: 了解各种脏活累活（如 Bilibili 适配、跨 Frame 问题、重启恢复）的处理思路。
