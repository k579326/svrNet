

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>

#include "conn.h"
#include "ep_svr.h"

using namespace std;


typedef int CONNID;

struct connect_table
{
	map<CONNID, svr_connect_t> 	conn_map;
	pthread_rwlock_t			rwlock;
};















