# 1. Socket 基础 API

![](https://i.loli.net/2019/06/30/5d18097310d1e94392.png)

## 使用

生成项目后，双击 `根目录\bin\Win32\Debug\TinyTCPServer.exe`启动服务器，客户端在 `根目录\bin\Win32\Debug\TinyTCPClient.exe`，启动两个客户端效果如下：

![](https://i.loli.net/2019/06/30/5d180a159178922393.png)

项目请参考 branch ：[01-server1.0-client1.0](https://github.com/Latias94/CppGameServerFromScratch/tree/01-server1.0-client1.0)

# 2. 建立能持续处理请求的 C/S 网络程序

![](https://i.loli.net/2019/06/30/5d180ac4bedfb83305.png)

前面一个版本服务器只能不断发送请求。新版本客户端能向服务端发送请求，服务器也能不断地处理第一个连接的客户端的请求。

![](https://i.loli.net/2019/06/30/5d1816ca39a6852989.png)

项目请参考 branch：[02-server1.1-client1.1](https://github.com/Latias94/CppGameServerFromScratch/tree/02-server1.1-client1.1)

# 3. 发送结构化的网络消息数据

![](https://i.loli.net/2019/07/02/5d1b02760ebaa88358.png)

前一个版本服务器只能发送字符串，这个版本使用结构体来封装数据。

![](https://i.loli.net/2019/07/01/5d19541bd4e9045671.png)

为了实现多人游戏网络实体之间的对象传输，游戏必须给这些对象规定数据格式，这样它们才能通过传输层协议发送。

序列化是一种将对象从内存中的随机访问格式转换为比特流格式的行为，这些比特流可以在硬盘上存储，或者通过网络传输。

起初这个版本服务器和客户端把报文分成包头（消息类型、消息包大小）和包体（数据），一次请求用两次 `send ` 函数分别发送包头和包体，接收请求也要分成两次，这样增加了出错的机率。后来把包头和包体结合成一个结构，这样一开始读取包头判断消息类型后，`recv ` 函数要注意接收的消息 buffer 初始读取位置以及读取的包体大小。

项目请参考 branch：[03-server1.2-client1.2](https://github.com/Latias94/CppGameServerFromScratch/tree/03-server1.2-client1.2)

# 4. 服务端升级为 select 模型处理多客户端

![](https://i.loli.net/2019/07/01/5d19b897f237976923.png)

![](https://i.loli.net/2019/07/02/5d1ad18dc881255704.png)

用了 Select 后可以同时处理多个客户端，且支持跨平台。

项目请参考 branch：[04-server1.3-client1.2](https://github.com/Latias94/CppGameServerFromScratch/tree/04-server1.3-client1.2)

## I/O 模型

Unix下可用的 5 种 I/O 模型：

* 阻塞式 I/O；
* 非阻塞式 I/O；
* I/O 复用（select 和 poll）；
* 信号驱动式 I/O（SIGIO）；
* 异步 I/O（POSIX 的 aio_ 系列函数）。

下文只介绍前三种模型。

### 阻塞式 I/O 模型

> 首先，要从你常用的 IO 操作谈起，比如 read 和 write，通常 IO 操作都是**阻塞 I/O** 的，也就是说当你调用 read 时，如果没有数据收到，那么线程或者进程就会被挂起，直到收到数据。
>
> [I/O多路复用技术（multiplexing）是什么？ - 用心阁的回答 - 知乎](https://www.zhihu.com/question/28594409/answer/74003996)

![](https://i.loli.net/2019/07/01/5d19c2788c28c67037.png)

### 非阻塞式 I/O 模型

进程把一个套接字设置成非阻塞是在通知内核：当所请求的 I/O 操作非得把本进程投入睡眠才能完成时，不要把本进程投入睡眠，而是返回一个错误。

![](https://i.loli.net/2019/07/02/5d1afc61d8b4996041.png)

前三次调用 `recvfrom` 时没有数据可返回，因此内核转而立即返回一个`EWOULDBLOCK` 错误。
第四次调用 `recvfrom` 时已有一个数据报准备好，它被复制到应用进程缓冲区，于是 `recvfrom` 成功返回。我们接着处理数据。
当一个应用进程像这样对一个非阻塞描述符循环调用 `recvfrom` 时，我们称之为轮询（polling）。应用进程持续轮询内核，以查看某个操作是否就绪。这么做往往耗费大量CPU时间，不过这种模型偶尔也会遇到，通常是在专门提供某一种功能的系统中才有。

## I/O 复用模型

有了 I/O 复用（I/O multiplexing，也称 I/O 多路复用），我们就可以调用 `select` 或 `poll`，阻塞在这两个系统调用中的某一个之上，而不是阻塞在真正的 I/O 系统调用上。

![](https://i.loli.net/2019/07/01/5d19c51713f8c17499.png)

进程阻塞于 `select` 调用，等待数据报套接字变为可读。当 `select` 返回“套接字可读”这一条件时，我们调用`recvfrom` 把所读数据报复制到应用进程缓冲区。

### Select

socket 库提供了同时检查多个 socket 的方式，只要其中有一个 socket 准备好了就开始执行，我们可以使用 `select` 函数实现这个操作。

具体参数解释参考 `Server.cpp` 的注释

## 参考：

[《UNIX网络编程 卷1：套接字联网API（第3版）》](https://book.douban.com/subject/26434583/) 第六章

# 5. 客户端也升级为 select 模型

![](https://i.loli.net/2019/07/02/5d1b0068dced498611.png)

除了给客户端像服务端一样的改用 select 模型之外，由于 `scanf` 函数是会阻塞线程，我们需要给命令输入多创建一个子线程，这样不会阻塞主循环的网络请求。

项目请参考 branch：[05-server1.3-client1.3](https://github.com/Latias94/CppGameServerFromScratch/tree/05-server1.3-client1.3)