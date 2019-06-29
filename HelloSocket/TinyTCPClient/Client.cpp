#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);

	//-- 简易 TCP 客户端

	// 1. 建立一个 socket
	// 监听和接收连接的过程时不对称的。只有被动的服务器需要一个监听 socket。
	// 希望发起连接的客户端应该创建 socket，并使用 connect 函数开始与远程服务器的握手过程。 
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("Error: socket 建立套接字失败\n");
	}
	else
	{
		printf("socket 建立套接字成功\n");
	}
	// 2. 连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	// 服务器地址
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// 调用 connect 函数通过给目的主机发送初始 SYN 数据包来启动 TCP 握手
	// 如果目的主机（e.g. 服务器）有绑定到适当端口的监听 socket，目的主机将调用 accept 函数来继续握手过程。
	// 默认情况下，connect 函数将阻塞调用线程，直到连接被接受，或者超时。
	int ret = connect(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("Error: connect 连接套接字失败\n");
	}
	else
	{
		printf("connect 连接套接字成功\n");
	}

	// 3. 接收服务器信息

	// 如果 recv 函数调用成功，返回接受的数据大小，这个值小于等于参数 len。
	// 当 len 非零时，如果 recv 返回 0，说明连接的另外一端发送了一个 FIN 数据包，承诺没有更多需要发送的数据。
	// 当 len 为零时，如果 recv 返回 0，说明 socket 上有可以读的数据。
	// 当有很多 socket 在使用时，这是检查是否有数据到来而不需要占用单独缓冲区的一个简便方法。
	// 当 recv 函数已经表明有可用的数据时，你可以保留一个缓冲区，然后再次调用 recv 函数，输入这个缓冲区和非零的 len。
	// 默认情况下，如果 socket 的接收缓冲区中没有数据，recv 函数阻塞调用线程，直到数据流的下一组数据到达，或者超时。
	char recvBuf[256] = {};
	int nlen = recv(_sock, recvBuf, 256, 0);
	if (nlen > 0)
	{
		printf("接收到数据：%s", recvBuf);
	}

	// 4. 关闭套接字 socket
	closesocket(_sock);

	WSACleanup();
	// 防止命令行窗口执行完就关闭
	getchar();
	return 0;
}
