# Proxy (中间代理层) 架构设计

> 状态: 规划中  
> 目标选型: Go (Golang) 或 Rust，推荐 Go。

## 1. 为什么需要 Proxy 层？

在 Chrome/Edge 的 Native Messaging 架构中，当扩展（Service Worker）执行 `chrome.runtime.connectNative()` 时，浏览器会强行 `Spawn`（派生）出一个全新的宿主进程。
- 如果直接让 HxMonitor（桌面 GUI 程序）作为宿主被拉起，当用户拥有多个浏览器配置（Profile）或横跨 Chrome/Edge 双浏览器时，系统中就会同时拉起**多个同样的 GUI 程序**，导致严重的抢占和冲突。
- **解决方案**：让浏览器拉起一个极度轻便、无图形界面的“Headless”代理程序（即本 Proxy），由它通过操作系统级的 IPC（如 Windows 命名管道）去连接真正唯一的 HxMonitor 桌面程序，充当“传声筒”。

## 2. 核心职责

1. **Stdio 通信**：依照 Chrome Native Messaging 的 4 字节前缀 JSON 规范，读取 `stdin` 并写入 `stdout`。
2. **IPC 桥接**：启动后，尝试连接本地存在的固定命名管道（Named Pipe，例如 `\\.\pipe\MediaControlHubPipe`）。
3. **数据透传**：
   - 从 `stdin` 读取来自浏览器的 JSON -> 发送入 `Named Pipe`。
   - 从 `Named Pipe` 读取来自桌面的 JSON -> 发送入 `stdout`。
4. **生命周期自管理 (核心防遗留)**：
   - 只要从浏览器的 `stdin` 读取到 **EOF（End Of File）**，就说明母体（扩展 SW）死亡或断开。此时 Proxy **必须立刻执行退出 (`exit(0)`)**，绝不允许产生僵尸进程。

## 3. 设计优势

- **体积小巧**：使用 Go/Rust 编译出的单文件二进制，仅有几 MB 且无需运行库。
- **无状态**：Proxy 内部没有任何业务逻辑、状态记录或文件落盘。
## 4. 如何在 Microsoft Edge 中使用本 Proxy

由于目标浏览器为 **Microsoft Edge**，请按以下步骤安装和配置 Native Messaging Host：

### 步骤 1：修改 Manifest 授权
打开本目录下的 `manifest.json` 文件：
```json
{
  "name": "com.hxmonitor.proxy",
  ...
  "allowed_origins": [
    "chrome-extension://<YOUR_EDGE_EXTENSION_ID>/"
  ]
}
```
将 `<YOUR_EDGE_EXTENSION_ID>` 替换为你正在开发的 Edge 扩展 ID。

**注意坑点**：虽然是 Edge 浏览器，但是其 Native Messaging Host 清单语法中，`allowed_origins` 依然必须以 `chrome-extension://` 开头，而**不是** `extension://` 或 `edge-extension://`。

### 步骤 2：执行注册表写入
双击运行本目录下的 **`install_edge.bat`**。
该脚本会自动在当前用户的注册表中添加键值，告诉 Edge 浏览器名字叫 `com.hxmonitor.proxy` 的宿主在哪个路径（即指向该目录下的 `manifest.json`）。

> *(如需卸载，请双击运行 `uninstall_edge.bat`)*

### 步骤 3：在 Edge 扩展中连接
在你的 Service Worker / Background Script 中，通过以下代码连接这个 Proxy：
```javascript
const port = chrome.runtime.connectNative("com.hxmonitor.proxy");

port.onMessage.addListener((msg) => {
  console.log("收到来自桌面的消息:", msg);
});

port.postMessage({ type: "hello", data: "edge_extension" });
```
*(注意：在 Edge 开发中，你可以使用 `chrome.runtime` 也可以使用 `browser.runtime`，两者在此处表现一致)*
