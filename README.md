# CppGameServerFromScratch

入门 C++ 游戏服务器，笔记大部分都在代码注释中。

## 环境

Visual Studio 2019

C++ 控制台应用

Windows，以后会考虑跨平台。

## 运行

输出目录在 `根目录\bin` 中，根据生成平台来管理文件结构。

例如生成 TinyTCPServer 项目后，执行文件会生成在 `根目录\bin\Win32\Debug\TinyTCPServer.exe`，双击即可运行服务器在本机地址 4567 端口上。

## 服务器/客户端版本

服务器版本和客户端版本从 1.0 开始迭代，一步一步加功能，具体可以切换 branch 分支看看每个版本的实现。不同版本注重的知识点不同，笔记也会有所增删。

![](https://i.loli.net/2019/07/01/5d195b2fae3be96970.png)

master branch 维护最新版本的服务端、客户端和 [Changelog](https://github.com/Latias94/CppGameServerFromScratch/blob/master/HelloSocket/TinyTCPServer/README.md)。

## 项目结构

1. [HelloSocket](https://github.com/Latias94/CppGameServerFromScratch/blob/master/HelloSocket/HelloSocket)

   初尝 Winsock2 库

2. [TinyTCPServer](https://github.com/Latias94/CppGameServerFromScratch/blob/master/HelloSocket/TinyTCPServer)

   一个简单的 TCP 服务器，具体到 [TinyTCPServer/README.md](https://github.com/Latias94/CppGameServerFromScratch/blob/master/HelloSocket/TinyTCPServer/README.md) 查看笔记，版本的修改信息也在里面。

3. [TinyTCPClient](https://github.com/Latias94/CppGameServerFromScratch/blob/master/HelloSocket/TinyTCPClient)

   一个简单的 TCP 客户端

## 参考

* [《网络多人游戏架构与编程》](https://book.douban.com/subject/27135506/)—— [美] Joshua Glazer & Sanjay Madhav
* [《UNIX网络编程 卷1：套接字联网API（第3版）》](https://book.douban.com/subject/26434583/) ——[美] W. Richard Stevens & Bill Fenner & Andrew M. Rudoff
* 课程：C++百万并发网络通信引擎架构与实现