# 1. Socket 基础 API

![](https://i.loli.net/2019/06/30/5d18097310d1e94392.png)

## 使用

生成项目后，双击 `根目录\bin\Win32\Debug\TinyTCPServer.exe`启动服务器，客户端在 `根目录\bin\Win32\Debug\TinyTCPClient.exe`，启动两个客户端效果如下：

![](https://i.loli.net/2019/06/30/5d180a159178922393.png)

其他版本文件可以参考 branch 版本

服务端 [Server.cpp](https://github.com/Latias94/CppGameServerFromScratch/blob/93cbb090dfd240e9cc517c84c04db900b8b0b0eb/HelloSocket/TinyTCPServer/Server.cpp)

客户端 [Client.cpp](https://github.com/Latias94/CppGameServerFromScratch/blob/93cbb090dfd240e9cc517c84c04db900b8b0b0eb/HelloSocket/TinyTCPClient/Client.cpp)

# 2. 建立能持续处理请求的 C/S 网络程序

![](https://i.loli.net/2019/06/30/5d180ac4bedfb83305.png)

前面一个版本服务器只能不断发送请求。新版本客户端能向服务端发送请求，服务器也能不断地处理第一个连接的客户端的请求。

![](https://i.loli.net/2019/06/30/5d1816ca39a6852989.png)

其他版本文件可以参考 branch 版本

# 3. 发送结构化的网络消息数据

![](C:\Users\narut\AppData\Roaming\Typora\typora-user-images\1561871313076.png)

前一个版本服务器只能发送字符串，这个版本使用结构体来封装数据。

![](https://i.loli.net/2019/07/01/5d19541bd4e9045671.png)

为了实现多人游戏网络实体之间的对象传输，游戏必须给这些对象规定数据格式，这样它们才能通过传输层协议发送。

序列化是一种将对象从内存中的随机访问格式转换为比特流格式的行为，这些比特流可以在硬盘上存储，或者通过网络传输。

起初这个版本服务器和客户端把报文分成包头（消息类型、消息包大小）和包体（数据），一次请求用两次 `send ` 函数分别发送包头和包体，接收请求也要分成两次，这样增加了出错的机率。后来把包头和包体结合成一个结构，这样一开始读取包头判断消息类型后，`recv ` 函数要注意接收的消息 buffer 初始读取位置以及读取的包体大小。