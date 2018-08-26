
//#include "windows.h"

#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <winsock2.h>


#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)
typedef struct
{
	unsigned short    	version;
	unsigned int      	msgID;
	unsigned int	  	length;         // length of data
	unsigned char   	data[];
}net_pkg_t;
#pragma pack(pop)




int test(int argc, char** argv)
{

	WSADATA data;

	if (argc != 2)
	{
		printf("argument is not correct!\n");
	}

	int ret = WSAStartup(MAKEWORD(2, 2), &data);
	
	
	int sock_fd;
	sockaddr_in addr;

	for (int i = 0; i < 1000; i++)
	{
		sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		addr.sin_family = AF_INET;
		inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);
		addr.sin_port = htons(10030);
		ret = connect(sock_fd, (sockaddr*)&addr, sizeof(sockaddr_in));
		if (ret != 0)
		{
			printf("[Failed]connect error, errcode = %d\n", WSAGetLastError());
		}
		else
		{
			printf("[Success]connect success!\n");
		}

		char sendbuf[1024 * 8];
		
		net_pkg_t* pack = (net_pkg_t*)sendbuf;

		pack->version = 0;
		pack->msgID = GetTickCount();
		pack->length = 39;
		memcpy(pack->data, "it is test message! head how to check?", 39);

		ret = send(sock_fd, sendbuf, sizeof(net_pkg_t) + 39, 0);
		printf("  send %d bytes!\n", ret);

		char buf[1024];
		ret = recv(sock_fd, buf, 100, 0);
		buf[ret] = 0;
		net_pkg_t* resp = (net_pkg_t*)buf;
		printf("  recv %d bytes!\n", ret);
		assert(resp->msgID == pack->msgID);
		
		printf("  server msg: %s\n", resp->data);
		//"[resp msg] server have received msg from client, response for it!\n"
		closesocket(sock_fd);
	}

    return 0;
}




int sendfile(int argc, char** argv)
{
    WSADATA data;

    if (argc != 2)
    {
        printf("argument is not correct!\n");
    }

    int ret = WSAStartup(MAKEWORD(2, 2), &data);


    int sock_fd;
    sockaddr_in addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);
    addr.sin_port = htons(10030);
    ret = connect(sock_fd, (sockaddr*)&addr, sizeof(sockaddr_in));
    if (ret != 0)
    {
        printf("[Failed]connect error, errcode = %d\n", WSAGetLastError());
    }
    else
    {
        printf("[Success]connect success!\n");
    }

    char sendbuf[1024 * 16];

    net_pkg_t* pack = (net_pkg_t*)sendbuf;

    pack->version = 0;
    int timelost = GetTickCount();

    //HANDLE fileHandle = CreateFile("text2.txt", GENERIC_ALL, 0, NULL, CREATE_ALWAYS, 0, NULL);
    //
    //SetFilePointer(fileHandle, 0x1fffffff, NULL, FILE_BEGIN);
    //SetEndOfFile(fileHandle);
    //CloseHandle(fileHandle);

    FILE* pf = fopen("text2.txt", "rb");
    //fseek(pf, 0, SEEK_SET);

    int totalsize = 0;
    for (int i = 0; feof(pf) == 0; i++)
    {
        pack->msgID = 0x7f616425;
        int readsize = fread(pack->data, 1, 1024 * 5, pf);
        if (readsize == 0)
        {
            printf("read file return 0!\n");
            continue;
        }

        if (readsize < 0)
        {
            printf("read file failed!\n");
            return 0;
        }

        pack->length = readsize;
        totalsize += readsize;

        int sendsize = send(sock_fd, sendbuf, sizeof(net_pkg_t) + readsize, 0);

        printf(" %d times, send bytes %d!\n", i, sendsize);
    }

    char retmsg[100];
    int recvsize = recv(sock_fd, retmsg, 24, 0);

    printf("[resp] %s\n", ((net_pkg_t*)retmsg)->data);

    fclose(pf);
    closesocket(sock_fd);

    float speed;

    speed = totalsize * 1.0 / (GetTickCount() - timelost) * 1000 / 1024;
    printf("[success] send completed! send bytes %d, lost time %dms, speed: %.3f\n", totalsize, (GetTickCount() - timelost), speed);


    getchar();

    return 0;
}



int senddata(int argc, char** argv)
{
    WSADATA data;

    if (argc != 2)
    {
        printf("argument is not correct!\n");
    }

    int ret = WSAStartup(MAKEWORD(2, 2), &data);


    int sock_fd;
    sockaddr_in addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);
    addr.sin_port = htons(10030);
    ret = connect(sock_fd, (sockaddr*)&addr, sizeof(sockaddr_in));
    if (ret != 0)
    {
        printf("[Failed]connect error, errcode = %d\n", WSAGetLastError());
    }
    else
    {
        printf("[Success]connect success!\n");
    }

    char sendbuf[1024 * 64];
    int bufsize = 2 << 30 - 1;
    int blocksize = 1024 * 64;


    net_pkg_t* pack = (net_pkg_t*)sendbuf;

    pack->version = 0;
    int timelost = GetTickCount();

    int totalsize = 0;
    for (int i = 0; bufsize > 0; i++)
    {
        if (bufsize <= blocksize)
        {
            blocksize = bufsize - 1;
            bufsize = blocksize;
        }
        pack->msgID = 0x7f616425;

        pack->length = blocksize;
        totalsize += (blocksize + sizeof(net_pkg_t));

        int sendsize = send(sock_fd, sendbuf, sizeof(net_pkg_t) + blocksize, 0);

        printf(" %d times, send bytes %d!\n", i, sendsize);

        bufsize -= blocksize;
    }

    char retmsg[100];
    int recvsize = recv(sock_fd, retmsg, 100, 0);

    assert(recvsize == sizeof(net_pkg_t) + ((net_pkg_t*)retmsg)->length);

    printf("[resp] %s\n", ((net_pkg_t*)retmsg)->data);

    closesocket(sock_fd);

    float speed;

    speed = totalsize * 1.0 / (GetTickCount() - timelost) * 1000 / 1024;
    printf("[success] send completed! send bytes %d, lost time %dms, speed: %.3f\n", totalsize, (GetTickCount() - timelost), speed);


    getchar();

    return 0;
}



int main(int argc, char** argv)
{
    return senddata(argc, argv);
}