#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESPONSE_BUFFER_SIZE 20 * 1024 * 1024 // Http响应时的文件缓冲区最大大小: 20MB

// HTTP服务句柄结构定义
typedef struct {
    char rootPath[MAX_PATH];
    unsigned short port;
    SOCKET listenSocket;
    volatile BOOL isRunning;
} MyHttpServer_t, *MyHttpServer;


/**
 * @brief 创建并初始化HTTP服务
 * @param rootPath 静态文件根目录
 * @param port 监听端口
 * @return 成功返回HTTP服务句柄，失败返回NULL
 */
MyHttpServer CreateMyHttpServer(const char* rootPath, int port);


/**
 * @brief 启动HTTP服务的监听循环
 * @param server CreateMyHttpServer返回的HTTP服务句柄
 */
DWORD WINAPI StartMyHttpServer(MyHttpServer server);

/**
 * @brief 关闭HTTP服务，释放资源
 * @param server CreateMyHttpServer返回的HTTP服务句柄
 */
void CloseMyHttpServer(MyHttpServer server);