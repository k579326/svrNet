


#pragma once


#define     PACK_VERSION			0


#define 	NET_PACK_MAX_SIZE		128 * 1024

#define 	NET_RECV_BUFF_SIZE		512 * 1024

// 防止缓冲区溢出的临界值
#define 	NET_BUFF_OVERFLOW_SIZE	128


// epoll_wait每次监听的最大事件个数
#define 	NET_EPOLL_EVENT_MAX_SIZE	100


#define     NET_WORK_TYPE_MAX_SIZE      100