
#pragma once

#pragma pack(push, 1)
typedef struct 
{
	unsigned short    	version;
    unsigned int      	msgID;
    unsigned int	  	length;         // length of data
	unsigned char   	data[];
}net_pkg_t;
#pragma pack(pop)

// 从数据流中解析出一个完整包
int parse_pack(void* buf, int* buf_len, void** pack);
void release_pack(void* pack);


int create_pack(void* buf, int buf_len, int msgID);





