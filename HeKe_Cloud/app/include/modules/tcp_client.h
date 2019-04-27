#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"

bool is_connected;
struct espconn user_tcp_conn;
void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip,int remote_port);

#endif

