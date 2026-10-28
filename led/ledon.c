#include <errno.h>  // errno
#include <fcntl.h>  // open()
#include <stdio.h>  // perror()
#include <unistd.h> // close(), read(), write()

int main(int argc, char *argv[]) {
  int fd = open("/dev/myled", O_RDWR);
  if (fd < 0) {
    perror("fail to open");
    return -1;
  }

  if (argc != 2) {
    printf("number of arguments is wrong!\n");
    return -1;
  }

  char buf[1];
  buf[0] = atoi(argv[1]);
  write(fd, buf, sizeof(buf));

  close(fd);
  return 0;
}