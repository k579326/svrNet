


#pragma once


#define     PACK_VERSION			0


#define		NET_PACK_SPACE_SIZE_LIMIT 	64 	* 1024			// �ͻ��������������ÿ�ε��û�������󳤶�

#define 	NET_RECV_BUFF_SIZE			(256 * 1024)		// ���ݽ��ջ������ռ䳤��

#define 	NET_PACK_SIZE_LIMIT		(NET_PACK_SPACE_SIZE_LIMIT + 1024)	// Э���+�û������ܳ�������
#define 	NET_REMAIN_CACHE_SIZE	NET_PACK_SIZE_LIMIT			    // �����������泤��
#define 	NET_RECV_SIZE_LIMIT		(NET_RECV_BUFF_SIZE - NET_REMAIN_CACHE_SIZE)	// ÿ�ν��յ����ݳ�������

#define 	NET_SEND_BUFF_SIZE			(256 * 1024)		// ���ͻ������ռ䳤��

// epoll_waitÿ�μ���������¼�����
#define 	NET_EPOLL_EVENT_MAX_SIZE	100

#define     NET_WORK_TYPE_MAX_SIZE      100