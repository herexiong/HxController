# Proxy <-> 桌面端 (HxMonitor) 内部通信协议

## 1. 通信载体：Windows 命名管道 (Named Pipe)

- 通信方式：多客户端 (Proxy) 连接到同一个服务端 (HxMonitor)。
- 管道名称建议：`\\.\pipe\MediaControlHubPipe`。
- 传输格式：原生 UTF-8 JSON 字符串流（可选：为了安全起见可用简单的换行符 `\n` 分隔，或者直接沿用 4 字节表示长度的包头）。

## 2. 行为准则与异常处理

1. **启动与重连**
   - Proxy 被浏览器拉起后，立刻尝试连接 Named Pipe。
   - 如果连接失败（说明 HxMonitor GUI 没开），Proxy 可以选择：
     a) 继续运行，将来自浏览器的消息静默丢弃，同时保持每隔 3 秒尝试重连。
     b) 直接退出（不推荐，这会导致扩展报 NativeHost disconnected）。推荐做法是**静默等待 HxMonitor 启动**。
2. **连接成功**
   - 将之前队列（如果有的话）积压的状态发送给桌面端。
3. **桌面端处理 (HxMonitor Server)**
   - HxMonitor 作为 Server，需要维护一个活动连接的 Socket 列表。
   - 当用户在 GUI 端点击“下一曲”时，HxMonitor 把 `{"type":"cmd", "name":"Next"}` **广播(Broadcast)** 给所有连接着的 Proxy `Socket`。此时哪怕你有 3 个浏览器在跑，不符合条件的浏览器的 Content Script 也会自然抛弃该命令，不会产生冲突。

## 3. 消息透传策略

- 在这一层，Proxy **完全不需要理解 JSON 里面的具体字段（如 type, payload）**。
- 它只负责把浏览器过来的 JSON 字节序列，原封不动地发入管道；把管道过来的 JSON 序列，原封不动发入 `stdout`。
