# 构建与开发指南

本文档覆盖从零开始的完整开发环境配置，包括浏览器扩展编译、加载和桌面程序构建。

---

## 1. 浏览器扩展

### 1.1 安装依赖

本项目使用 npm 进行包管理，运行：

```bash
npm install
```

如遇 `vue-tsc` 版本兼容问题：

```bash
npm install vue-tsc@latest -D
```

### 1.2 编译

**开发模式（热更新）**

```bash
npm run dev
```

**生产构建**（产物输出到 `dist/`）

```bash
npm run build
```

### 1.3 加载到浏览器

1. 打开 Chrome 或 Edge，进入扩展管理页面：
   - Chrome: `chrome://extensions`
   - Edge: `edge://extensions`
2. 开启右上角 **开发者模式**。
3. 点击 **加载已解压的扩展程序 (Load unpacked)**。
4. 选择本项目根目录下的 **`dist/`** 文件夹。
5. 记录扩展 ID（格式如 `nbmkfkfceagckncedpdhpfjdfoffageb`），后续注册 NM Host 时需要用到。

---

## 2. Native Messaging Host 注册

扩展通过 Native Messaging 与桌面程序通信，需要先在系统注册表注册 Host。

### 2.1 确认扩展 ID

加载扩展后，在扩展管理页面获取扩展 ID，确认它与 `native-host/com.hxmonitor.proxy.json` 中 `allowed_origins` 的值一致：

```json
"allowed_origins": ["chrome-extension://<YOUR_EXTENSION_ID>/"]
```

如不一致，编辑 JSON 文件替换为正确 ID，然后重新执行注册。

### 2.2 注册（需管理员权限）

进入 `native-host/` 目录，右键以**管理员身份运行** `install.bat`。

脚本会将 Host 清单路径写入以下注册表位置：

```
HKCU\Software\Microsoft\Edge\NativeMessagingHosts\com.hxmonitor.proxy
HKCU\Software\Google\Chrome\NativeMessagingHosts\com.hxmonitor.proxy
```

### 2.3 卸载

右键以管理员身份运行 `native-host/uninstall.bat`，删除上述注册表项。

---

## 3. 桌面程序（HxMonitor）

桌面程序使用 Qt6 + MinGW 编译，详细步骤见 [`HxMonitor/build.MD`](../HxMonitor/build.MD)。

**快速编译（Debug）**

```powershell
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
cd "C:\File_Disk\workspace\HxMonitor\HxMonitor\build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug"
mingw32-make.exe -j8
```

> **注意**：编译前需关闭正在运行的 `HxMonitor.exe`（进程占用会导致链接失败）。

---

## 4. 项目结构

```
extension/       扩展源码
  ui/            Vue 3 Popup 界面
  sw/            Service Worker（Hub Core）
  content/       Content Scripts
  shared/        共享类型与协议定义
dist/            编译产物（加载此目录）
native-host/     NM Host 清单(代理)与注册脚本
doc/             设计文档
```

---

## 5. 常见问题

| 问题 | 解决方式 |
|---|---|
| `Extension context invalidated` | 扩展更新后的正常现象，刷新对应网页即可 |
| 样式不生效 | 确认加载的是 `dist/` 目录，而非源码目录 |
| 连接状态显示 Offline | 检查 NM Host 是否已注册，查看 `HxMonitor_debug.log` |
| `vue-tsc` 类型错误 | 执行 `npm install vue-tsc@latest -D` 后重新构建 |
