

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ep_svr.h"



int work_func(const void* data, int len, void* outdata, int* outlen)
{
	char buf[1024 * 64];
	memcpy(buf, data, len);

	buf[len] = 0;

	printf("[msg] %s\n", buf);

    memcpy(outdata, "[resp msg] server have received msg from client, response for it!\n", 66);

	*outlen = 66;

	return 0;

}




int main(int argc, char** argv)
{
	net_svr_t* ptr = NULL;
    int ret = 0;

    if (argc < 2)
    {
        printf("need input IP address!\n");
        return 0;
    }

	ptr = net_create(work_func, 10);

    if (ptr == NULL)
    {
        printf("server net start failed!\n");
        return 0;
    }

	net_start(ptr, argv[1], 10030);

    getchar();

    net_destroy(ptr);

	return 0;
}











