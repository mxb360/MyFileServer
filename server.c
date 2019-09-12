/* 华清远见项目实训 - 文件服务器
 *
 * server.c  服务器部分源码
 * 提供文件服务器的服务器端部分的实现
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#include "socket.h"
#include "tools.h"
#include "config.h"
#include "server_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 服务器启动检查 */
void server_check(void)
{
    if (file_check(SERVER_FILE_PATH) != -1)
        system("mkdir " SERVER_FILE_PATH);
    if (file_check(USER_INFO_PATH) != -1)
        system("mkdir " USER_INFO_PATH);
    printf("用户文件目录：%s\n", SERVER_FILE_PATH);
    printf("用户信息目录：%s\n", USER_INFO_PATH);

}

/* 处理连接到的客户端 */
void resolve(Socket server, Socket client)
{
    MyFileServer file_server;

    printf("连接到客户端 [%s, %d]\n", client->ipstr, client->port);
    file_server = MyFileServer_Create(server, client);
    Socket_SendString(client, "Welcome to MyFileServer!");

    while (1) {
         /* 处理来自客户端的命令 */
         MyFileServer_ResolveCmd(file_server);
         /* 判断用户是否下线 */
         if (file_server->logout) {
             printf("用户[%s, %d]已下线。\n", file_server->client->ipstr, file_server->client->port);
             Socket_Delete(file_server->client);
             free(file_server);
             break;
         }
    }
}

int main(int argc, char *argv[])
{
    Socket server;

    printf("华清远见项目实训 - 文件服务器V1.0 服务器部分\n");
    printf("作者： Ma Xiaobo\n日期：2018-11\n\n");
    printf("服务器端正在启动 ...\n");

    /* 创建服务器 */
    server = Socket_CreateServer(SOCKET_SERVER, SERVER_PORT, SERVER_LISTEN);
    if (server == NULL) {
        printf("服务器启动失败\n");
        return 1;
    }

    printf("服务器启动成功\n[外部IP: %s 端口： %d]\n等待客户端的连接 ...\n\n", SERVER_IP, SERVER_PORT);
    server_check();
    printf("Ctrl+C退出服务器。\n\n");

    /* 等待客户端的连接 */
    while (1) {
        Socket client = Socket_ServerAccept(server);
        /* 如果有连接，开一个线程处理该客户端的连接 */
        if (client && !fork())
            resolve(server, client);
    }

    return 0;
}
