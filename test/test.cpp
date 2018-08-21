// socketTest.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#include "windows.h"
#include "winsock2.h"
#include <WS2tcpip.h>

#include <assert.h>

#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)
typedef struct
{
	unsigned short    	version;
	unsigned int      	msgID;
	unsigned short	  	length;         // length of data
	unsigned char   	data[];
}net_pkg_t;
#pragma pack(pop)


int main(int argc, char** argv)
{

	WSADATA data;

	if (argc != 2)
	{
		printf("argument is not correct!\n");
	}

	int ret = WSAStartup(MAKEWORD(2, 2), &data);
	
	
	int sock_fd;
	sockaddr_in addr;

	for (int i = 0; i < 1; i++)
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
		pack->msgID = 199954;
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
		//closesocket(sock_fd);
	}


	
	getchar();

    return 0;
}

