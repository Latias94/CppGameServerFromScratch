# CppGameServerFromScratch

入门 C++ 游戏服务器，笔记大部分都在代码注释中。

## 环境

Visual Studio 2019

C++ 控制台应用

Windows，以后会考虑跨平台。

## 运行

输出目录在 `根目录\bin` 中，根据生成平台来管理文件结构。

例如生成 TinyTCPServer 项目后，执行文件会生成在 `根目录\bin\Win32\Debug\TinyTCPServer.exe`，双击即可运行服务器在本机地址 4567 端口上。

## 项目顺序

1. HelloSocket

   初尝 Winsock2 库

2. TinyTCPServer

   一个简单的只会返回信息的 TCP 服务器

3. TinyTCPClient

   一个简单的只会发送数据的 TCP 客户端

## 参考

* [《网络多人游戏架构与编程》](https://book.douban.com/subject/27135506/)—— [美] Joshua Glazer & Sanjay Madhav
* 课程：C++百万并发网络通信引擎架构与实现