
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

#include "Glist.h"
#include "Gqueue.h"
#include "ep_svr.h"
#include "net_config.h"

#include "cache.h"






static client_data_t* create_client_data()
{
	client_data_t* cds = (client_data_t*)malloc(sizeof(client_data_t));
	memset(cds, 0, sizeof(client_data_t));

	return cds;
}


static void destroy_client_data(client_data_t* data)
{
    free(data);
    
    return ;
}

static void clearfunc(void* ptr)
{
    destroy_client_data((client_data_t*)ptr);
    return;
}


int init_env(struct es_svrinfo_t* info)
{
	pthread_mutex_init(&info->clt.cltlock, NULL);

    easy_list_init(&info->clt.cltlist, clearfunc);
	info->ep_fd = epoll_create(1);

	sendthread_init(&info->sender, info->ep_fd);
	sendthread_start(&info->sender);
	
    return 0;
}

int uninit_env(struct es_svrinfo_t* info)
{
	close(info->ep_fd);
    
    pthread_mutex_lock(&info->clt.cltlock);
    easy_list_uninit(&info->clt.cltlist);
    pthread_mutex_unlock(&info->clt.cltlock);
    pthread_mutex_destroy(&info->clt.cltlock);

	sendthread_uninit(&info->sender);
	
    return 0;
}

int set_socket_unblock(int fd)
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



void* listen_thread(void* param)
{
	struct es_svrinfo_t* pSrc = (struct es_svrinfo_t*)param;
	int err;

	err = listen(pSrc->sock_listen, 100);
	if (err != 0)
	{
		printf("%s\r\n", strerror(errno));
		return 0;
	}

	while (1)
	{
		client_data_t* cds = create_client_data();
		
		socklen_t size = sizeof(struct sockaddr_in);
		cds->client_fd = accept(pSrc->sock_listen, (struct sockaddr*)&cds->net_info, &size);
        
		if (cds->client_fd < 0)
		{
		    destroy_client_data(cds);
			printf("[Accept] failed errmsg: %s \n", strerror(errno));
			continue;
		}
		else
		{
			uint32_t ipaddr = cds->net_info.sin_addr.s_addr;
			printf(" [Accept] client ip %d:%d:%d:%d\n", ipaddr & 0x000000FF, (ipaddr & 0x0000FF00) >> 8,
				(ipaddr & 0x00FF0000) >> 16, (ipaddr & 0xFF000000) >> 24);
			printf(" [Accept] socket for this client: %d\n", cds->client_fd);
			printf(" [Accept] port of client:         %d\n", ntohs(cds->net_info.sin_port));
		}

		set_socket_unblock(cds->client_fd);

		struct epoll_event ep_evt;
		ep_evt.events = EPOLLIN | EPOLLET | EPOLLOUT;
		ep_evt.data.fd = cds->client_fd;
		err = epoll_ctl(pSrc->ep_fd, EPOLL_CTL_ADD, cds->client_fd, &ep_evt);
        
		pthread_mutex_lock(&pSrc->clt.cltlock);
        easy_list_insert_to_tail(pSrc->clt.cltlist, cds);
		pthread_mutex_unlock(&pSrc->clt.cltlock);

		usleep(1000 * 10);
	}

	return ;
}



static void remove_client(easy_list_t* clientlist, int fd)
{
	easy_item_t* item = NULL;
	client_data_t* tmp = NULL;

	item = easy_list_head(clientlist);
	while (item != NULL)
	{
		easy_list_getData(item, (void**)&tmp);
		if (tmp->client_fd == fd)
		{
		    // net_cache_uninit(tmp->buffer);
			printf("[Closed] clear client connect! socket:%d have been closed\n", tmp->client_fd);
			easy_list_remove(clientlist, item);
			break;
		}
		item = easy_list_next(item);
	}

	return;
}


static void* find_client(easy_list_t* clientlist, int fd)
{
    easy_item_t* item = NULL;
    client_data_t* tmp = NULL;

    item = easy_list_head(clientlist);
    while (item != NULL)
    {
        easy_list_getData(item, (void**)&tmp);
        if (tmp->client_fd == fd)
        {
            break;
        }
        item = easy_list_next(item);
    }

    return tmp;
}



