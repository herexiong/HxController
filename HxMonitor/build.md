# BUILD.md — 编译、部署与启动指南

> **层级：** 操作层  
> **上级文档：** [CLAUDE.md](CLAUDE.md)

---

## 环境要求

| 组件 | 路径 |
|------|------|
| Qt SDK | `C:\Qt\6.9.0\mingw_64\` |
| MinGW 工具链 | `C:\Qt\Tools\mingw1310_64\` |
| qmake | `C:\Qt\6.9.0\mingw_64\bin\qmake.exe` |
| mingw32-make | `C:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe` |

---

## 编译步骤

### 1. 设置环境变量（每次新终端会话）

```powershell
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
```

CMD：
```cmd
set PATH=C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%
```

### 2. 进入 build 目录并生成 Makefile

```powershell
cd build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug
qmake.exe ../../HxMonitor.pro -spec win32-g++ "CONFIG+=debug"
```

### 3. 编译

```powershell
mingw32-make.exe -j8
```

产物：`debug\HxMonitor.exe` 或 `release\HxMonitor.exe`（取决于构建配置）

### 4. 部署 Qt 运行时 DLL

> [!IMPORTANT]
> 每次 `clean` 或重建后必须重新执行此步骤，否则启动时报"找不到 Qt6Core.dll"。

```powershell
# 若编译 Debug 版本
windeployqt.exe debug\HxMonitor.exe
# 若编译 Release 版本
windeployqt.exe release\HxMonitor.exe
```

### 5. 清理

```powershell
mingw32-make.exe clean       # 删除编译产物
mingw32-make.exe distclean   # 删除编译产物 + Makefile
```

---

## 一键编译脚本

### Debug 版本

```powershell
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
Push-Location build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug
mingw32-make.exe -j8
windeployqt.exe debug\HxMonitor.exe
Pop-Location
```

### Release 版本

```powershell
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
Push-Location build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug
qmake.exe ../../HxMonitor.pro -spec win32-g++ "CONFIG+=release"
mingw32-make.exe -j8
windeployqt.exe release\HxMonitor.exe
Pop-Location
```

---

## 启动模式

### 普通模式（默认）

双击 `HxMonitor.exe` 或不带参数运行。程序自动检测管理员权限，不足时弹出 UAC 提示重启。

### Debug 模式（开发调试）

```powershell
.\debug\HxMonitor.exe --debug
```

- 附加到当前终端（`AttachConsole`），日志输出到 stderr
- **跳过 UAC 提权**，进程权限与终端一致（`taskkill` 可正常结束进程）
- 窗口标题显示 `[DEBUG]`

示例输出：
```
========================================
  HxMonitor V0.0  [DEBUG MODE]
  Log: C:/.../HxMonitor_debug.log
========================================
[DEBUG] [LocalServerHost] Server listening on MediaControlHubPipe
[DEBUG] [LocalServerHost] Client connected. Total clients: 1
[DEBUG] [LocalServerHost] ⬆ cmd/GetMediaList id=1 (56 bytes) {...}
```

### 日志文件

所有模式下，日志均写入（Append）：`<应用目录>/HxMonitor_debug.log`

查看最新日志：
```powershell
Get-Content ".\debug\HxMonitor_debug.log" | Select-Object -Last 50
```
