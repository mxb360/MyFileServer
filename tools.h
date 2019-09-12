/* 华清远见项目实训 - 文件服务器
 *
 * tools.h 文件服务器公用函数原型
 * 提供文件服务器公用函数原型
 *
 * 作者: Ma Xiaobo
 * 日期: 2018-11
 */

#ifndef _MY_TOOLS_H
#define _MY_TOOLS_H

void  print_error(const char *errstr, int is_exit);
long  get_file_size(const char *filename);
int   file_check(const char *file_name);
const char *get_file_name_in_path(const char *path);
int   is_space_str(const char *str);
char *str_strip(char *str);
const char *path_join(const char *path1, const char *path2);
int   str_blit(char *str, const char *des, char *strbuf[], int buf_size);
void  free_strings(char *strbuf[]);
void  print_file_size(long size);

#endif
