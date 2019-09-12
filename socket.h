/* 华清远见项目实训 - 文件服务器
 *
 * socket.h 文件服务器Socket封装原型
 * 提供文件服务器Socket封装的原型
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */


#ifndef _MY_SOCKET_H
#define _MY_SOCKET_H


#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* socket的类型(TCP / UDP) */
typedef enum _SocketType {
    SOCKET_TCP, SOCKET_UDP,
} SocketType;

/* Socket对象的类型标记 */
#define SOCKET_SERVER        0
#define SOCKET_CLIENT        1
#define SOCKET_SERVER_CLIENT 2

#define EXIT_IF_SOCKET_ERROR 0
#define RECV_BUF_SIZE   2096

#define SOCKET_USLEEP_TIME 30000

/* Socket对象的封装 */
typedef struct _Socket {
    int socket;                  /* socket文件描述符 */
    SocketType type;             /* socket类型 */
    int port;                    /* 端口号 */
    int _sc;
    char ipstr[20];              /* IP地址 */
    struct sockaddr_in _addr;
} *Socket;

Socket  Socket_CreateServer(SocketType type, uint16_t port, uint16_t listen_count);
Socket  Socket_CreateClient(SocketType type, const char *ipstr, uint16_t port);
Socket  Socket_ServerAccept(Socket _socket);
ssize_t Socket_SendString(Socket _socket, const char *str);
char *Socket_RecvString(Socket _socket, char *buf);
int   Socket_SendFile(Socket _socket, const char *file_name, void (*fun)(int));
int   Socket_RecvFile(Socket _socket, const char *file_name, void (*fun)(int));
void  Socket_Print(Socket _socket);
void  Socket_Delete(Socket _socket);
void  Socket_SendDelay(void);

#endif
