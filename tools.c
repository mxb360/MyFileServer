/* 华清远见项目实训 - 文件服务器
 *
 * tools.c 文件服务器公用函数的实现
 * 提供文件服务器公用函数的实现
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>

/* 打印当前错误
 * errstr: 错误描述
 * is_exit: 是否在出错时退出程序
 */
void print_error(const char *errstr, int is_exit)
{
    printf("Error %d: %s: %s\n", errno, errstr, strerror(errno));
    if (is_exit)
        exit(1);
}

/* 获取文件大小
 * filename: 文件名
 * 返回：文件大小，单位为字节，出错返回-1
 */
long get_file_size(const char *filename)
{
    struct stat statbuf;
    if (stat(filename,&statbuf) == -1) {
        print_error(filename, 0);
        return -1;
    }
    long size=statbuf.st_size;
    return size;
}

/* 打印文件大小
 * 打印格式：文件大小： xxx(xxx)\n
 */
void print_file_size(long size)
{
    if (size < 1024)
        printf("文件大小: %ldB", size);
    else if (size < 1048756)
        printf("文件大小: %.2fK", size / 1024.);
    else if (size < 1073741824)
        printf("文件大小: %.2fM", size / 1024. / 1024);
    else
        printf("文件大小: %.2fG", size / 1024. / 1024 / 1024);
    printf("(%ld)\n", size);
}

/* 文件（夹）检查
 * file_name: 文件名
 * 返回：如果是文件，返回1，是目录返回-1，不存在返回0
 */
int file_check(const char *file_name)
{
    struct stat info;
    if (stat(file_name, &info) < 0) {
        print_error(file_name, 0);
        return 0;
    }

    if (S_ISDIR(info.st_mode)) {
        //printf("Error: %s: Is a directory\n", file_name);
        return -1;
    }

    return 1;
}

/* 获取一个路径中的文件名
 * path: 路径
 * 返回: 文件名
 */
const char *get_file_name_in_path(const char *path)
{
    char *p;

    p = strrchr(path, '/');
#if defined(_WIN32) || defined(_WIN64)
    if (p == NULL)
        p = strrchr(path, '\\');
#endif
    return p ? p + 1: path;
}

/* 判断字符串是不是全是空白字符串
 * str: 待判断的字符串
 * 返回: 是返回1，不是返回0
 */
int is_space_str(const char *str)
{
    while (*str)
        if (!isspace(*str++))
            return 0;
    return 1;
}

/* 去除字符串的首尾空格
 * str: 待去除的字符串
 * 返回: str(去除后的字符串)
 */
char *str_strip(char *str)
{
    char *_str = str, *__str = str;
    while (isspace(*str))
        str++;
    while (*str)
        *_str++ = *str++;
    while (isspace(*--_str));
    *(_str + 1) = '\0';
    return __str;
}

/* 路径拼接，将两个路径连接成一个路径
 * path1: 路径1
 * path2: 路径2
 * 返回：拼接后的结果
 */
static char _path_join_buf[256];
const char *path_join(const char *path1, const char *path2)
{
    size_t size = strlen(path1);
    strcpy(_path_join_buf, path1);
    if (_path_join_buf[size - 1] != '/')
        _path_join_buf[size] = '/', _path_join_buf[size + 1] = '\0';
    while (*path2 == '/')
        path2++;
    strcat(_path_join_buf, path2);
    return _path_join_buf;
}

/* 字符串分割，将字符串按分割符分开，病存放到字符指针数组中
 * str: 待分割的字符串
 * des: 分割符
 * strbuf: 分割后的结果（用完请释放字符串的内存,直接使用free_strings(strbuf)即可）
 * buf_size: strbuf的大小
 * 返回: 分割后的字符串的个数
 */
int str_blit(char *str, const char *des, char *strbuf[], int buf_size)
{
    char *p;
    int i = 0;

    p = strtok(str, des);
    do {
        *strbuf++ = p;
        p = strtok(NULL, des);
    } while (++i < buf_size && p);
    *strbuf = NULL;

    return i;
}

/* 释放字符串指针中的空间
 * 注意，这些指针又malloc分配，且最后一个指针为NULL
 */
void free_strings(char *strbuf[])
{
    int i;

    for (i = 0; strbuf[i]; i++)
        free(strbuf[i]);
}
