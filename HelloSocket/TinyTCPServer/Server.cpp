#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

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

	// 4. accept 等待接收客户端连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _recvSock;

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
		DataHeader header = {};

		// 5. 接收客户端数据，接收的 socket 是客户端提供的 socket
		// 先读取包头数据，判断消息的类型。
		int nLen = recv(_recvSock, (char*)& header, sizeof(DataHeader), 0);
		// 当 len 非零时，如果 recv 返回 0，说明连接的另外一端发送了一个 FIN 数据包，承诺没有更多需要发送的数据。
		if (nLen <= 0)
		{
			printf("客户端已退出，结束接收连接的循环。\n");
			break;
		}

		// 6. 处理请求
		// 7. send 向客户端发送一条数据

		// 单客户端几乎不可能发生少包（消息报接收不全）的可能，在多客户端同时发送多个消息时再判断是否有少包的情况
		// if (nLen - sizeof(DataHeader)<=0)
		// {
		// 	  printf("少包，包头接收不全");
		// }
		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			Login login = {};
			// 由于已经接收了包头，buffer 需要从包体开头读取，取包体数据。数据长度 len 应该是总数据包大小减去包头大小
			recv(_recvSock, (char*)& login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
			printf("接收到的命令：CMD_LOGIN，数据长度：%d，userName=%s，password=%s\n",login.dataLength, login.userName,login.password);

			// 暂时不判断密码对错
			LoginResult res;
			send(_recvSock, (char*)& res, sizeof(LoginResult), 0);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout logout = {};
			recv(_recvSock, (char*)& logout + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
			printf("接收到的命令：CMD_LOGOUT，数据长度：%d，userName=%s\n", logout.dataLength, logout.userName);
			LogoutResult res;
			send(_recvSock, (char*)& res, sizeof(LogoutResult), 0);

		}
		break;
		default:
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_recvSock, (char*)& header, sizeof(DataHeader), 0);
		}
	}

	// 8. 关闭套接字 Socket
	closesocket(_sock);

	WSACleanup();

	printf("服务端已退出，任务结束。");
	getchar();
	return 0;
}