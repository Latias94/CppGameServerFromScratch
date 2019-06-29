#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>

// 静态链接库
// #pragma comment(lib, "ws2_32.lib") // 只在 windows 有效
// 或者在 项目->属性->配置属性->链接器->输入->附加依赖项 中添加

int main()
{
    // 在 POSIX 平台，socket 库在默认下就是激活状态，不需要特意启动 socket 功能。
    // 但是 Winsock2 需要显式地启动和关闭，并允许用户指定使用什么版本
    // WORD 两个字节，低字节表示主版本号，高字节表示所需要的 Winsock 实现的最低版本
    WORD ver = MAKEWORD(2, 2);
	// lpWSAData 指向 Windows 特定的数据结构
    WSADATA data;
	// 返回值 成功为0，或者错误代码
	WSAStartup(ver, &data);

	// 关闭库需要调用 Cleanup，返回的是错误代码。
	// 当一个进程调用 WSACleanup 时，会结束所有未完成的 socket 操作，释放所有 socket 资源。
	// 所以关闭 socket 之前，最好确保所有 socket 都已经关闭并且没有在使用。
	// WSAStartup 时引用计数的，所以调用 WSACleanup 的次数与调用 WSAStartup 的次数必须一致，保证清理了一切东西。
	WSACleanup();
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单