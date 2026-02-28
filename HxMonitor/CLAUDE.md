# HxMonitor — 项目导航

> **面向：** LLM 助手 / 新接手的开发者  
> **作用：** 快速建立项目全局认知，所有详细信息均通过链接指向专项文档

---

## 项目一句话描述

HxMonitor 是一个 **Windows Qt 6 桌面程序**，同时承担两个职责：
1. 采集硬件状态（CPU/GPU/内存/网络）并通过串口发送给外部设备
2. 作为本地 IPC 服务端（MediaControlHubPipe），与媒体控制代理（Proxy）双向通讯以控制浏览器媒体播放

---

## 文档地图

| 文档 | 层级 | 内容 |
|------|------|------|
| **本文件** `CLAUDE.md` | 顶层导航 | 项目概览、文档索引、快速上手 |
| [doc/architecture.md](doc/architecture.md) | 架构层 | 模块划分、数据流、组件关系图 |
| [BUILD.md](BUILD.md) | 操作层 | 编译、部署、启动模式 |
| [NativeMessagingProtocol.md](NativeMessagingProtocol.md) | 专项协议 | 与代理通信的 JSON 消息格式与命令清单 |
| [doc/codebase_reference.md](doc/codebase_reference.md) | 细节参考 | 每个类的成员、方法、信号完整列表 |
| [doc/known_issues.md](doc/known_issues.md) | 细节参考 | 已知陷阱、运行时错误、历史决策记录 |

> **阅读顺序建议**：`CLAUDE.md` → `doc/architecture.md` → 按需查阅专项文档

---

## 技术栈速查

| 项目 | 值 |
|------|-----|
| 语言 | C++17 |
| UI 框架 | Qt 6.9.0（Core / Gui / Widgets / SerialPort / Network） |
| 编译工具链 | MinGW 13.1 64-bit（`mingw32-make` + `qmake`） |
| 目标平台 | Windows 10/11 x64 |
| IPC 管道名 | `MediaControlHubPipe` |

---

## 外部运行时依赖

程序运行时需要以下文件与 `HxMonitor.exe` 同目录或在指定路径中：

| 文件 | 预期路径 | 缺失后果 |
|------|----------|----------|
| Qt 运行时 DLL | exe 同目录（由 `windeployqt` 部署） | 启动报"找不到入口点" |
| `CmdMonitor.exe` | `<应用目录>/CmdMonitor/publish/CmdMonitor.exe` | 硬件监控功能不可用 |
| `icon.png` | `<应用目录>/icon.png` | 系统托盘图标显示为空 |

---

## 快速上手

### 编译

```powershell
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
cd build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug
mingw32-make.exe -j8
```

→ 详细步骤见 [BUILD.md](BUILD.md)

### 调试启动（终端日志模式）

```powershell
.\debug\HxMonitor.exe --debug
```

日志文件始终写入：`<应用目录>/HxMonitor_debug.log`

→ 启动模式说明见 [BUILD.md](BUILD.md#启动模式)

### IPC 联调 (原 Native Messaging)

前端与代理开发者需要阅读：[NativeMessagingProtocol.md](NativeMessagingProtocol.md)

---

## 源文件索引

```
HxMonitor/
├── CLAUDE.md                    ← 本文件
├── HxMonitor.pro                ← qmake 构建配置
├── BUILD.md                     ← 编译与部署指南
├── NativeMessagingProtocol.md   ← NM 协议规范（面向扩展开发者）
├── main.cpp                     ← 入口：日志初始化与单实例防护
├── widget.h / widget.cpp        ← 主窗口：硬件监控 + 媒体控制 + 诊断面板
├── localserverhost.h / .cpp     ← IPC 服务端通信协议实现 (NamedPipe)
├── monitorlabel.h / .cpp        ← 硬件监控数据展示组件
├── widget.ui                    ← Qt Designer 表单（800×600 空容器）
└── doc/
    ├── architecture.md          ← 架构概述（模块 + 数据流）
    ├── codebase_reference.md    ← 类/成员/信号完整参考
    └── known_issues.md          ← 已知问题与陷阱
```
