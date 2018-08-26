
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <arpa/inet.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include "ep_svr.h"
#include "config.h"
#include "common.h"
#include "engine.h"
#include "conn.h"
#include "sendhelper.h"
#include "recvhelper.h"
#include "pack.h"


static int set_socket_unblock(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	if (flag == -1)
	{
		printf("fcntl error : %s!", strerror(errno));
		return -1;
	}

	flag = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	if (flag == -1)
	{
		printf("fcntl error : %s!", strerror(errno));
		return -1;
	}

	return 0;
}



static void* listen_thread(void* param)
{
	net_svr_t* svr = (net_svr_t*)param;
	int err;

	err = listen(svr->svr_sock, 100);
	if (err != 0)
	{
		printf("%s\r\n", strerror(errno));
		return 0;
	}

	while (1)
	{
		svr_connect_t conn;
		struct sockaddr_in si;
		char ip[16];
		CONNID connId;
		struct epoll_event ep_evt;	
		
		memset(&conn, 0, sizeof(svr_connect_t));
		socklen_t size = sizeof(struct sockaddr_in);
		
		conn.clt.client_fd = accept(svr->svr_sock, (struct sockaddr*)&si, &size);
        
		if (conn.clt.client_fd < 0)
		{
			printf("[Accept] failed errmsg: %s \n", strerror(errno));
			continue;
		}
		else
		{
			if (inet_ntop(AF_INET, &si.sin_addr.s_addr, ip, 16) == NULL)
			{
				printf("[IP convert ERROR] %s\n", strerror(errno));
				return ;
			}
			
			memcpy(conn.clt.ip, &si.sin_addr.s_addr, sizeof(si.sin_addr.s_addr));
			conn.clt.iptype = 0; // ipv4
			
			/*
			uint32_t ipaddr = cds->net_info.sin_addr.s_addr;
			printf(" [Accept] client ip %d:%d:%d:%d\n", ipaddr & 0x000000FF, (ipaddr & 0x0000FF00) >> 8,
				(ipaddr & 0x00FF0000) >> 16, (ipaddr & 0xFF000000) >> 24);
			printf(" [Accept] socket for this client: %d\n", cds->client_fd);
			printf(" [Accept] port of client:         %d\n", ntohs(cds->net_info.sin_port));
			*/
		}

		set_socket_unblock(conn.clt.client_fd);

		ep_evt.events = EPOLLIN | EPOLLET | EPOLLOUT;
		connId = conn_insert(svr->conntable, &conn);
		ep_evt.data.u64 = connId;
		
		err = epoll_ctl(svr->ep_fd, EPOLL_CTL_ADD, conn.clt.client_fd, &ep_evt);
		if (err == -1)
        {
            conn_remove(svr->conntable, connId);
		    printf("[New Connect %d Add To Epoll Failed!]\n", conn.clt.client_fd);
        }
       
		usleep(1000 * 10);
	}

	return ;
}



static int handle_EPOLLIN_event(struct epoll_event* event, net_svr_t* svr)
{
    int err = 0;
	int curfd = 0;
	int result = 0;
	CONNID connid;
	
    unsigned char recv_buff[NET_RECV_BUFF_SIZE];		// 128K
	unsigned char cache_buf[NET_REMAIN_CACHE_SIZE];			// 512K
	int cache_size = NET_REMAIN_CACHE_SIZE;
	
	connid = event->data.u64;
	
	curfd = conn_get_sockfd(svr->conntable, connid);
	if (curfd == -1)
	{
		return 0;
	}
	
    err = recv(curfd, recv_buff, NET_RECV_SIZE_LIMIT, 0);

	if (err > 0)
	{
		// 搜索不完整数据缓冲区中是否存在该连接的数据。
		result = MRecv_find(svr->rhpr, connid, cache_buf, &cache_size);
		if (result == 0)
		{
			// 如果有，取出来放到recv_buff的前面。
			memmove(recv_buff + cache_size, recv_buff, err);
			memcpy(recv_buff, cache_buf, cache_size);
			err += cache_size;
		}	
		else if (result == -1 && cache_size != -1)
		{
			printf("[Exception] cache_buf not enough, data is %d\n", cache_size);
			assert(0);		// TODO: 不太可能吧，如果有，后续处理
		}
		else
		{
			// curfd不在map中
		}

		// 从map中移除
		MRecv_remove(svr->rhpr, connid);
	}

	cache_size = 0;	// 这个缓冲区重复利用
	
    while (err > 0)
    {
        int succ;
		int pack_len = 0;
		void* pack = NULL;
		
		// 很重要
		err += cache_size;
		
		// 解析当前recv_buff数据, 每解析出完整的一包，启动一个任务线程
		pack_len = parse_pack(recv_buff, &err, &pack);
		while (pack_len != 0)
		{
			while (workpool_push_work(&svr->pool, connid, pack, pack_len) == -1)
			{
				printf("[EXCEPTION] thread pool is full!\n");

	            // 休息100毫秒再试，那边已经处理不过来了
	            usleep(100 * 1000);  
			}
			pack_len = parse_pack(recv_buff, &err, &pack);
		}

		// 保留剩余数据长度
		cache_size = err;
		// 继续接收数据,不要覆盖buf中未处理的数据
		err = recv(curfd, recv_buff + err, NET_RECV_SIZE_LIMIT - err, 0);
    }

	if (err <= -1)
	{
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // cache_buf中的数据加socket扔进缓冲区，等待下次数据来。
            if (cache_size != 0)
				assert(MRecv_insert(svr->rhpr, connid, recv_buff, cache_size) == 0);
        }
        else
        {
            printf("[ERROR] exception occur! %d\n", errno);
        }
	}
	else if (err == 0)
	{// 断开连接了
		QSend_remove_all_by_connid(svr->shpr->m_queue, connid);	// 清理发送缓冲区中该连接未发送的数据
		conn_remove(svr->conntable, connid);					// 从连接表中删除连接
		MRecv_remove(svr->rhpr, connid);						// 从接收缓冲区中删除未处理的请求数据
		close(curfd);											// 关闭连接
	}

    return 0;
} 


