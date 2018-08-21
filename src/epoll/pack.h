
#pragma once


typedef struct 
{
	unsigned short    	version;
    unsigned int      	msgID;
    unsigned short	  	length;         // length of data
	unsigned char   	data[];
}net_pkg_t;


// 从数据流中解析出一个完整包
int parse_pack(void* buf, int* buf_len, void** pack);
void release_pack(void* pack);


int create_pack(void* buf, int buf_len, int msgID);

int pack_msgID(void* pack);




