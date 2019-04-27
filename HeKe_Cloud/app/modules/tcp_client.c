#include "tcp_client.h"

void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg)
{
	os_printf("sent data successed");
}

void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg)
{
	os_printf("disconnect successed");
}

void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg,char *pdata,unsigned short len)
{
	struct espconn *pespconn=arg;
	os_printf("receive data :%s\r\n",pdata);
	os_delay_us(300);
//	espconn_disconnect((struct espconn *)arg);
	if(strncmp(pdata,"(getall )",len)==0) //login success
	{
		is_connected=true;
		char buffer[255]={0};
		os_sprintf(buffer,"(setTermDetail \"pid\" \"1\" \"mid\" \"2\" \"cid\" \"1\" \"provider\" \"LSA\" \"category\" \"yuba\" \"model\" \"phone\" \"400-800-999\")\n");//C3-2
		espconn_sent(pespconn,buffer,strlen(buffer));
	}
}

void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg)
{
	struct espconn *pespconn=arg;
	char buffer[255]={0};
	espconn_regist_recvcb(pespconn,user_tcp_recv_cb);
	espconn_regist_sentcb(pespconn,user_tcp_sent_cb);
	espconn_regist_disconcb(pespconn,user_tcp_discon_cb);
	os_sprintf(buffer,"(login \"wq_8F_ccd29be70139\" \"code\" \"azBhQk9LR0tzelNncmhtK2dGMFNFOTNURFAyWkk1ZjFqaDZZOXYzOEVMbkRqRC9wSzVkNUlSY0o4ZG9KdTVyWWJQ\" \"DEVICE\")\n");//C3-1
	espconn_sent(pespconn,buffer,strlen(buffer));//teminal to login the cloud of the API
}

void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err)
{
	os_printf("wrong code：%d\r\n",err);
	espconn_connect((struct espconn *)arg);
}

void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip,int remote_port)
{

	user_tcp_conn.type=ESPCONN_TCP;
	user_tcp_conn.state=ESPCONN_NONE;
	user_tcp_conn.proto.tcp=(esp_tcp *)os_zalloc(sizeof(esp_tcp));
	os_memcpy(user_tcp_conn.proto.tcp->local_ip,local_ip,4);
	os_memcpy(user_tcp_conn.proto.tcp->remote_ip,remote_ip,4);
	user_tcp_conn.proto.tcp->local_port=espconn_port();
	user_tcp_conn.proto.tcp->remote_port=remote_port;

	espconn_regist_connectcb(&user_tcp_conn,user_tcp_connect_cb);
	espconn_regist_reconcb(&user_tcp_conn,user_tcp_recon_cb);

	espconn_connect(&user_tcp_conn);
}