static int handle_EPOLLOUT_event(struct epoll_event* event, net_svr_t* svr)
{
	CONNID connid;
	int sockfd = 0;
    int buflen = 0;
    int send_len = 0;
    unsigned char* ptr = NULL;
    unsigned char sendbuf[NET_SEND_BUFF_SIZE];

	buflen = NET_SEND_BUFF_SIZE;
	
	connid = event->data.u64;
	sockfd =  conn_get_sockfd(svr->conntable, connid);
	if (sockfd == -1)
	{
		return;
	}
	
	// 从发送缓冲区中找对应socket的数据包
	if (QSend_pop_first(svr->shpr->m_queue, connid, sendbuf, &buflen) == -1)
	{
		goto err;
	}

    ptr = sendbuf;

    do
    {
        send_len = send(sockfd, ptr, buflen, 0);

        if (send_len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("[EXCEPTION] socket Send buffer is Full!\n");
                // 休息100毫秒再试，网卡处理不过来了
                usleep(100 * 1000);
                continue;
            }
            else
            {
                printf("[ERROR] send error!\n");
				// TODO:写回缓冲区
                break;
            }
        }
        else
        {
            buflen = buflen - send_len;
            ptr += send_len;
        }
    }while (0 < buflen);

err:

	event->events = EPOLLIN | EPOLLET;
	epoll_ctl(svr->ep_fd, EPOLL_CTL_MOD, sockfd, event);

    return 0;
}



static void* work_routine(void* param)
{
	int err;
	net_svr_t* svr = (net_svr_t*)param;

	struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * NET_EPOLL_EVENT_MAX_SIZE);

	while (1)
	{
    	int i = 0;
		err = epoll_wait(svr->ep_fd, events, NET_EPOLL_EVENT_MAX_SIZE, -1);

		for (; i < err; i++)
		{
			if (events[i].events & EPOLLIN)
			{
                handle_EPOLLIN_event(&events[i], svr);
			}
			
            if (events[i].events & EPOLLOUT)
			{
                handle_EPOLLOUT_event(&events[i], svr);
			}
		}

		usleep(1000 * 1);
	}
}



net_svr_t* net_create(TASKHANDLER proc, int poolsize)
{
	int err = 0;
	net_svr_t* svr = (net_svr_t*)malloc(sizeof(net_svr_t));
	
	svr->svr_sock = -1;
	svr->ep_fd = -1;
	svr->conntable = NULL;
	svr->shpr = NULL;
	svr->rhpr = NULL;
	
	// 创建socket
	svr->svr_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (svr->svr_sock == -1)
	{
		err = -1;
		goto end;
	}
	
	// 创建epoll
	svr->ep_fd = epoll_create(1);
    if (svr->svr_sock == -1)
	{
		err = -1;
		goto end;
	}
	
	// 创建连接表
	svr->conntable = create_conn_table();
	
	// 创建数据接受助手
	svr->rhpr = create_recv_helper();
    
	// 创建数据发送助手
	svr->shpr = create_send_helper(svr);
    
	// 开启数据发送线程
    svr->shpr->start(svr->shpr->thread);

    // 任务处理线程池初始化
	workpool_start(&svr->pool, proc, poolsize);

end:	
	if (err == -1)
	{
		if (svr->svr_sock >= 0) close(svr->svr_sock);
		if (svr->ep_fd >= 0) close(svr->ep_fd);
		free(svr);
		svr = NULL;
	}
	
    return svr;
}

int net_start(net_svr_t* svr, char* ip, short port)
{
	int err;
    struct sockaddr_in si;
   
	si.sin_family = AF_INET;
	si.sin_port = htons(port);
	
	if (inet_pton(AF_INET, ip, &si.sin_addr.s_addr) != 1)
	{
		printf("[IP convert ERROR] %s\n", strerror(errno));
		return -1;
	}
	si.sin_addr.s_addr = inet_addr(ip);

    err = bind(svr->svr_sock, (struct sockaddr*)&si, sizeof(si));
	if (err != 0)
	{
		printf("[BIND ERROR] %s\n", strerror(errno));
		return -1;
	}
    
	pthread_create(&svr->loop_tid, NULL, work_routine, svr);
	pthread_create(&svr->lis_tid, NULL, listen_thread, &svr->svr_sock);
    
	return 0;
}


int net_destroy(net_svr_t* svr)
{
	// TODO: 拒绝所有epollin事件
	
	// 销毁数据接收助手
	release_recv_helper(svr->rhpr);
	// 停止线程池
	workpool_stop(&svr->pool);
	
	// TODO: 把所有发送队列中的数据清空或者全部发出去
	
	// TODO: 等待队列中数据发送完毕，然后停止数据发送线程
	
	// 销毁发送助手
	release_send_helper(svr->shpr);
	// 销毁连接表
    release_conn_table(svr->conntable);
    // 关闭epoll
	close(svr->ep_fd);
	
	free(svr);
	
    return 0;
}


