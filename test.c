


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "svr_net.h"


int work_func(void* data, int len, void* outdata, int* outlen)
{
	char buf[1024 * 64];

	memcpy(buf, data, len);
	buf[len] = 0;

	printf("[msg] %s\n", buf);

	*outlen = 0;

	return 0;
}



int main(int argc, char** argv)
{
    int ret = 0;
    if (argc < 2)
    {
        printf("need input IP address!\n");
        return 0;
    }
    
	es_svrinfo_t* ptr = create_svrNet();
    if (ptr == NULL)
    {
        printf("server net start failed!\n");
        return 0;
    }


	svrNet_setThreadPool(ptr, work_func, NULL, NULL, 10);
	svrNet_start(ptr, argv[1], 10030);

	
    
    getchar();
    
    svrNet_stop(ptr);
	Destroy_svrNet(ptr);

	
	return 0;
}






