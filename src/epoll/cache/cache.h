
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

int map_insert(int client_socket, const void* data, int len);


int map_find(int client_socket, void* data, int* len);


int map_remove(int client_socket);



int QSend_push(int socket, void* data, int len);
int QSend_Socket_of_front();
int QSend_pop_first(int socket, void* data, int* len);
int QSend_size();


#ifdef __cplusplus
}
#endif



