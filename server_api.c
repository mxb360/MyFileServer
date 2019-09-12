/* 华清远见项目实训 - 文件服务器
 *
 * server_api.c 文件服务器服务器端外部接口的实现
 * 提供文件服务器服务器端外部接口的实现
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#include "server_api.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* 客户端密码检查
 * user_name: 用户名
 * passwd: 密码
 * 返回: 通过返回1，不通过返回0
 */
int MyFileServer_PasswdCheck(const char *user_name, const char *passwd)
{
    char passwd_buf[MAX_PASSWD];
    char *_passwd;

    _passwd = MyFileServer_GetUserPasswd(user_name, passwd_buf);
    if (_passwd == NULL)
        return 0;
    return strcmp(passwd, _passwd) == 0;
}

/* 获取用户的密码
 * user_name: 用户名
 * buf: 密码缓存区
 * 返回: buf， 失败返回NULL
 */
char *MyFileServer_GetUserPasswd(const char *user_name, char *buf)
{
    FILE *fp;

    if ((fp = fopen(path_join(USER_INFO_PATH, user_name), "r")) == NULL)
        return NULL;
    fgets(buf, MAX_PASSWD, fp);
    fclose(fp);
    str_strip(buf);
    return buf;
}

/* 客户端登录
 * file_server: 由服务器接收到的客户端
 * 返回: 成功返回1，失败返回0
 */
int MyFileServer_Login(MyFileServer file_server)
{
    if (file_server == NULL)
        return 0;

    file_server->is_login = MyFileServer_PasswdCheck(file_server->user_name, file_server->passwd);
    if (file_server->is_login == 0)
        return 0;

    strcpy(file_server->current_path, path_join(file_server->current_path, file_server->user_name));
    file_server->pwd = file_server->current_path + strlen(file_server->current_path);
    if (file_check(file_server->current_path) != -1) {
        char buf[60];
        printf("%s的用户目录不存在，将创建之\n", file_server->user_name);
        strcpy(buf, "mkdir ");
        strcat(buf, file_server->current_path);
        system(buf);
    }
    printf("用户文件目录： %s(%s)\n", file_server->pwd, file_server->current_path);

    return 1;
}

/* --私有-- 执行与路径命令 */
static int exec_cmd_about_file(MyFileServer file_server, const char *cmd, const char *args1, const char *args2)
{
    char buf1[RECV_BUF_SIZE], buf2[RECV_BUF_SIZE];
    strcpy(buf2, path_join(file_server->current_path, args1));

    if (args2) {
        strcat(buf2, " ");
        strcat(buf2, path_join(file_server->current_path, args2));
    }

    strcpy(buf1, cmd);
    strcat(buf1, buf2);
    printf("exec cmd: %s\n", buf1);
    if (system(buf1)) {
        printf("指令执行失败！\n");
        return  0;
    } else {
        printf("指令执行成功\n");
        return  1;
    }
}

/* 创建客户端
 * server: 服务器的Socket指针
 * client: 由服务器接收到的客户端Socket指针
 * 返回: 成功返回1，失败返回0
 */
MyFileServer MyFileServer_Create(Socket server, Socket client)
{
    MyFileServer file_server = (MyFileServer)malloc(sizeof(struct _MyFileServer));
    if (file_server) {
        file_server->server = server;
        file_server->client = client;
        file_server->is_login = 0;
        file_server->logout = 0;
        strcpy(file_server->current_path, SERVER_FILE_PATH);
        file_server->pwd = file_server->current_path + strlen(SERVER_FILE_PATH) - 1;
    }

    return file_server;
}

/* 更改当前路径
 * file_server: 服务器的Socket指针
 * path: 路径
 * 返回: 成功返回1，失败返回0
 */
int MyFileServer_Chdir(MyFileServer file_server, const char *path)
{
    const char *str;

    if (path[0] == '/') {
        str = path_join(path_join(SERVER_FILE_PATH, file_server->user_name), path);
    } else
        str = path_join(file_server->current_path, path);

    if (file_check(str) == -1) {
        strcpy(file_server->current_path, str);
        return 1;
    }
    return 0;
}

/* 获取当前路径
 * file_server: 服务器的Socket指针
 * buf: 存放路径的缓存区
 * 返回: 成功返回1，失败返回0
 */
