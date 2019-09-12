/* 华清远见项目实训 - 文件服务器
 *
 * client_api.h 文件服务器客户端外部接口原型
 * 提供文件服务器的客户端外部接口原型
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#ifndef _MY_CLIENT_API_H
#define _MY_CLIENT_API_H

#include "socket.h"
#include "config.h"
#include "tools.h"

/* 文件服务器客户端的封装 */
typedef struct _MyFileClient {
    Socket client;                      /* 客户端套接字 */

    char user_name[MAX_USER_NAME];      /* 客户端用户名 */
    char passwd[MAX_PASSWD];            /* 客户端密码 */

    int is_login;                       /* 用户是否处于登录状态 */

    char current_path[SERVER_MAX_PATH]; /*服务器工作的当前路径（相对路径）*/
} *MyFileClient;

MyFileClient MyFileClient_Create(Socket client, const char *user_name, const char *passwd);
int MyFileClient_Login(MyFileClient file_client);
int MyFileClient_UpLoadFile(MyFileClient file_client, const char *file1, const char *file2);
int MyFileClient_DownLoadFile(MyFileClient file_client, const char *file1, const char *file2);
int MyFileClient_Chdir(MyFileClient file_client, const char *path);
int MyFileClient_GetDir(MyFileClient file_client);
int MyFileClient_GetDirFiles(MyFileClient file_client, char *filebuf[]);
int MyFileClient_Mkdir(MyFileClient file_client, const char *path);
int MyFileClient_RM(MyFileClient file_client, const char *path);
int MyFileClient_MV(MyFileClient file_client, const char *path1, const  char *path2);

#endif
