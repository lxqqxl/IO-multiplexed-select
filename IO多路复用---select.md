# I/O多路复用---select

select函数：该函数准许进程指示内核等待多个事件中的任何一个发送，并只在有一个或者多个事件发生或经历一段指定的时间后唤醒

```c++
#include <sys/select.h>
#include <sys/time.h>

int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout)
返回值：就绪描述符的数目，超时返回，出错返回-1
```