int MyFileServer_GetDir(MyFileServer file_server, char *buf)
{
    strcpy(buf, file_server->pwd);
    return 1;
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

/* 处理客户端的指令
 * file_server: 服务器的Socket指针
 * 返回: 成功返回1，失败返回0
 */
int MyFileServer_ResolveCmd(MyFileServer file_server)
{
    char buf1[RECV_BUF_SIZE], buf2[RECV_BUF_SIZE];

    /* 接收指令 */
    Socket_RecvString(file_server->client, buf1);

    /* 登录 */
    if (strcmp(buf1, CMD_LOGIN) == 0) {
        printf("客户端[%s, %d]尝试登录...\n", file_server->client->ipstr, file_server->client->port);
        Socket_RecvString(file_server->client, buf1);
        strcpy(file_server->user_name, buf1);
        Socket_RecvString(file_server->client, buf1);
        strcpy(file_server->passwd, buf1);
        printf("来自该客户端的账号及密码：[%s, %s] 验证中...\n", file_server->user_name, file_server->passwd);
        if (MyFileServer_Login(file_server)) {
            printf("验证成功\n");
            Socket_SendString(file_server->client, SERVER_OK);
            return 1;
        } else {
            printf("验证失败\n");
            Socket_SendString(file_server->client, SERVER_ERR);
            return 0;
        }
    }

    /* 登出 */
    if (strcmp(buf1, CMD_LOGOUT) == 0 || buf1[0] == '\0') {
        file_server->is_login = 0;
        file_server->logout = 1;
        return 1;
    }

    /* 上传文件 */
    if (strcmp(buf1, CMD_SEND) == 0) {
        printf("客户端[%s, %d]请求上传文件\n", file_server->client->ipstr, file_server->client->port);
        Socket_RecvString(file_server->client, buf1);
        if (Socket_RecvFile(file_server->client, path_join(file_server->current_path, buf1), _print_persent) == -1) {
            printf("\n文件上传失败\n");
            return 0;
        }  else {
            printf("\n文件上传完成\n");
            return 1;
        }
    }

    /* 下载文件 */
    if (strcmp(buf1, CMD_GET) == 0) {
        int check;

        printf("客户端[%s, %d]请求下载文件\n", file_server->client->ipstr, file_server->client->port);
        Socket_RecvString(file_server->client, buf1);

        check = file_check(path_join(file_server->current_path, buf1));
        if (check == 0) {
            printf("错误：%s不是文件\n", buf1);
            Socket_SendString(file_server->client, SERVER_ERR_NO_FILE);
            return 0;
        } else if (check == -1) {
            printf("错误：%s是文件文件夹\n", buf1);
            Socket_SendString(file_server->client, SERVER_ERR_DIR);
            return 0;
        }

        Socket_SendString(file_server->client, SERVER_OK);
        Socket_SendDelay();

        if (Socket_SendFile(file_server->client, path_join(file_server->current_path, buf1), _print_persent) == -1) {
            printf("\n文件下载失败\n");
            return 0;
        } else {
            printf("\n文件下载结束\n");
            return 1;
        }
    }

    /* 更改目录 */
    if (strcmp(buf1, CMD_CD) == 0) {
        printf("客户端[%s, %d]请求更改目录\n", file_server->client->ipstr, file_server->client->port);
        Socket_RecvString(file_server->client, buf1);
        if (MyFileServer_Chdir(file_server, buf1) == 1) {
            printf("目录更改成功，新目录：%s(%s)\n", file_server->pwd, file_server->current_path);
            Socket_SendString(file_server->client, SERVER_OK);
            return 1;
        }
        Socket_SendString(file_server->client, SERVER_ERR);
        printf("目录更改失败\n");
        return 1;
    }

    /* 获取当前目录 */
    if (strcmp(buf1, CMD_PWD) == 0) {
        Socket_SendString(file_server->client, file_server->pwd);
        return 1;
    }

    /* 列出当前路径下的所有文件和目录 */
    if (strcmp(buf1, CMD_LS) == 0) {
        DIR    *dir;
        struct dirent *ptr;

        printf("客户端[%s, %d]请求获取当前路径下的所有文件\n", file_server->client->ipstr, file_server->client->port);
        dir = opendir(file_server->current_path);
        while((ptr = readdir(dir)) != NULL) {
            if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..") == 0)
                continue;
            strcpy(buf1 + 1, ptr->d_name);
            buf1[0] = (ptr->d_type == 8) ? '-' : '+';
            //puts(buf1);
            Socket_SendString(file_server->client, buf1);
            Socket_SendDelay();
        }
        closedir(dir);
        Socket_SendString(file_server->client, "...");
        return 1;
    }

    /* 新建文件夹 */
    if (strcmp(buf1, CMD_MKDIR) == 0) {
        printf("客户端[%s, %d]请求新建文件夹\n", file_server->client->ipstr, file_server->client->port);
        Socket_RecvString(file_server->client, buf1);
        if (exec_cmd_about_file(file_server, "mkdir ", buf1, NULL)) {
            Socket_SendString(file_server->client, SERVER_OK);
            return  1;
        } else {
            Socket_SendString(file_server->client, SERVER_ERR);
            return  0;
        }
    }

    /* 删除文件或文件夹 */
    if (strcmp(buf1, CMD_RM) == 0) {
        printf("客户端[%s, %d]请求删除文件或文件夹\n", file_server->client->ipstr, file_server->client->port);
        Socket_RecvString(file_server->client, buf1);
        if (exec_cmd_about_file(file_server, "rm -r ", buf1, NULL)) {
            Socket_SendString(file_server->client, SERVER_OK);
            return  1;
        } else {
            Socket_SendString(file_server->client, SERVER_ERR);
            return  0;
        }
    }

    /* 移动文件或文件夹 */
    if (strcmp(buf1, CMD_MV) == 0) {
        printf("客户端[%s, %d]请求移动文件或文件夹\n", file_server->client->ipstr, file_server->client->port);
        Socket_RecvString(file_server->client, buf1);
        Socket_RecvString(file_server->client, buf2);
        if (exec_cmd_about_file(file_server, "mv ", buf1, buf2)) {
            Socket_SendString(file_server->client, SERVER_OK);
            return  1;
        } else {
            Socket_SendString(file_server->client, SERVER_ERR);
            return  0;
        }
    }

    /* 未知指令 */
    printf("\"客户端[%s, %d]的未知指令: %s\n", file_server->client->ipstr, file_server->client->port, buf1);
    return 0;
}