static int handle_EPOLLIN_event(struct epoll_event* event, struct es_svrinfo_t* pSrc)
{
    int err = 0;
	int curfd = 0;
	int result = 0;
    unsigned char recv_buff[NET_RECV_BUFF_SIZE];		// 需要移动数据，要两倍大
	unsigned char cache_buf[NET_PACK_MAX_SIZE];
	int cache_size = NET_PACK_MAX_SIZE;

	curfd = event->data.fd;

    err = recv(curfd, recv_buff, NET_RECV_BUFF_SIZE, 0);

	if (err > 0)
	{
		// 搜索不完整数据缓冲区中是否存在该连接的数据。
		result = map_find(curfd, cache_buf, &cache_size);
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
		map_remove(curfd);
	}

	
	cache_size = 0;	// 这个缓冲区重复利用
	
    while (err > 0)
    {
        int succ;
		int pack_len = 0;
		void* pack = NULL;

		if (cache_size != 0)
		{
            memmove(recv_buff + cache_size, recv_buff, err);
			memcpy(recv_buff, cache_buf, cache_size);
			err += cache_size;
			cache_size = 0;	// 重置
		}
		
		
		// 解析当前recv_buff数据, 每解析出完整的一包，启动一个任务线程
		pack_len = parse_pack(recv_buff, &err, &pack);
		while (pack_len != 0)
		{
			while (workpool_dowork(&pSrc->pool, curfd, pack, pack_len) == -1)
			{
				printf("[EXCEPTION] thread pool is full!\n");

	            // 休息100毫秒再试，那边已经处理不过来了
	            usleep(100 * 1000);  
			}
			pack_len = parse_pack(recv_buff, &err, &pack);
		}

		assert(err >= 0);
		
		// recv_buf中剩下的不足一包的数据保留
		if (err > 0)
		{
			memcpy(cache_buf, recv_buff, err);
			cache_size = err;
		}

		err = recv(curfd, recv_buff, NET_RECV_BUFF_SIZE, 0);
    }

	if (err <= -1)
	{
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // cache_buf中的数据加socket扔进缓冲区，等待下次数据来。
            if (cache_size != 0)
				assert(map_insert(curfd, cache_buf, cache_size) == 0);
        }
        else
        {
            printf("[ERROR] exception occur! %d\n", errno);
        }
	}
	else if (err == 0)
	{
		pthread_mutex_lock(&pSrc->clt.cltlock);
		remove_client(pSrc->clt.cltlist, curfd);
		pthread_mutex_unlock(&pSrc->clt.cltlock);
		close(curfd);
	}

    return 0;
} 


static int handle_EPOLLOUT_event(struct epoll_event* event, struct es_svrinfo_t* pSrc)
{
	int sockfd = 0;
    int buflen = 0;
    int remain = 0;
    client_data_t* clientdata = NULL;
    unsigned char sendbuf[NET_PACK_MAX_SIZE];
	buflen = NET_PACK_MAX_SIZE;

	sockfd = event->data.fd;
	
	// 从发送缓冲区中找对应socket的数据包
	if (QSend_pop_first(sockfd, sendbuf, &buflen) == -1)
	{
		goto err;
	}

    do
    {
        remain = send(sockfd, sendbuf, buflen, 0);
        
        if (remain == -1)
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
            buflen = buflen - remain;
        }
    }while (0 <= buflen);


err:
	event->events = EPOLLIN | EPOLLET;
	epoll_ctl(pSrc->ep_fd, EPOLL_CTL_MOD, sockfd, event);

    return 0;
}



void* work_routine(void* param)
{
	int err;
	struct es_svrinfo_t* pSrc = (struct es_svrinfo_t*)param;

	struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * NET_EPOLL_EVENT_MAX_SIZE);

	while (1)
	{
    	int i = 0;
		err = epoll_wait(pSrc->ep_fd, events, NET_EPOLL_EVENT_MAX_SIZE, -1);

		for (; i < err; i++)
		{
			if (events[i].events & EPOLLIN)
			{
                handle_EPOLLIN_event(&events[i], pSrc);
			}
			
            if (events[i].events & EPOLLOUT)
			{
                handle_EPOLLOUT_event(&events[i], pSrc);
			}
		}

		usleep(1000 * 1);
	}
}


