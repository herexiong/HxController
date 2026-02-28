# E2C Proxy (Edge to Controller Proxy)

E2C Proxy 是针对 HxMonitor 桌面端应用开发的一款超轻量级、无图形界面的 Native Messaging 代理中间件。

由于现代浏览器（如 Microsoft Edge / Google Chrome）的 Native Messaging 机制会在连接时强制派生出全新的宿主进程。为了避免在多个浏览器配置（Profile）或多浏览器同时运行时产生严重的进程冲突，E2C Proxy 应运而生。

它负责在后台静默运行，将浏览器发出的所有数据请求安全、无缝地通过 Windows 系统级进程间通信（命名管道）转发给唯一的 HxMonitor 桌面主控程序。

## ✨ 核心特性

- **极致极简**：基于 Go 语言编写，编译后仅有一个体积几 MB 的轻量级单文件 `.exe`，没有任何外部运行库依赖。
- **无后台残留**：自带极其敏锐的进程防僵死机制，只要浏览器扩展断开连接或是关闭，Proxy 瞬间安全自毁，保证系统绝对干净。
- **透明透传**：充当完美的“传声筒”，只要接上就能用。
- **多开与并发支持**：支持用户在开启 5 个不同的 Edge 浏览器窗口时，平铺共存而不会抢占 HxMonitor 的底层资源。

## 🚀 快速上手 (面向用户)

对于普通用户来说，该程序的安装和卸载也是全自动化的：

1. **环境注册**：双击运行本目录下的 `install_edge.bat` 脚本（自动完成 Microsoft Edge 的注册表依赖挂载）。
2. **在浏览器使用**：安装您的 HxMonitor 配套浏览器扩展，当需要控制音乐或者媒体时，该 Proxy 会在您毫不知情的情况下在后台静默为您完成一切转发工作。
3. **环境卸载**：如果你需要卸载，请双击运行 `uninstall_edge.bat`。

## 🛠️ 获取技术支持 (面向开发者)

如果你是一名开发者，并且想要了解该程序的底层通信逻辑与架构设计，请参阅本项目的 [doc 目录](./doc/)：

- [系统架构与开发路线](./doc/architecture_and_roadmap.md)
- [Proxy 技术细节总览](./doc/technical_overview.md)
- [通信协议规范](./doc/protocol.md)
- [扩展与桌面端的代码接入指南](./doc/integration_guide.md)
