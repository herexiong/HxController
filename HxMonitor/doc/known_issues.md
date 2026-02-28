# HxMonitor — 已知问题与陷阱

> **层级：** 细节参考层  
> **上级文档：** [architecture.md](architecture.md)（架构概述）

本文记录开发过程中遇到的非显而易见的问题、运行时陷阱和历史决策，避免重复踩坑。

---

## Qt 6.9.0 — `comparesEqual(QJsonObject)` 符号缺失

**现象**：程序启动时弹出"无法定位程序输入点 `comparesEqual` 于动态链接库 `Qt6Core.dll`"，程序立即退出。

**根因**：Qt 6.9.0 MinGW 构建的 `Qt6Core.dll` 中，`comparesEqual(QJsonObject const&, QJsonObject const&)` 虽然声明为 `Q_CORE_EXPORT`，但实际未导出。当 `QJsonObject` 出现在 moc 处理的头文件（`.h`）的 signal 或 slot 参数类型中，`moc` 生成的元对象代码会实例化该符号，引发运行时错误。

**解决方案**：  
所有跨模块（尤其是 signal/slot）的 JSON 传递一律使用 `QByteArray`（compact JSON 字节串）。`QJsonObject`、`QJsonDocument` 仅在 `.cpp` 文件内部使用，绝不出现在 `.h` 的 public/signals/slots 部分。

**受影响的接口**：
- `LocalServerHost::messageReceived(QByteArray)` ← 原本曾设计为 `QJsonObject`
- `LocalServerHost::sendCommand(name, QByteArray payloadJson)`
- `Widget::onNativeMessage(const QByteArray &rawJson)`

---

## 多进程锁定 EXE 导致链接失败

**现象**：`mingw32-make` 报 `cannot open output file debug\HxMonitor.exe: Permission denied`，链接阶段失败。

**根因**：程序以管理员权限运行后，UAC 提升会创建新进程，旧进程也可能残留。普通 PowerShell 会话的 `taskkill /F` 无法结束高权限进程，导致 exe 文件被锁定。

**解决方案**：
```powershell
# 方法1：以管理员身份运行（右键 PowerShell → 以管理员身份运行）
taskkill /F /IM HxMonitor.exe

# 方法2：任务管理器 → 详细信息 → 右键 HxMonitor.exe → 结束进程树
```

**预防措施**：调试时使用 `--debug` 启动，该模式跳过 UAC 提权，进程权限与终端一致，`taskkill` 可正常结束。

---

## 清理 Build 目录后 Qt DLL 丢失

**现象**：`mingw32-make clean` 或删除 build 目录后，重新编译的程序启动时报"找不到 Qt6Core.dll"或"无法定位程序输入点"。

**根因**：`windeployqt` 将 Qt 运行时 DLL 复制到可执行文件所在目录（如 `debug/` 或 `release/`）。`clean` 命令删除编译产物包括这些 DLL，但不会自动重新部署。

**解决方案**：每次 `clean` 后重新编译，再执行一次部署（根据你编译的目标选择）：
```powershell
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
# 如果是 Debug 编译
windeployqt.exe debug\HxMonitor.exe
# 如果是 Release 编译
windeployqt.exe release\HxMonitor.exe
```

---

## `QApplication::applicationDirPath()` 必须在 `QApplication` 构造后调用

**现象**：在 `QApplication a(argc, argv)` 之前调用 `QApplication::applicationDirPath()` 返回空字符串，导致日志文件创建在错误位置（通常是工作目录根）。

**解决方案**：日志文件路径的计算和 `g_logFile` 的初始化必须放在 `QApplication a(argc, argv)` 之后：
```cpp
QApplication a(argc, argv);
// 之后才能安全调用
QString logPath = QCoreApplication::applicationDirPath() + "/HxMonitor_debug.log";
```

---

## 媒体列表为空的排查步骤

若 IPC 控制正常（Play/Pause 有效）但媒体列表显示"暂无媒体播放"：

1. **读取日志文件**确认 `onNativeMessage` 是否被调用：
   ```powershell
   Get-Content ".\HxMonitor_debug.log" | Select-Object -Last 50
   ```
2. 日志中查找 `[Widget] tabs keys:`，确认 `count` 是否为 0
3. 若收到消息但 `tabs` 为空：代理/扩展侧尚未检测到媒体，在浏览器中播放任意视频/音频后点击"🔄 刷新"
4. 若 `onNativeMessage` 未被调用：检查 IPC 传输或者代理转发的消息的 `type`/`name` 是否与协议一致（见 [../NativeMessagingProtocol.md](../NativeMessagingProtocol.md)）
