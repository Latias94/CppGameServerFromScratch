# 1. Socket 基础 API

![](https://i.loli.net/2019/06/30/5d18097310d1e94392.png)

## 使用

生成项目后，双击 `根目录\bin\Win32\Debug\TinyTCPServer.exe`启动服务器，客户端在 `根目录\bin\Win32\Debug\TinyTCPClient.exe`，启动两个客户端效果如下：

![](https://i.loli.net/2019/06/30/5d180a159178922393.png)

文件可以参考 commit 版本

服务端 [Server.cpp](https://github.com/Latias94/CppGameServerFromScratch/blob/93cbb090dfd240e9cc517c84c04db900b8b0b0eb/HelloSocket/TinyTCPServer/Server.cpp)

客户端 [Client.cpp](https://github.com/Latias94/CppGameServerFromScratch/blob/93cbb090dfd240e9cc517c84c04db900b8b0b0eb/HelloSocket/TinyTCPClient/Client.cpp)

# 2. 建立能持续处理请求的 C/S 网络程序

![](https://i.loli.net/2019/06/30/5d180ac4bedfb83305.png)

前面一个版本服务器只能不断发送请求。新版本客户端能向服务端发送请求，服务器也能不断地处理第一个连接的客户端的请求。

![](https://i.loli.net/2019/06/30/5d1816ca39a6852989.png)