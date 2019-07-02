#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

#include <vector>

// 静态链接库
// #pragma comment(lib, "ws2_32.lib") // 只在 windows 有效
// 或者在 项目->属性->配置属性->链接器->输入->附加依赖项 中添加

// 网络数据报文的格式定义，需要和客户端保持一致

// 消息类型
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

// 包头：描述本次消息包的大小，描述数据的作用
struct DataHeader
{
	short dataLength;
	short cmd;
};

// 包体： 数据
struct Login :public DataHeader // 消息报结构直接继承包头结构，这样可以直接在构造函数直接初始化包头
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char password[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0; // 0 为登录错误，1 为登录成功
	}
	int result;
};

struct Logout :public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult :public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0; // 0 为登出错误，1 为登出成功
	}
	int result;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_NEW_USER_JOIN;
		socketId = 0;
	}
	int socketId;
};

std::vector<SOCKET> g_clients;

int handleSocket(SOCKET _recvSock)
{
	// 用缓冲区来接收数据 为了适应固长数据和变长数据。现在还不处理粘包、分包的问题
	char szRecz[4096] = {};
	// 5. 接收客户端数据，接收的 socket 是客户端提供的 socket
	// 先读取包头数据，判断消息的类型。
	int nLen = recv(_recvSock, szRecz, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecz;

	// 当 len 非零时，如果 recv 返回 0，说明连接的另外一端发送了一个 FIN 数据包，承诺没有更多需要发送的数据。
	if (nLen <= 0)
	{
		printf("Socket %d 客户端已退出。\n", (int)_recvSock);
		return -1;
	}
	// 6. 处理请求
	// 7. send 向客户端发送一条数据

	// 单客户端几乎不可能发生少包（消息报接收不全）的可能，在多客户端同时发送多个消息时再判断是否有少包的情况
	// if (nLen - sizeof(DataHeader)<=0)
	// {
	// 	  printf("少包，包头接收不全");
	// }
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		// 由于已经接收了包头，buffer 需要从包体开头读取，取包体数据。数据长度 len 应该是总数据包大小减去包头大小
		recv(_recvSock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		// 由于改用了缓冲区来储存返回数据，现在直接拿到缓冲区的指针来获得包体数据
		Login* login = (Login*)szRecz;

		printf("接收到 Socket %d 客户端的命令：CMD_LOGIN，数据长度：%d，userName=%s，password=%s\n", (int)_recvSock, login->dataLength, login->userName, login->password);

		// 暂时不判断密码对错
		LoginResult res;
		send(_recvSock, (char*)& res, sizeof(LoginResult), 0);

	}
	break;
	case CMD_LOGOUT:
	{
		recv(_recvSock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecz;

		printf("接收到 Socket %d 客户端的命令：CMD_LOGOUT，数据长度：%d，userName=%s\n", (int)_recvSock, logout->dataLength, logout->userName);
		LogoutResult res;
		send(_recvSock, (char*)& res, sizeof(LogoutResult), 0);

	}
	break;
	default:
	{
		DataHeader header = { 0,CMD_ERROR };
		send(_recvSock, (char*)& header, sizeof(DataHeader), 0);
	}

	break;
	}
	return 0;
}

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);

	//-- 简易 TCP 服务端

	// 1. 建立一个 socket 套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 2. 绑定用于接收客户端连接的端口

	sockaddr_in _sin = {};
	// 指定地址类型
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// INADDR_ANY 指 0.0.0.0 任意地址 // 或者指定一个地址：inet_addr("127.0.0.1");

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

	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("Error: listen 网络端口失败\n");
	}
	else
	{
		printf("listen 网络端口成功\n");
	}

	// 接收缓存区
	char _recvBuf[128] = {};

	// 不断接收连接
	while (true)
	{
		// 在 POSIX 平台，内核利用文件描述符（File Descriptor 文件句柄）来访问文件。fd_set，可以理解为一个集合，这个集合中存放的是文件描述符。
		// WinSock2.h 头文件将 fd_set 最大长度设置为 64，也就是说最多同时处理 64 个连接。以后有需要的话，需要修改这个限制。
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		// 要初始化一个空的 fd_set，首先在堆栈上声明，然后使用 FD_ZERO 宏将其赋值为 0。
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// 使用 FD_SET 宏给集合添加一个 socket。
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		// 1. select 函数接收客户端 socket -> g_client 暂存客户端 socket -> （目前在这一步）将接收的连接放入 fdRead -> select 函数 -> 客户端 socket 可读则处理请求
		// 如果之前客户端 socket 数组中已经有了值，说明已经接收到了客户端连接。
		// 于是将客户端 socket 放到 fdRead 中检查可读性，如果可读，再
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		// select 函数来进行 I/O 复用（I/O multiplexing），可以同时检查多个 socket ，只要其中有一个 socket 准备好了就开始执行操作。
		// 在 POSIX 平台，nfds 是待检查的编号最大的 socket 的标识符。在 POSIX 平台，每一个 socket 只是一个整数，所以可以直接将所有 socket 的最大值传入这个函数。
		// 在 Windows 平台，socket 表示为指针，而不是整数，所以这个参数不起作用，可以忽略。

		// readfds 是指向 socket 集合的指针，包含要检查可读性的 socket。其中 fds，即 fd_set。当数据包到达 readfds 集合中的 socket，select 函数尽快将控制返回给调用线程。
		// 首先，将所有还没有收到数据的 socket 从集合中移出，然后当 select 函数返回时，仍然在 readfds 集合中的 socket 保证不会被读操作阻塞。
		// 给 readfds 传入 nullptr 可以跳过任何 socket 的可读性检查。
		// writefds 与 readfds 类似，fd_set 存储的是待检查可写性的 socket。通常只有当 socket 的输出缓冲区有太多数据时，socket 才会阻塞写操作。
		// 当 select 函数返回时，仍然在 writefds 集合中的 socket 都保证可写。
		// exceptfds 是指向 fd_set 的指针，这个 fd_set 存储待检查错误的 socket。当 select 函数返回时保留在 exceptfds 中所有 socket 都已经发生了错误。
		// 传入 nullptr 到 readfds、writefds 和 exceptfds 中可以跳过相对应的检查。

		// timeout 是指向超时之前可以等待最长时间的指针。如果在 readfds 中的任意一个 socket 可读，writefds 中的任意一个 socket 可写，
		// 或者 exceptfds 中的任意一个 socket 发生错误之前发生超时，清空所有集合，select 函数将控制返回给调用线程。给 timeout 输入 nullptr 来表明没有超时限制。
		// timeout 这个参数有以下三种情况：
		// 1. 永远等待下去：仅在有一个描述符准备好 I/O 时才返回。为此，我们把该参数设置为空指针 nullptr。
		// 2. 等待一段固定时间：在有一个描述符准备好 I/O 时返回，但是不超过由该参数所指向的 timeval 结构中指定的秒数和微秒数。
		// 3. 根本不等待：检查描述符后立即返回，这称为轮询（polling）。为此，该参数必须指向一个 timeval 结构，而且其中的定时器值（由该结构指定的秒数和微秒数）必须为 0。
		timeval t = { 0,0 }; // 第三种，将网络模型设置为非阻塞
		// select 函数返回执行之后保留在 readfds、writefds 和 exceptfds 中 socket 的数量。如果发生超时，这个值是 0。
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0)
		{
			printf("select 任务结束。\n");
			break;
		}

		// 使用 FD_ISSET 宏检查在 select 函数返回之后，一个 socket 是否在集合中
		if (FD_ISSET(_sock, &fdRead))
		{
			// 在 fd_set 集合中删除一个文件描述符（这里是删除监听 socket） 
			FD_CLR(_sock, &fdRead);

			// 4. accept 等待接收客户端连接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _recvSock;

			// _sock 可读的话，说明有客户端连接。之后再将接收到的客户端 socket 在下一次循环中放到 fdRead 检查可读性，再进行处理请求。
			_recvSock = accept(_sock, (sockaddr*)& clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _recvSock)
			{
				printf("Error: accept 接收到无效 socket\n");
			}
			else
			{
				// 新客户端加入后，群发消息给其他客户端
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					send(g_clients[n], (char*)& userJoin, sizeof(NewUserJoin), 0);
				}

				printf("新客户端加入：Socket = %d, IP = %s \n", (int)_recvSock, inet_ntoa(clientAddr.sin_addr));
				// 用 vector 动态数组将客户端 socket 保存起来
				g_clients.push_back(_recvSock);
			}
		}

		// select 函数返回后，如果有客户端 socket 可读，则处理其 socket
		// 按照 fdRead 原始数据顺序处理
		// printf("可读的 socket 有 %d 个\n", (int)fdRead.fd_count);
		for (int n = 0; n < (int)fdRead.fd_count; n++)
		{
			if (-1 == handleSocket(fdRead.fd_array[n]))
			{
				// 处理 socket 出错，则移除 fdRead 中相应的客户端 socket
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}

	}

	// 如果 select 任务结束退出循环，则关闭所有 socket
	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}

	// 8. 关闭套接字 Socket
	closesocket(_sock);

	WSACleanup();

	printf("服务端已退出，任务结束。");
	getchar();
	return 0;
}