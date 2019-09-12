/* 华清远见项目实训 - 文件服务器
 *
 * config.h  文件服务器配置代码
 * 提供文件服务器的相关配置
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#ifndef _MY_CONFIG_H
#define _MY_CONFIG_H

/* 服务端的监听数、IP地址、端口号配置 */
#define SERVER_LISTEN 125
#define SERVER_PORT   8088
#define SERVER_IP     "127.0.0.1"

#define MAX_USER_NAME   20
#define MAX_PASSWD      15
#define SERVER_MAX_PATH 128

/* 服务器/客户端的文件路径配置 */
#define CLIENT_FILE_PATH   "./client-file/"
#define SERVER_FILE_PATH   "./server-file/"

#define USER_INFO_PATH     "./user-info/"

/* 服务器的应答信息配置 */
#define SERVER_OK          "ok"
#define SERVER_ERR         "err"
#define SERVER_ERR_NO_FILE "errnofile"
#define SERVER_ERR_DIR     "errordir"

/* 用于服务器和客户端的内部通信命令配置 */
#define CMD_LOGIN    "@login"
#define CMD_LOGOUT   "@logout"
#define CMD_SEND     "@send"
#define CMD_GET      "@get"
#define CMD_CD       "@cd"
#define CMD_PWD      "@pwd"
#define CMD_LS       "@ls"
#define CMD_MKDIR    "@mkdir"
#define CMD_RM       "@rm"
#define CMD_MV       "@mv"

#endif
