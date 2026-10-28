#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void beep_event(int fd, int value);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("too many arguments\n");
    return 1;
  }

  int fd = open("/dev/input/event3", O_WRONLY);
  if (fd == -1) {
    printf("Failed to open input device\n");
    return 1;
  }

  switch (*argv[1]) {
  case '0':
    beep_event(fd, 0);
    break;
  case '1':
    beep_event(fd, 1);
    break;
  default:
    printf("Invalid argument\n");
  }
  close(fd);

  return 0;
}

void beep_event(int fd, int value) {
  struct input_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = EV_SND;
  ev.code = SND_BELL;
  ev.value = value;
  write(fd, &ev, sizeof(ev));

  memset(&ev, 0, sizeof(ev));
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(fd, &ev, sizeof(ev));
}