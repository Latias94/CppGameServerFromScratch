#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

// 网络数据报文的格式定义，需要和服务器端保持一致

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
struct Login :public DataHeader
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

	while (true)
	{
		// 3. 输入请求命令
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);

		// 4. 处理请求
		// 如果输入 exit 则退出客户端程序
		if (0 == strcmp(cmdBuf, "exit"))
		{
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			// 5. 向服务端发送请求
			Login login;
			strcpy(login.userName, "admin");
			strcpy(login.password, "pwd");
			
			send(_sock, (char*)& login, sizeof(Login), 0);
			// 接收服务器返回的数据
			LoginResult res = {};
			recv(_sock, (char*)& res, sizeof(LoginResult), 0);
			printf("LoginResult: %d\n", res.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "admin");
			send(_sock, (char*)& logout, sizeof(Logout), 0);
			// 接收服务器返回的数据
			LoginResult res = {};
			recv(_sock, (char*)& res, sizeof(LogoutResult), 0);
			printf("LogoutResult: %d\n", res.result);
		}
		else
		{
			printf("不支持的命令，请重新输入。\n");
		}
	}

	// 7. 关闭套接字 socket
	closesocket(_sock);

	WSACleanup();
	printf("客户端已退出，任务结束。");
	// 防止命令行窗口执行完就关闭
	getchar();
	return 0;
}
