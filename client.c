/* 华清远见项目实训 - 文件服务器
 *
 * client.c  客户端部分源码
 * 提供文件服务器的客户端部分的实现
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#include "socket.h"
#include "tools.h"
#include "config.h"
#include "client_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sys_exit = 0;
MyFileClient file_client;

/* 显示帮助 */
void show_help(void)
{
    printf("华清远见项目实训 - 文件服务器V1.0 客户端部分\n");
    printf("作者： Ma Xiaobo\n日期：2018-11\n\n");
    printf("cmd:\n    ");
    printf("put  $file1 [$file2] 将文件$file1上传至服务器[并重命名为$file2]\n    ");
    printf("get  $file1 [$file2] 将文件$file1c重服务器下载至本地目录[并重命名为$file2]\n    ");
    printf("cd [$path]      进入服务器根目录[$path目录]\n    ");
    printf("mkdir $folder   在服务器当前目录下创建目录$folder\n    " );
    printf("quit/exit       退出程序\n    ");
    printf("help            显示本帮助\n    ");
    printf("ls              列出当前服务器当前目录下的所有文件\n    ");
    printf("rm $file        列出服务器中的文件或文件夹$file\n    ");
    printf("                注意：此指令非常危险！删除后不可恢复！\n    ");
    printf("mv $file $file2 将文件$file重命名为&file2\n");
}

/* 连接服务器 */
Socket connect_server(char *ipstr, int port)
{
    Socket client;

    printf("正在连接服务器[%s, %d] ...\n", ipstr, port);
    client = Socket_CreateClient(SOCKET_TCP, ipstr, port);
    if (client == NULL) {
        printf("服务器连接失败！\n");
        exit(1);
    } else
        printf("服务器连接成功。\n");

    printf("\n来自服务器的消息：");
    puts(Socket_RecvString(client, NULL));
    return client;
}

/* 登录到服务器 */
void login_server(Socket client)
{
    int login_success = 0;
    char pwd_buf[MAX_PASSWD], user_name[MAX_USER_NAME];

    printf("\n你需要登录到服务器:\n");
    while (!login_success) {
        printf("请输入账号： ");
        fgets(user_name, sizeof user_name, stdin);
        str_strip(user_name);

        printf("请输入密码： ");
        fgets(pwd_buf, sizeof pwd_buf, stdin);
        str_strip(pwd_buf);

        file_client = MyFileClient_Create(client, user_name, pwd_buf);
        if (MyFileClient_Login(file_client)) {
            printf("\n登录成功！\n\n");
            login_success = 1;
        } else {
            printf("\n登录失败，请重试：\n");
            free(file_client);
        }
    }
}

/* 客户端退出 */
void client_exit(void)
{
    Socket_SendString(file_client->client, CMD_LOGOUT);
    Socket_Delete(file_client->client);
    free(file_client);
    printf("Bye!\n");
    exit(0);
}

/* 客户端启动检查 */
void client_check(void)
{
    if (file_check(CLIENT_FILE_PATH) != -1)
        system("mkdir " CLIENT_FILE_PATH);
    printf("用户文件下载目录：%s\n", CLIENT_FILE_PATH);


}

