#include "sqlite3.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

// 此处设置端口号码，要与客户端的端口设置保持一致
// 建议设置程不常用的端口好，最好大于5000
#define SERVER_PORT 8188

sqlite3 *db;
char **errmsg;
char data[6];
char cmdSQL[128];

// 数据库回调函数
int callback(void *para, int f_num, char **f_val, char **f_name);
// 处理接收到的数据
void proceed(char *recvbuf);

int main(void) {
  struct sockaddr_in server_addr = {0};
  struct sockaddr_in client_addr = {0};
  char ip_str[20] = {0};
  int sockfd, connfd;
  int addrlen = sizeof(client_addr);
  char recvbuf[512];
  int ret;

  // 打开数据库
  ret = sqlite3_open("test.db", &db);
  if (ret) {
    printf("can't open database!\n");
    exit(1);
  }
  printf("database open successfully!\n");

  ret = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS autoRecd (tem VARCHAR(5))",
                     callback, NULL, errmsg);
  if (ret != SQLITE_OK) {
    printf("Error creating table: %s\n", sqlite3_errmsg(db));
    exit(1);
  }
  ret = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS manRecd (tem VARCHAR(5))",
                     callback, NULL, errmsg);
  if (ret != SQLITE_OK) {
    printf("Error creating table: %s\n", sqlite3_errmsg(db));
    exit(1);
  }

  /* 打开套接字，得到套接字描述符 */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (0 > sockfd) {
    perror("socket error");
    exit(EXIT_FAILURE);
  }

  /* 将套接字与指定端口号进行绑定 */
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERVER_PORT);

  ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (0 > ret) {
    perror("bind error");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  /* 使服务器进入监听状态 */
  ret = listen(sockfd, 50);
  if (0 > ret) {
    perror("listen error");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  /* 阻塞等待客户端连接 */
  connfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
  if (0 > connfd) {
    perror("accept error");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("有客户端接入...\n");
  inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, sizeof(ip_str));
  printf("客户端主机的IP地址: %s\n", ip_str);
  printf("客户端进程的端口号: %d\n", client_addr.sin_port);

  /* 接收客户端发送过来的数据 */
  for (;;) {

    // 接收缓冲区清零
    memset(recvbuf, 0x0, sizeof(recvbuf));

    // 读数据
    ret = recv(connfd, recvbuf, sizeof(recvbuf), 0);
    if (0 >= ret) {
      perror("recv error");
      close(connfd);
      break;
    }

    // 将读取到的数据以字符串形式打印出来
    printf("from client: %s\n", recvbuf);

    proceed(recvbuf);

    // 如果读取到"exit"则关闭套接字退出程序
    if (0 == strncmp("exit", recvbuf, 4)) {
      printf("server exit...\n");
      close(connfd);
      break;
    }
  }

  /* 关闭套接字 */
  close(sockfd);
  // 关闭数据库
  sqlite3_close(db);
  exit(EXIT_SUCCESS);
}

int callback(void *para, int f_num, char **f_val, char **f_name) {
  int i;

  for (i = 0; i < f_num; i++)
    printf(" %s |", f_val[i]);
  putchar('\n');

  return 0;
}

void proceed(char *recvbuf) {
  char flag=0;

  // 识别内容
  if (strncmp("autoRecd", recvbuf, 8) == 0) {
    memcpy(data, recvbuf + 8, 5);
    data[5] = '\0'; // 确保字符串以空字符结尾
    sprintf(cmdSQL, "INSERT INTO autoRecd VALUES ('%s')", data);
    printf("run cmdSQL：%s\n", cmdSQL);
    sqlite3_exec(db, cmdSQL, callback, NULL, errmsg);
    flag=1;
  } else if (strncmp("manRecd", recvbuf, 7) == 0) {
    memcpy(data, recvbuf + 7, 5);
    data[5] = '\0'; // 确保字符串以空字符结尾
    sprintf(cmdSQL, "INSERT INTO manRecd VALUES ('%s')", data);
    printf("run cmdSQL：%s\n", cmdSQL);
    sqlite3_exec(db, cmdSQL, callback, NULL, errmsg);
    flag=1;
  }

  if(flag){
    int fd=open("/myfile/newdata.csv",O_WRONLY|O_APPEND);
    write(fd,data,4);
    write(fd,"\n",1);
    close(fd);
  }
}
