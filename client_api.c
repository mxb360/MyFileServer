/* 华清远见项目实训 - 文件服务器
 *
 * client_api.c 文件服务器客户端外部接口的实现
 * 提供文件服务器客户端外部接口的实现
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#include "client_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char _cmd_buf[RECV_BUF_SIZE];

/* 创建一个文件服务器的客户端
 * client: 客户端的Socket对象
 * user_name: 用户名
 * passwd: 密码
 * 返回：成功创建返回客户端指针，失败返回NULL
 */
MyFileClient MyFileClient_Create(Socket client, const char *user_name, const char *passwd)
{
    MyFileClient file_client = (MyFileClient)malloc(sizeof(struct _MyFileClient));
    if (file_client) {
        file_client->client = client;
        file_client->is_login = 0;
        strcpy(file_client->user_name, user_name);
        strcpy(file_client->passwd, passwd);
        strcpy(file_client->current_path, "/");
    }

    return file_client;
}

/* 客户端登录到文件服务器
 * file_client: 客户端指针
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_Login(MyFileClient file_client)
{
    /* 发送登录请求 */
    Socket_SendString(file_client->client, CMD_LOGIN);
    Socket_SendDelay();
    /* 发送用户名 */
    Socket_SendString(file_client->client, file_client->user_name);
    Socket_SendDelay();
    /* 发送密码 */
    Socket_SendString(file_client->client, file_client->passwd);
    /* 等等服务器的验证信息 */
    if (strcmp(Socket_RecvString(file_client->client, _cmd_buf), SERVER_OK) == 0) {
        file_client->is_login = 1;
        return 1;
    }

    file_client->is_login = 0;
    return 0;
}

/* --私有 打印文件下载/上传的进度 */
static void _print_persent(int persent)
{
    // static int old_persent = -1;

    // if (old_persent != persent) {
        printf("\r已完成%d%% ...", persent);
    //     old_persent = persent;
    // }
}

/* 客户端上传文件
 * file_client: 客户端指针
 * file1: 待上传的文件完整路径
 * file2: 在服务器中保存的文件名
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_UpLoadFile(MyFileClient file_client, const char *file1, const char *file2)
{
    int err;

    /* 文件验证 */
    err = file_check(file1);
    if (err == 0) {
        printf("错误：系统中不存在文件%s\n", file1);
        return 0;
    } else if (err == -1) {
        printf("错误：%s是文件夹，暂不支持上传文件夹！\n", file1);
        return 0;
    }

    /* 发送上传请求 */
    Socket_SendString(file_client->client, CMD_SEND);
    Socket_SendDelay();
    /* 发送上传文件名 */
    Socket_SendString(file_client->client, file2);
    Socket_SendDelay();
    /* 发送文件 */
    return Socket_SendFile(file_client->client, file1, _print_persent) == 0;
}

/* 客户端下载文件
 * file_client: 客户端指针
 * file1: 待下载的文件名
 * file2: 保存的文件名
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_DownLoadFile(MyFileClient file_client, const char *file1, const char *file2)
{
    char *err;

    /* 发送下载请求 */
    Socket_SendString(file_client->client, CMD_GET);
    Socket_SendDelay();
    /* 发送待下载的文件名 */
    Socket_SendString(file_client->client, file1);
    /* 等等服务器的确认响应 */
    err = Socket_RecvString(file_client->client, NULL);
    if (strcmp(err, SERVER_ERR_NO_FILE) == 0) {
        printf("错误: 在服务器中不存在文件:“%s”！\n", file1);
        return 0;
    } else if (strcmp(err, SERVER_ERR_DIR) == 0) {
        printf("错误: “%s”是文件夹，暂不支持下载文件夹！\n", file1);
        return 0;
    }
    /* 接收文件 */
    return Socket_RecvFile(file_client->client, path_join(CLIENT_FILE_PATH, file2), _print_persent) == 0;
}

/* 更改服务器的当前路径
 * file_client: 客户端指针
 * path: 当前路径
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_Chdir(MyFileClient file_client, const char *path)
{
    /* 发送路径修改请求 */
    Socket_SendString(file_client->client, CMD_CD);
    Socket_SendDelay();

    /* 整理并发送路径 */
    if (path == NULL || strcmp(path, "/") == 0) {
        Socket_SendString(file_client->client, "/");
    } else if (strcmp(path, "..") == 0) {
        char *str = strrchr(file_client->current_path, '/');
        if (str == file_client->current_path)
            str[1] = 0;
        else
            *str = 0;
        //printf("cd ..: %s\n", file_client->current_path);
        Socket_SendString(file_client->client, file_client->current_path);
    } else if (strcmp(path, ".") == 0) {
        Socket_SendString(file_client->client, file_client->current_path);
    } else {
        Socket_SendString(file_client->client, path);
    }

    /* 等待服务器的结果 */
    if (strcmp(Socket_RecvString(file_client->client, NULL), SERVER_OK) != 0) {
        printf("操作失败！\n");
        return 0;
    }

    /* 更新当前路径 */
    MyFileClient_GetDir(file_client);
    return 1;
}

/* 更新服务器的当前路径
 * file_client: 客户端指针
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_GetDir(MyFileClient file_client)
{
    /* 发送路径更新请求 */
    Socket_SendString(file_client->client, CMD_PWD);
    /* 修改路径 */
    Socket_RecvString(file_client->client, file_client->current_path);
    return 1;
}

/* 获取服务器当前目录下的所有文件
 * file_client: 客户端指针
 * filebuf: 存放文件名的指针数组
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_GetDirFiles(MyFileClient file_client, char *filebuf[])
{
    int i = 0;
    const char *file_name;

    /* 发送获取服务器当前目录下的所有文件请求 */
    Socket_SendString(file_client->client, CMD_LS);

    /* 获得所有文件 */
    while((file_name = Socket_RecvString(file_client->client, NULL))[0] != '.') {
        //puts(file_name);
        filebuf[i] = malloc(strlen(file_name) + 1);
        //if (file_name[i])
            strcpy(filebuf[i++], file_name);
    }
    filebuf[i] = NULL;
    return i;
}

/* 在服务器当前目录下新建一个文件夹
 * file_client: 客户端指针
 * path: 文件夹名
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_Mkdir(MyFileClient file_client, const char *path)
{
    Socket_SendString(file_client->client, CMD_MKDIR);
    Socket_SendDelay();
    Socket_SendString(file_client->client, path);
    return  strcmp(Socket_RecvString(file_client->client, NULL), SERVER_OK) == 0;
}

/* 删除服务器中的文件或文件夹
 * file_client: 客户端指针
 * path: 文件夹名
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_RM(MyFileClient file_client, const char *path)
{
    Socket_SendString(file_client->client, CMD_RM);
    Socket_SendDelay();
    Socket_SendString(file_client->client, path);
    return  strcmp(Socket_RecvString(file_client->client, NULL), SERVER_OK) == 0;
}

/* 移动(重命名)服务器中的文件或文件夹
 * file_client: 客户端指针
 * path1: 原文件(夹)名
 * path2: 新文件(夹)名
 * 返回: 成功返回1，失败返回0
 */
int MyFileClient_MV(MyFileClient file_client, const char *path1, const  char *path2)
{
    Socket_SendString(file_client->client, CMD_MV);
    Socket_SendDelay();
    Socket_SendString(file_client->client, path1);
    Socket_SendDelay();
    Socket_SendString(file_client->client, path2);
    return  strcmp(Socket_RecvString(file_client->client, NULL), SERVER_OK) == 0;
}
