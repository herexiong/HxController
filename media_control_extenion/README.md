# Media Control Hub

**Media Control Hub** 是一个用于突破浏览器沙盒限制、实现**真正的全局跨应用媒体控制**的解决方案。它由一个浏览器扩展（主控枢纽）、一个中间代理层（Proxy）和一个本地桌面程序（宿主）协同组成。

## 🎯 痛点与目标

开发者在开发基于 Web 或桌面的媒体控制器时，通常无法在外部控制浏览器内的网页视频。本项目通过建立双向通信通道，使得外部程序（如快捷键键盘、甚至未来的串口/USB控制器）能直接读写浏览器中各个标签页（如 YouTube、Bilibili）的播放状态并进行完整的双向操作控制。

## 🧩 核心三层协同架构

1. **扩展层 (Extension) - 统筹全局**
   - **Content Script（探针与触手）**: 注入每个存在的网页，提取网页播放器的标题与封面图。它是唯一能直接执行 `play()/pause()` DOM操作的地方。
   - **Service Worker (中枢大脑)**: MV3后台脚本，汇总所有标签页的媒体状态，计算出当前的 **Active Tab**，并负责通过 `Native Messaging` 发送长连接。
   - **Popup UI (展示面板)**: Vue 3 控制板。
2. **中间代理通信层 (Proxy) - 防冲突屏障**
   - 依赖 Chrome 的 **Native Messaging** 机制。浏览器启动扩展时，会根据注册表拉起该代理进程 (`e2c_proxy.exe`)。
   - 因为无图形界面，它透明处理并桥接浏览器的 `stdin/stdout`，通过 Named Pipe 转发报文，完美解决多浏览器窗口触发的多进程 UI 实例冲突问题。
3. **桌面控制中枢 (HxMonitor) - Qt 终端**
   - 作为唯一的服务端应用监听本地计算机的 Named Pipe。不管连入多少个浏览器触发的 Proxy，都能全盘接受状态。它将全局物理热键命令（如播放、下一曲）通过管道广播 (Broadcast) 给所有连接着的代理客户端。

## 🛠 快速上手 (使用方法)

### 1. 编译并加载浏览器扩展
你需要 Node.js 环境。
```bash
npm install
npm run build
```
编译产物位于 `dist/`。打开你的浏览器（Chrome/Edge），在 `chrome://extensions` 页面开启开发者模式，点击“加载已解压的扩展程序”，选中 `dist/` 文件夹。
**记住** 你在浏览器上看到的生成扩展 ID（如: `nbmkfkfceagckncedpdhpfjdfoffageb`）。

### 2. 注册并部署 E2C Proxy
1. 前往 `native-host/com.hxmonitor.proxy.json`。
2. 确保 `allowed_origins` 里的 ID 与上一步你在浏览器中生成的 ID 是**一致的**。
3. 把你编译好的代理层程序放入 `native-host` 目录下，并确保 JSON 中的 `path` 填写的是**绝对路径**且指向了它。
4. 在当前目录下，**以管理员身份运行 `install.bat`**。
5. 去你的扩展页面刷新（Reload）扩展，插件图标变绿并且没有 Offline 错误标志，即为连接成功。

### 3. 运行 HxMonitor 桌面端
使用 Qt 启动桌面 GUI 端。按下系统设定的热键（如果适用）或点击 UI 按钮即可在应用外遥控网页媒体！

## 📚 文档索引

深度的开发者设计图表、模块边界、协议规定请参阅以下导航：

| 范畴 | 详情阅读路径 |
|---|---|
| **底层 IPC 双向设计总览图** | [doc/01-架构设计.md](./doc/01-架构设计.md) |
| **功能切片与各模块详细职责** | [doc/02-模块职责.md](./doc/02-模块职责.md) |
| **详细 Native Messaging Payload 定义**| [doc/08-NativeMessaging协议规范.md](./doc/08-NativeMessaging协议规范.md) |
| **功能切片与各模块详细职责** | [doc/07-功能点索引.md](./doc/07-功能点索引.md) 与 [doc/02-模块职责.md](./doc/02-模块职责.md) |
