# E2C Proxy 接入指南 (Integration Guide)

既然 E2C Proxy (中间件) 已经就绪，接下来需要在“浏览器扩展”和“桌面端 HxMonitor”两端分别进行对接。

## 1. 浏览器扩展端 (Edge Extension) 对接

浏览器扩展通过 Native Messaging API 启动 Proxy，并通过标准的 `Port` 对象进行收发。

### 1.1 确认 Manifest 权限
在你的 **Edge 扩展项目**的 `manifest.json` 中，确保声明了 `nativeMessaging` 权限：
```json
{
  "permissions": [
    "nativeMessaging"
  ]
}
```

### 1.2 在后台脚本 (Background/Service Worker) 中连接
在扩展后台脚本中，调用 `connectNative` 并传入我们在注册表中写入的 Host 名字 `com.hxmonitor.proxy`：

```javascript
// 1. 发起连接，会自动在系统后台静默拉起 e2c_proxy.exe
const port = chrome.runtime.connectNative("com.hxmonitor.proxy");

// 2. 监听来自 HxMonitor (桌面端) 的消息
port.onMessage.addListener((msg) => {
    console.log("收到桌面端指令:", msg);
    // 例如收到 {"type":"cmd", "name":"Next"}
    // 在这里执行控制网页音乐下一曲的逻辑
});

// 3. 监听连接断开 (排错用)
port.onDisconnect.addListener(() => {
    console.log("与 Proxy 的连接断开:", chrome.runtime.lastError);
});

// 4. 主动发送状态给 HxMonitor (桌面端)
// 可以把当前网页播放器的状态发给桌面端
function sendStatusToDesktop(state) {
    port.postMessage({
        event: "status",
        state: state // 如 "playing", "paused"
    });
}
```
**注意：** 您刚才在 Proxy 的 `manifest.json` 中填写的 `chrome-extension:/nbmk...` 少了一个斜杠，我已经帮您修正为 `chrome-extension://nbmkfkfceagckncedpdhpfjdfoffageb/`。修改后请务必双击运行一次 `install_edge.bat` 重新注册。

---

## 2. 桌面客户端 (HxMonitor) 对接

HxMonitor 桌面程序需要作为 **Named Pipe Server (命名管道服务端)** 运行，等待 Proxy 客户端的接入。

管道名称固定为：`\\.\pipe\MediaControlHubPipe`

开发 HxMonitor 时（假设您使用 C#、C++、Python 或 Go 开发 GUI），需要监听这个管道。

### 示例 1: C# / .NET 接入 (推荐 GUI 常用语言)
如果您使用 C# 开发 HxMonitor，可以使用 `NamedPipeServerStream`：

```csharp
using System;
using System.IO;
using System.IO.Pipes;
using System.Text;
using System.Threading.Tasks;

class HxMonitorServer
{
    static async Task Main()
    {
        Console.WriteLine("HxMonitor 正在启动管道服务...");
        // 创建允许双向通信的 Named Pipe
        using var pipeServer = new NamedPipeServerStream("MediaControlHubPipe", PipeDirection.InOut, 10, PipeTransmissionMode.Byte, PipeOptions.Asynchronous);
        
        Console.WriteLine("等待浏览器 Proxy 连接...");
        await pipeServer.WaitForConnectionAsync();
        Console.WriteLine("浏览器 Proxy 已连接！");

        using var reader = new StreamReader(pipeServer, Encoding.UTF8);
        using var writer = new StreamWriter(pipeServer, Encoding.UTF8) { AutoFlush = true };

        // 发送控制指令给浏览器
        // 注意：由于 Proxy 代码中以换行符 '\n' 读取，所以在发送 JSON 字符串后必须加上 \n
        await writer.WriteAsync("{\"type\":\"cmd\", \"name\":\"Next\"}\n");

        // 读取浏览器发来的播放状态
        while (pipeServer.IsConnected)
        {
            /// 以换行符为界读取 Json 数据
            string message = await reader.ReadLineAsync();
            if (message != null)
            {
                Console.WriteLine($"收到浏览器状态: {message}");
            }
        }
    }
}
```

### 示例 2: Go 语言接入
如果您的 HxMonitor 也是用 Go 写的，可以使用我们测试用过的 `npipe.v2` 库：
```go
package main

import (
	"bufio"
	"fmt"
	"gopkg.in/natefinch/npipe.v2"
)

func main() {
	ln, _ := npipe.Listen(`\\.\pipe\MediaControlHubPipe`)
	defer ln.Close()

	for {
		conn, _ := ln.Accept()
		fmt.Println("浏览器 Proxy 已连接")
		
		go func(c net.Conn) {
			defer c.Close()
			// 发送指令给浏览器 (必须带 \n 换行符)
			c.Write([]byte("{\"type\":\"cmd\", \"name\":\"PlayPause\"}\n"))

			// 接收浏览器的回复
			scanner := bufio.NewScanner(c)
			for scanner.Scan() {
				fmt.Println("收到浏览器信息:", scanner.Text())
			}
		}(conn)
	}
}
```

### 核心机制总结
- **启动顺序无关**：你可以先开 Edge 浏览器，也可以先开 HxMonitor 桌面端。如果先开浏览器，Proxy 代理会在后台静默每 3 秒尝试寻找 HxMonitor；一旦 HxMonitor 开启管道，它们就会立刻连上。
- **数据格式约定**：桌面端只要往管道里丢 **带 `\n` 结尾的纯 JSON 字符串**，Proxy 就会自动帮你转换成 4字节前缀标准并塞进宿主进程，再流转给扩展。反之亦然。
