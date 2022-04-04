#ifndef INPUT_H
#define INPUT_H

#define READKEY_BUFF 64
#define KEY_RELEASE 0
#define KEY_PRESS 1

struct input_event ev[READKEY_BUFF];
int fd_ReadKey, rd_ReadKey, size_ReadKey = sizeof(struct input_event);
time_t t;

#endif
