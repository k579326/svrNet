

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net_config.h"
#include "pack.h"




int parse_pack(void* buf, int* buf_len, void** data)
{
	int data_len;
    net_pkg_t* head;
    
	if (*buf_len <= sizeof(net_pkg_t))
	{
		*data = NULL;
		return 0;
	}	

	
	head = (net_pkg_t*)buf;
	data_len = head->length;

	if (data_len + sizeof(net_pkg_t) > *buf_len)
	{
		*data = NULL;
		data_len = 0;
	}
	else
	{
		*data = (net_pkg_t*)malloc(data_len);
		memcpy(*data, (char*)buf + sizeof(net_pkg_t), data_len);
		memmove(buf, (char*)buf + sizeof(net_pkg_t) + data_len, *buf_len - data_len);
		*buf_len -= (sizeof(net_pkg_t) + data_len);
	}

	return data_len;
}


int create_pack(void* buf, int buf_len, int msgID)
{
	net_pkg_t* ptr = NULL;
	
	memmove((char*)buf + sizeof(net_pkg_t), buf, buf_len);

	ptr = (net_pkg_t*)buf;
	
	ptr->version = PACK_VERSION;
	ptr->length = buf_len;
	ptr->msgID = msgID;

	return buf_len + sizeof(net_pkg_t);
}


int pack_msgID(void* pack)
{
	return ((net_pkg_t*)pack)->msgID;
}




void release_pack(void* pack)
{
	free(pack);
	return ;
}






