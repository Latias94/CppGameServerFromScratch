#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <thread>

// 网络数据报文的格式定义，需要和服务器端保持一致

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

int handleSocket(SOCKET _sock)
{
	// 用缓冲区来接收数据 为了适应固长数据和变长数据。现在还不处理粘包、分包的问题
	char szRecz[4096] = {};
	// 先读取包头数据，判断消息的类型。
	int nLen = recv(_sock, szRecz, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecz;

	// 当 len 非零时，如果 recv 返回 0，说明连接的另外一端发送了一个 FIN 数据包，承诺没有更多需要发送的数据。
	if (nLen <= 0)
	{
		printf("与服务器断开连接，任务结束\n");
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		// 由于已经接收了包头，buffer 需要从包体开头读取，取包体数据。数据长度 len 应该是总数据包大小减去包头大小
		recv(_sock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginResult = (LoginResult*)szRecz;
		printf("接收到服务器的消息：CMD_LOGIN_RESULT，数据长度：%d\n", loginResult->dataLength);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_sock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logoutResult = (LogoutResult*)szRecz;
		printf("接收到服务器的消息：CMD_LOGOUT_RESULT，数据长度：%d\n", logoutResult->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_sock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* joinResult = (NewUserJoin*)szRecz;
		printf("接收到服务器的消息：CMD_NEW_USER_JOIN，数据长度：%d\n", joinResult->dataLength);
	}
	break;
	}
	return 0;
}


bool g_bRun = true;

// 把输入命令的逻辑放到子线程中，这样的话需要 g_bRun 全局变量来判断程序是否要退出
void cmdThread(SOCKET _sock)
{
	while (true)
	{
		char cmdBuf[128] = {};
		// scanf 是阻塞函数，需要在其他线程中使用，否则会阻塞主循环
		scanf("%s", cmdBuf);

		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("退出程序\n");
			g_bRun = false;
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "admin");
			strcpy(login.password, "pwd");

			send(_sock, (char*)& login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "admin");	

			send(_sock, (char*)& logout, sizeof(Logout), 0);
		}
		else
		{
			printf("不支持的命令。\n");
		}
	}
}

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);

	//-- 简易 TCP 客户端

	// 1. 建立一个 socket
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
	int ret = connect(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("Error: connect 连接套接字失败\n");
	}
	else
	{
		printf("connect 连接套接字成功\n");
	}

	// 启动线程，第二个参数能给函数传参
	std::thread t1(cmdThread, _sock);
	// 让子线程与主线程分离，否则主线程退出但子线程仍然 attach 主线程会出错。
	t1.detach();

	while (g_bRun)
	{
		// 转用 select 网络模型
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		// 定义了 timeval 的作用：等待一段固定时间。在有一个描述符准备好 I/O 时返回，但是不超过由该参数所指向的 timeval 结构中指定的秒数和微秒数。
		// 也就是说与赋值 0 相比，这样不会再阻塞客户端，可以 select 的同时处理其他事情。
		timeval t = { 1,0 };
		int ret = select(_sock, &fdRead, 0, 0, &t);
		if (ret < 0)
		{
			printf("select 任务结束。\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if (-1 == handleSocket(_sock))
			{
				printf("select 任务结束，与服务器断开连接。\n");
				break;
			}
		}


		// Sleep(1000); // 只适用于 Windows
	}

	// 7. 关闭套接字 socket
	closesocket(_sock);

	WSACleanup();
	printf("客户端已退出，任务结束。");
	// 防止命令行窗口执行完就关闭
	getchar();
	return 0;
}
