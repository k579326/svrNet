


#pragma once


#define     PACK_VERSION			0


#define		NET_PACK_SPACE_SIZE_LIMIT 	64 	* 1024			// 客户端与服务器交互每次的用户数据最大长度

#define 	NET_RECV_BUFF_SIZE			(256 * 1024)		// 数据接收缓冲区空间长度

#define 	NET_PACK_SIZE_LIMIT		(NET_PACK_SPACE_SIZE_LIMIT + 1024)	// 协议包+用户数据总长度上限
#define 	NET_REMAIN_CACHE_SIZE	NET_PACK_SIZE_LIMIT			    // 不完整包缓存长度
#define 	NET_RECV_SIZE_LIMIT		(NET_RECV_BUFF_SIZE - NET_REMAIN_CACHE_SIZE)	// 每次接收的数据长度上限

#define 	NET_SEND_BUFF_SIZE			(256 * 1024)		// 发送缓冲区空间长度

// epoll_wait每次监听的最大事件个数
#define 	NET_EPOLL_EVENT_MAX_SIZE	100

#define     NET_WORK_TYPE_MAX_SIZE      100