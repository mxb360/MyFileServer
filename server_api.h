/* 华清远见项目实训 - 文件服务器
 *
 * server_api.h 文件服务器服务器端外部接口的原型
 * 提供文件服务器服务器端外部接口的原型
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#ifndef _MY_SERVER_API_H
#define _MY_SERVER_API_H

#include "socket.h"
#include "config.h"
#include "tools.h"

/* 文件服务器服务器端的封装 */
typedef struct _MyFileServer {
    Socket server;    /* 服务器套接字 */
    Socket client;    /* 客户端套接字 */

    int is_login;     /* 用户是否处于登录状态 */
    int logout;       /* 用户是否已登出，登出则中断连接 */

    char user_name[MAX_USER_NAME];  /* 客户端用户名 */
    char passwd[MAX_PASSWD];        /* 客户端密码 */

    char current_path[SERVER_MAX_PATH];  /* 服务器工作的当前路径（绝对路径） */
    char *pwd;                           /*服务器工作的当前路径（相对路径）*/
} *MyFileServer;

char *MyFileServer_GetUserPasswd(const char *user_name, char *buf);
int MyFileServer_PasswdCheck(const char *user_name, const char *passwd);
int MyFileServer_Login(MyFileServer file_server);
int MyFileServer_ResolveCmd(MyFileServer file_server);
MyFileServer MyFileServer_Create(Socket server, Socket client);
int MyFileServer_Chdir(MyFileServer file_server, const char *path);
int MyFileServer_GetDir(MyFileServer file_server, char *buf);

#endif
