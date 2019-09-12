/* 华清远见项目实训 - 文件服务器
 *
 * socket.c 文件服务器Socket封装
 * 提供文件服务器Socket封装的实现
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#include "socket.h"
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

static char _recv_buf[RECV_BUF_SIZE];


/* 创建一个Socket服务器对象，创建socket并开始监听
 * type: socket类型(为SOCKET_TCP、SOCKET_UDP中的一个)
 * port: 端口号
 * listen_count: 监听的数量
 */
Socket Socket_CreateServer(SocketType type, uint16_t port, uint16_t listen_count)
{
    Socket _socket;
    int _type;

    /* TCP或者UDP */
    switch (type) {
        case SOCKET_TCP: _type = SOCK_STREAM; break;
        case SOCKET_UDP: _type = SOCK_DGRAM; break;
        default: return NULL;
    }
    /* 创建socket套接字 */
    _socket = (Socket)malloc(sizeof(struct _Socket));
    if (_socket) {
        _socket->socket = socket(AF_INET, _type, 0);
        if (_socket->socket == -1) {
            print_error("socket create error", EXIT_IF_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* 绑定 */
        bzero(&(_socket->_addr), sizeof(_socket->_addr));
        _socket->_addr.sin_family = AF_INET;
        _socket->_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        _socket->_addr.sin_port = htons(port);
        _socket->port = port;
        _socket->_sc = SOCKET_SERVER;
        inet_ntop(AF_INET, &_socket->_addr.sin_addr.s_addr, _socket->ipstr, sizeof(_socket->ipstr));

        if (bind(_socket->socket, (struct sockaddr *)&_socket->_addr, sizeof _socket->_addr) == -1) {
            print_error("socket bind error", EXIT_IF_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* 监听 */
        if (listen(_socket->socket, listen_count) == -1) {
            print_error("socket listen error", EXIT_IF_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
    }

    return _socket;
}

/* 创建一个Socket客户端对象，创建socket并连接服务器
 * type: socket类型(为SOCKET_TCP、SOCKET_UDP中的一个)
 * port: 服务器端口号
 * ipstr: 服务器ip地址
 * 返回客户端对象，失败返回NULL
 */
Socket Socket_CreateClient(SocketType type, const char *ipstr, uint16_t port)
{
    Socket _socket;
    int _type;

    /* TCP或者UDP */
    switch (type) {
        case SOCKET_TCP: _type = SOCK_STREAM; break;
        case SOCKET_UDP: _type = SOCK_DGRAM; break;
        default: return NULL;
    }
    /* 创建socket套接字 */
    _socket = (Socket)malloc(sizeof(struct _Socket));
    if (_socket) {
        _socket->socket = socket(AF_INET, _type, 0);
        if (_socket->socket == -1) {
            print_error("socket create error", EXIT_IF_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
        /* 绑定 */
        bzero(&(_socket->_addr), sizeof(_socket->_addr));
        _socket->_addr.sin_family = AF_INET;
        inet_pton(AF_INET, ipstr, &_socket->_addr.sin_addr.s_addr);
        _socket->_addr.sin_port = htons(port);
        _socket->port = port;
        _socket->_sc = SOCKET_CLIENT;
        strcpy(_socket->ipstr, ipstr);
        /* 连接 */
        if (connect(_socket->socket, (struct sockaddr *)&_socket->_addr, sizeof(_socket->_addr)) == -1) {
            print_error("socket connect error", EXIT_IF_SOCKET_ERROR);
            free(_socket);
            return NULL;
        }
    }

    return _socket;
}

/* 服务器Socket对象接受客户端的连接，连接成功后返回一个Server-Client Socket对象
 * _socket: 服务器Socket对象
 * 返回：连接到的客户端对象，如果失败返回NULL
 */
Socket Socket_ServerAccept(Socket _socket)
{
    Socket _client;
    socklen_t client_addr_len;

    if (!_socket || _socket->_sc != SOCKET_SERVER) {
        printf("Error: This Socket Object is not a ServerSocket\n");
        return NULL;
    }

    _client = (Socket) malloc(sizeof(struct _Socket));
    if (_client) {
        _client->socket = accept(_socket->socket, (struct sockaddr *)&_client->_addr, &client_addr_len);
        if (_client->socket == -1) {
            print_error("socket accept error", 0);
            free(_client);
            return NULL;
        }

        _client->port = ntohs(_client->_addr.sin_port);
        _client->_sc = SOCKET_SERVER_CLIENT;
        inet_ntop(AF_INET, &_socket->_addr.sin_addr.s_addr, _client->ipstr, sizeof(_client->ipstr));
    }

    return _client;
}

/* 发送字符串
 * _socket: Socket对象
 * str: 待发送的字符串
 * 返回发送的字符个数，失败返回-1
 */
ssize_t Socket_SendString(Socket _socket, const char *str)
{
    ssize_t n;

    if (!_socket) {
        printf("Error: Socket_SendString: This Socket Object is NULL\n");
        return -1;
    }
    /* 发送字符串 */
    if ((n = send(_socket->socket, str, strlen(str), 0)) == -1) {
        print_error("socket send error", 0);
    }

    return n;
}

/* 发送文件
 * _socket: Socket对象
 * file_name: 待发送的文件名
 * 返回: 发送成功返回0，失败返回-1
 */
int Socket_SendFile(Socket _socket, const char *file_name, void (*fun)(int))
{
    FILE *fp;
    size_t n;
    long file_size, current_size = 0;
    char buf[RECV_BUF_SIZE/2];
    int persent;

    if (!_socket) {
        printf("Error: Socket_SendFile: This Socket Object is NULL\n");
        return -1;
    }
    /* 获取文件大小 */
    if ((file_size = get_file_size(file_name)) == -1)
        return -1;
    printf("文件名: %s  ", file_name);
    print_file_size(file_size);
    /* 打开待发送的文件 */
    if ((fp = fopen(file_name, "r")) == NULL) {
        print_error(file_name, 0);
        return -1;
    }
    /* 先发送文件的大小 */
    if (send(_socket->socket, &file_size, sizeof file_size, 0) == -1) {
        print_error("socket send error", 0);
        fclose(fp);
        return -1;
    }
    Socket_SendDelay();
    /* 循环发送，直到文件发送完成 */
    while (!feof(fp)) {
        n = fread(buf, 1, RECV_BUF_SIZE/2, fp);
        current_size += n;
        send(_socket->socket, buf, n, 0);
        persent = current_size * 100 / file_size;
        if (fun)
            fun(persent);
    }

    fclose(fp);
    return 0;
}

/* 接收字符串
 * _socket: Socket对象
 * buf: 接收字符串缓存区，如果为NULL，使用内置缓存区
 * 返回：接收字符串缓存区
 */
char *Socket_RecvString(Socket _socket, char *buf)
{
    ssize_t n;
    buf = buf ? buf : _recv_buf;

    if (!_socket) {
        printf("Error: Socket_RecvString: This Socket Object is NULL\n");
        return NULL;
    }
    /* 接收字符串 */
    if ((n = recv(_socket->socket, buf, RECV_BUF_SIZE, 0)) == -1) {
        print_error("socket recv error", 0);
        return NULL;
    }
    buf[n] = '\0';

    return buf;
}

/* 接收文件
 * _socke: Socket对象
 * file_name: 接收到的文件的文件名
 * 返回: 如果接收失败返回-1，成功返回0
 */
int Socket_RecvFile(Socket _socket, const char *file_name, void (*fun)(int))
{
    FILE *fp;
    ssize_t n;
    int persent = 0;
    char *buf[RECV_BUF_SIZE];
    long file_size, currect_size = 0;

    if (!_socket) {
        printf("Error: Socket_RecvFile: This Socket Object is NULL\n");
        return -1;
    }
    /* 新建一个空文件，由于存储接收到的文件 */
    if ((fp = fopen(file_name, "w")) == NULL) {
        print_error(file_name, 0);
        return -1;
    }
    /* 获取待接收文件的大小 */
    if (recv(_socket->socket, &file_size, sizeof file_size, 0) == -1) {
        print_error("socket recv error", 0);
        fclose(fp);
        return -1;
    }
    printf("文件名: %s  ", file_name);
    print_file_size(file_size);
    /* 循环接收文件内容，直到接收完成（接收的内容等于文件内容） */
    while (currect_size < file_size) {
        n = recv(_socket->socket, buf, RECV_BUF_SIZE, 0);
        fwrite(buf, 1, (size_t)n, fp);
        currect_size += n;
        persent = currect_size * 100 / file_size;
        if (fun)
            fun(persent);
    }

    fclose(fp);
    return 0;
}

/* 打印socket对象
 */
void Socket_Print(Socket _socket)
{
    char *typestr, *_scstr;

    if (_socket == NULL) {
        printf("Socket(NULL)\n");
        return;
    }
    typestr = _socket->type == SOCKET_TCP ? "TCP" : "UDP";
    _scstr = _socket->_sc == SOCKET_SERVER ? "Sever" : _socket->_sc == SOCKET_CLIENT  ?  "Client" : "ServerClient";
    printf("%sSocket(type: %s, ip: %s, port: %d)\n", _scstr, typestr, _socket->ipstr, _socket->port);
}

/* 清除socket对象
 */
void Socket_Delete(Socket _socket)
{
    close(_socket->socket);
    free(_socket);
}

/* 发送间隔延时 */
void Socket_SendDelay(void)
{
    usleep(SOCKET_USLEEP_TIME);
}