/* 处理客户端命令 */
void resolve_cmd(char *str)
{
    char *strbuf[20], *cmd;

    str_blit(str, " ", strbuf, 20);
    cmd = strbuf[0];

    /* 帮助 */
    if (strcmp("help", cmd) == 0) {
        show_help();
        return;
    /* 退出 */
    } else if (strcmp("quit", cmd) == 0 || strcmp("exit", cmd) == 0) {
        client_exit();
    /* 上传文件 */
    } else if (strcmp("put", cmd) == 0 ) {
        const char *file1, *file2;

        if (strbuf[1] == NULL) {
            printf("send 指令不全，输入“help”查看帮助。\n");
            return;
        }
        /* 获取文件名 */
        file1 = strbuf[1];
        file2 = strbuf[2] ? get_file_name_in_path(strbuf[2]) : get_file_name_in_path(file1);
        printf("开始上传文件：%s ...\n", file1);
        if (MyFileClient_UpLoadFile(file_client, file1, file2))
            printf("\n文件上传完成。保存到服务器:%s\n", path_join(file_client->current_path, file2));
        else
            printf("\n文件上传失败。\n");
    /* 下载文件 */
    } else if (strcmp("get", cmd) == 0) {
        const char *file1, *file2;

        if (strbuf[1] == NULL) {
            printf("get 指令不全，输入“help”查看帮助。\n");
            return;
        }
        /* 获取文件名 */
        file1 = strbuf[1];
        file2 = strbuf[2] ? get_file_name_in_path(strbuf[2]) : get_file_name_in_path(file1);
        printf("开始下载文件：%s ...\n", file1);
        if (MyFileClient_DownLoadFile(file_client, file1, file2))
            printf("\n文件下载完成。保存到本地:%s\n", path_join(CLIENT_FILE_PATH, file2));
        else
            printf("\n文件下载失败。\n");
    /* 更改目录 */
    } else if (strcmp("cd", cmd) == 0) {
        const char *dir = strbuf[1] == NULL ? "/" : strbuf[1];
        if (MyFileClient_Chdir(file_client, dir))
            printf("目录更改成功\n");
        else
            printf("目录更改失败，没有“%s”目录\n", dir);
    /* 查看所有文件 */
    } else if (strcmp("ls", cmd) == 0) {
        int i;
        char *filebuf[128];

        MyFileClient_GetDirFiles(file_client, filebuf);
        /* 输出文件/文件夹名 */
        for (i = 0; filebuf[i]; i++) {
            printf("  %s", filebuf[i] + 1);
            if (filebuf[i][0] == '+')
                printf("/");
            printf("\n");
        }
        free_strings(filebuf);
    /* 新建文件夹 */
    } else if (strcmp("mkdir", cmd) == 0) {
        if (strbuf[1] == NULL) {
            printf("mkdir 指令不全，输入“help”查看帮助。\n");
            return;
        }

        if (MyFileClient_Mkdir(file_client, strbuf[1])) {
            printf("操作成功。\n");
        } else {
            printf("操作失败!\n");
        }
    /* 删除文件(夹) */
    } else if (strcmp("rm", cmd) == 0) {
        if (strbuf[1] == NULL) {
            printf("rm 指令不全，输入“help”查看帮助。\n");
            return;
        }
        if (strbuf[1][0] == '/') {
            printf("rm /...: 不允许的操作。请去除最前面的'/'\n");
            return;
        }
        if (MyFileClient_RM(file_client, strbuf[1])) {
            printf("文件(夹)“%s”已删除。\n", strbuf[1]);
        } else {
            printf("删除失败!文件“%s”不存在。\n", strbuf[1]);
        }
    /* 移动/重命名文件(夹) */
    } else if (strcmp("mv", cmd) == 0) {
        if (strbuf[1] == NULL || strbuf[2] == NULL) {
            printf("mv 指令不全，输入“help”查看帮助。\n");
            return;
        }
        if (strbuf[1][0] == '/') {
            printf("mv /...: 不允许的操作。请去除最前面的'/'\n");
            return;
        }
        if (MyFileClient_MV(file_client, strbuf[1], strbuf[2])) {
            printf("成功将文件(夹)“%s”重命名/移动为“%s”。\n", strbuf[1], strbuf[2]);
        } else {
            printf("重命名/移动失败,文件“%s”不存在!\n", strbuf[1]);
        }
    /* 无效指令 */
    } else
        printf("错误：无法识别的指令“%s”，输入“help”查看帮助。\n", cmd);
}


int main(int argc, char *argv[])
{
    Socket client;
    char *ipstr;
    int port;
    char buf[RECV_BUF_SIZE];

    /* 处理命令行参数 */
    if (argc == 1)
        ipstr = SERVER_IP, port = SERVER_PORT;
    else if (argc == 2)
        ipstr = argv[1], port = SERVER_PORT;
    else if (argc == 3)
        ipstr = argv[1], port = atol(argv[2]);
    else {
        printf("参数错误。 使用：%s [ip] [port]\n", argv[0]);
        return 1;
    }

    /* 连接服务器 */
    printf("华清远见项目实训 - 文件服务器V1.0 客户端部分 \n");
    client = connect_server(ipstr, port);

    /* 登录服务器 */
    login_server(client);
    client_check();

    printf("\n华清远见项目实训 - 文件服务器V1.0\n");
    printf("输入“help“更多信息\n\n");

    /* 等等输入命令并处理命令 */
    while (!sys_exit) {
        printf("(%s)%s> ", file_client->user_name, file_client->current_path);
        fgets(buf, RECV_BUF_SIZE, stdin);

        /* 处理输入的字符串 */
        if (is_space_str(buf))
            continue;
        str_strip(buf);

        /* 处理命令 */
        resolve_cmd(buf);
        printf("\n");
    }

    return 0;
}
