#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

// 静态链接库
// #pragma comment(lib, "ws2_32.lib") // 只在 windows 有效
// 或者在 项目->属性->配置属性->链接器->输入->附加依赖项 中添加

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);

	//-- 简易 TCP 服务端
	// 在伯克利套接字 API 中，Socket 本身存储连接状态，意思时主机针对每个保持的 TCP 连接，都需要一个额外的、单独的 socket。
	// TCP 需要三次握手启动客户端和服务器之间的连接
	// 服务器要接收这三次握手中初始阶段的数据包，必须首先创建一个 socket，绑定到指定的端口，然后才能监听传入的连接请求。
	// 使用 socket 和 bind 函数创建和绑定一个 socket 后，才可以使用 listen 函数启动监听。

	// 1. 建立一个 socket 套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 2. 绑定用于接收客户端连接的端口

	// 每一个网络层数据包都需要一个源地址和一个目的地址，我们可以使用 sockaddr 创建地址信息
	// 由于 sockaddr 用 sa_data[14] 字节数组来保存地址格式，这样不利于我们手动去填写，因为我们需要知道各种地址族的地址格式
	// 所以我们需要用 socketaddr_in 来创建一个数据包的地址，这个类型方便自定义地址
	sockaddr_in _sin = {};
	// 指定地址类型
	_sin.sin_family = AF_INET;

	// 当利用 4 字节整数(例如下面的 4567)设置 IP 地址或者设置端口号时，很重要的一件事是考虑 TCP/IP 协议族和主机有可能在多字节数的字节序上采用不同的标准。
	// 字节序是指整数在内存中保存的顺序，你可能有听过大端字节序和小端字节序
	// 现在我们知道 socket 地址结构体中的多字节数赋值必须将主机字节序转换成网络字节序就足够了
	// 为了实现这个功能，socket API 提供了 htons 函数和 htonl 函数。h 表示 host，n 表示 network，l 表示32位长整数，s 表示16位短整数。
	_sin.sin_port = htons(4567);

	// sin_addr 存储 4 字节的 IP 地址。in_addr 类型在不同的 socket 库之间有差异。
	// 在一些平台上，它是简单的 4 字节整数。IPv4 地址通常不写成 4 字节的整数，而是由英文句号分隔的 4 个单独的字节。
	// 出于这个原因，一些平台提供一个结构体来封装这个结构，用于设置不同格式的地址，也就是 sin_addr 的类型 IN_ADDR。 
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// INADDR_ANY 指 0.0.0.0 任意地址 // 或者指定一个地址：inet_addr("127.0.0.1");

	// 通知操作系统 socket 将使用一个特定地址和传输层接口的过程称为：绑定
	// 我们需要通过一个特定地址的绑定允许你确定 socket 应该使用哪个接口
	// 当主机作为路由器或网络之间的桥梁时，这十分有用，因为不同的接口有可能连接完全不同的计算机
	// 对于多人游戏，指定网络接口没那么重要，事实上，通常需要为所有可用的网络接口和主机的所有 IP 地址绑定端口，为此可以把 S_addr 赋值为 INADDR_ANY。

	// 将编辑好的 socketaddr_in 强转回 sockaddr 类型
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in)))
	{
		printf("Error: bind 网络端口失败\n");
	}
	else
	{
		printf("bind 网络端口成功\n");
	}

	// 3. listen 监听网络端口

	// sock 是设置为监听模式的 socket。监听模式的 socket 每收到 TCP 握手的第一阶段数据包时，存储这个请求，直到相应的进程调用函数来接受这个连接，并继续握手操作。
	// backlog 是队列中允许传入的最大连接数。一旦队列中的传入连接数量达到最大值，任何后续的连接都将被丢弃。输入 SOMAXCONN 表示使用默认的 backlog 值。
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("Error: listen 网络端口失败\n");
	}
	else
	{
		printf("listen 网络端口成功\n");
	}

	// 4. accept 等待接收客户端连接
	// 如果 accept 函数执行成功，将创建并返回一个可以与远程主机通信的新 socket。这个新 socket 被绑定到与监听 socket 相同的端口号上.
	// 当操作系统收到一个目的端口是该绑定端口的数据包时，它使用源地址和源端口来确定哪个 socket 应该接收这个数据包
	// TCP 要求每台主机针对每个保持的 TCP 连接都需要单独的 socket
	// 默认情况下，如果没有待接收的传入连接，accept 函数将阻塞调用线程，直到收到一个传入的连接，或者超时。
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _recvSock;

	// addr 是指向 sockaddr 结构体的指针，它不能控制接收哪个连接，能做的只是存储请求连接的远程主机的地址。
	// addrlen 是指向 addr 缓冲区大小的指针，以字节为单位。当真正写入地址之后，accept 函数将更新这个参数。
	_recvSock = accept(_sock, (sockaddr*)& clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _recvSock)
	{
		printf("Error: accept 接收到无效 socket\n");
	}
	printf("新客户端加入：Socket = %d, IP = %s \n", (int)_recvSock, inet_ntoa(clientAddr.sin_addr));

	// 接收缓存区
	char _recvBuf[128] = {};

	// 不断接收连接
	while (true)
	{
		// 5. 接收客户端数据，接收的 socket 是客户端提供的 socket
		int nLen = recv(_recvSock, _recvBuf, 128, 0);
		// 当 len 非零时，如果 recv 返回 0，说明连接的另外一端发送了一个 FIN 数据包，承诺没有更多需要发送的数据。
		if (nLen <= 0)
		{
			printf("客户端已退出，结束接收连接的循环。\n");
			break;
		}
		printf("接收到：%s\n", _recvBuf);

		// 6. 处理请求
		// strcmp 函数比较两个字符串是否相等，相等返回 0
		if (0 == strcmp(_recvBuf, "getName"))
		{
			char msgBuf[] = "Frankorz";
			send(_recvSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else if (0 == strcmp(_recvBuf, "getAge"))
		{
			char msgBuf[] = "25";
			send(_recvSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else
		{
			// 7. send 向客户端发送一条数据

			// 连接的 TCP socket 存储远程主机的地址信息。因此，进程不需要为传输数据的函数传入地址参数，可以直接用 send 函数通过 socket 发送数据。
			// 第二个参数 buf 是写入缓冲区。将数据放到输出缓冲区中，socket 库来决定在将来某一时间发送出去。
			// len 是传输的字节数量。只要 socket 的输出缓冲区有空间，网络库就可以将数据放到缓冲区中，然后等到缓冲区数据块大小合适时再发送出去。+ 1 包括结尾符。
			// flags 是对控制数据发送标志进行按位或运算的结果。大多数游戏代码中，该参数取值为 0。
			// 如果没有判断返回值，数据有可能没有完全发送。
			char msgBuf[] = "Hello, I'm Server";
			send(_recvSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
	}

	// 8. 关闭套接字 Socket
	closesocket(_sock);

	WSACleanup();

	printf("服务端已退出，任务结束。");
	getchar();
	return 0;
}