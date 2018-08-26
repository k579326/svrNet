

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "pack.h"




int parse_pack(void* buf, int* buf_len, void** pack)
{
	int pack_len;
    net_pkg_t* head;
    
	if (*buf_len < sizeof(net_pkg_t))
	{
		*pack = NULL;
		return 0;
	}	

	
	head = (net_pkg_t*)buf;
	pack_len = head->length + sizeof(net_pkg_t);

	if (pack_len > *buf_len)
	{
		*pack = NULL;
		pack_len = 0;
	}
	else
	{
		*pack = (net_pkg_t*)malloc(pack_len);
		memcpy(*pack, (char*)buf, pack_len);
		memmove(buf, (char*)buf + pack_len, *buf_len - pack_len);
		*buf_len -= (pack_len);
	}

	return pack_len;
}


int create_pack(void* buf, int buf_len, int msgID)
{
	net_pkg_t* ptr = (net_pkg_t*)buf;

	memmove(ptr->data, buf, buf_len);

	
	//memmove((char*)buf + sizeof(net_pkg_t), buf, buf_len);

	ptr->version = PACK_VERSION;
	ptr->length = buf_len;
	ptr->msgID = msgID;

	return buf_len + sizeof(net_pkg_t);
}


void release_pack(void* pack)
{
	free(pack);
	return ;
}






