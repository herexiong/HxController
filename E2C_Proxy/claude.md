# E2C Proxy System Context

## 1. 项目概述 (Project Overview)
E2C Proxy 是一个极其轻量级的“无头 (Headless)”中转代理中间件。核心解决现代浏览器（Edge/Chrome）在触发 Native Messaging 时强制产生新独立进程导致的“多浏览器实例冲突”问题。它作为透明的“传声筒”，将浏览器的 `stdin`/`stdout` 数据以长连接方式安全桥接到系统唯一的 HxMonitor 命名管道 (Named Pipe) 服务端上。

## 2. 核心技术栈 (Tech Stack)
- **核心语言**: Go (Golang) V1.22+
- **构建输出**: 单文件极小型 Windows CLI Executable (`-ldflags="-H windowsgui -s -w"` 彻底隐藏命令行黑窗)。
- **核心依赖**: `gopkg.in/natefinch/npipe.v2` (用于 Windows 命名管道 `Named Pipe` 客户端侧通信)。
- **系统生态**: Microsoft Edge Native Messaging Host 注册表机制。

## 3. 目录结构与模块职责 (Directory Structure)
```text
/
├── doc/               # 开发者技术文档 (架构图、集成指南、通信协议说明)。
├── nativemsg/         # 标准 I/O 通信层，专门处理 Chrome/Edge 的 4 字节头部报文与控制。
├── pipeclient/        # 命名管道 IPC 通信层，负责 `\\.\pipe\MediaControlHubPipe` 的重连、读写。
├── proxy/             # 业务路由与生命周期管理层，绑定 `nativemsg` 与 `pipeclient` 的并发读写通道。
├── test_*.go          # 本地化的模拟测试工具 (Mock 服务端与全链路 Runner)。
├── install_edge.bat   # 自动将 Proxy 本地路径注册入 Windows 注册表的工具。
├── manifest.json      # Native Messaging 的关键宿主描述文件 (记录了合法扩展 ID)。
└── main.go            # 代理程序的执行入口，控制日志配置与初始化启动逻辑。
```

## 4. 架构与设计模式 (Architecture & Patterns)
- **并发路由模型 (Goroutines & Channels)**：
  - Proxy 核心启动了独立的 Goroutines 分别持续监听来自浏览器的 `os.Stdin` 与来自桌面的 `Named Pipe`。
- **降级与防阻塞机制 (Non-blocking Drop)**：
  - 当桌面端 (HxMonitor) 未启动 / 意外崩溃时，Proxy 会立刻尝试每 3 秒重连；此期间如果收到来自浏览器的状态汇报，将**无阻塞静默丢弃**（使用 `select default` 模式），避免通道积压引发内存溢出。
- **零僵尸进程生命周期 (Zero Zombie Processes)**：
  - 当 `stdin` 读取到 `EOF` 或者读取发生致命错误时（标志着浏览器或对应标签页被关闭），Proxy 必须**立刻安全执行 `os.Exit(0)`** 自毁，坚决不驻留后台。

## 5. 开发与构建命令 (Development & Build)
- **本地化集成测试**：
  `go run test_runner.go` （自动拉起 Proxy 和测试模拟器验证双向管道协议传输）。
- **生产无头 (Headless) 编译**：
  `go build -ldflags="-H windowsgui -s -w" -o e2c_proxy.exe main.go`
- **开发者重新格式化与依赖整理**：
  `go fmt ./...` 和 `go mod tidy`

## 6. 编码规范与 LLM 守则 (Coding Rules & LLM Directives)
- **[铁律 1] 防阻塞设计原则**：在 `proxy` 包的代码修改中，从 `chromeMsgs` 这样的 Channel 读取或写入数据时，绝对不能使用可能导致 Goroutine 死锁的阻塞写。必须配合 `select {}` 和超时间隙/静默丢弃策略。
- **[铁律 2] 通信格式红线**：原生消息前缀。任何由桌面发向 `stdout` 的 JSON，在经过 `nativemsg.WriteMessage` 之前，务必确保它是单纯的 JSON 字节流，**千万不要在其中夹带常规的 `fmt.Println`/`log.Print` 脏日志打印在 stdout 上**，这会污染 4 字节的头协议，立刻导致 Edge/Chrome 报错并强行断开连接！如果需要排查问题，必须强制将日志输出重定向至本地实体文件。
- **[铁律 3] 命名管道换行符约定**：向命名管道发送指令时，务必在 JSON 结尾手工附带 `\n` 换行符（反之读取时要使用 `\n` 裁剪），这是确保长连接正确按段解析的关键约定。
- **[联动修改约束]**：如果你变更了此项目的管道名称或启动机制，必须同步修改 `doc/integration_guide.md` 和 `doc/architecture_and_roadmap.md`，以告知最终端和前端开发者。如果你修改了项目允许的跨域扩展 ID，务必提示开发者重新运行 `install_edge.bat` 更新 Windows 注册表。
