# HxController

`HxController` 是一套面向 Windows 的桌面控制与硬件监控方案，当前仓库主要包含：

- `HxMonitor`：基于 Qt 6 的桌面主控程序，负责硬件遥测采集、串口桥接、浏览器媒体控制和本地 IPC。
- `media_control_extenion`：浏览器扩展，用于发现网页媒体会话并把控制指令转发到桌面端。
- `E2C_Proxy`：Native Messaging / 本地代理相关实现与安装脚本。

这次更新重点完成了 `HxMonitor` 的 FluentUI 界面重构，包括新的性能总览面板、趋势曲线、明暗主题适配、调试面板按 `--debug` 显示，以及更稳定的自适应布局。

## HxMonitor 特性

- 基于 `Qt 6.9 + Widgets + QML + FluentUI`
- 展示 `CPU / GPU / Memory / Network` 分组性能卡片
- 为关键指标提供短时趋势折线
- 支持串口桥接，将监控数据同步到外部设备
- 通过本地 IPC / Native Messaging 与浏览器扩展通信
- 提供媒体会话列表和快捷控制
- 支持明暗主题切换
- `--debug` 模式下显示日志查看与 IPC 日志面板

## 本次界面更新

- 接入 `FluentUI`
- 新增 `qml/MainView.qml` 作为主界面
- 将性能信息重组为总览卡片 + 子组件卡片
- 优化信息层级、字重、颜色和暗色模式可读性
- 增加 CPU/GPU/内存/网络趋势曲线
- 修复窗口缩放时的卡片错位、曲线越界和列表重叠问题

## 目录结构

```text
HxController/
├─ HxMonitor/
│  ├─ widget.cpp / widget.h
│  ├─ main.cpp
│  ├─ qml/
│  │  └─ MainView.qml
│  ├─ qml.qrc
│  ├─ third_party/
│  │  └─ FluentUI/
│  ├─ build.md
│  └─ CLAUDE.md
├─ media_control_extenion/
├─ E2C_Proxy/
└─ README.md
```

## HxMonitor 构建

构建环境：

- Qt: `C:\Qt\6.9.0\mingw_64`
- MinGW: `C:\Qt\Tools\mingw1310_64`

常用命令：

```powershell
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
Set-Location .\HxMonitor\build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug
mingw32-make.exe release -j8
windeployqt.exe --qmldir ..\..\qml --qmlimport ..\..\third_party\FluentUI\dist .\release\HxMonitor.exe
```

正式程序默认输出目录：

```text
HxMonitor/build/Desktop_Qt_6_9_0_MinGW_64_bit-Debug/release/HxMonitor.exe
```

更详细的构建说明见：

- [HxMonitor/build.md](./HxMonitor/build.md)
- [HxMonitor/CLAUDE.md](./HxMonitor/CLAUDE.md)

## 运行方式

普通模式：

```powershell
.\HxMonitor.exe
```

调试模式：

```powershell
.\HxMonitor.exe --debug
```

调试模式下会额外显示日志相关界面，并在程序目录写入 `HxMonitor_debug.log`。

## 已知说明

- 若 `CmdMonitor.exe` 因提权失败未启动，部分硬件源数据可能受影响，但主界面仍可正常加载。
- 若重新编译时报 `Permission denied`，通常是旧的 `HxMonitor.exe` 进程还未退出。

## 仓库提交建议

提交 `HxMonitor` 相关改动时，建议重点检查以下文件：

- `HxMonitor/main.cpp`
- `HxMonitor/widget.cpp`
- `HxMonitor/widget.h`
- `HxMonitor/HxMonitor.pro`
- `HxMonitor/qml.qrc`
- `HxMonitor/qml/MainView.qml`
- `HxMonitor/third_party/FluentUI/dist/FluentUI/`

