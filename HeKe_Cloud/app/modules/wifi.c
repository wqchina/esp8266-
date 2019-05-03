/*
 * wifi.c
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */
#include "wifi.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "tcp_client.h"
//#include "mqtt_msg.h"
//#include "debug.h"
//#include "user_config.h"
//#include "config.h"


static ETSTimer wifilink;
static uint8_t wifiStatus = STATION_IDLE;

//wificall wificb=NULL;

void ICACHE_FLASH_ATTR user_esp_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct ip_info info;
	wifi_get_ip_info(STATION_IF,&info);
	my_station_init(ipaddr,&info.ip,9999);//使用TCP连接HeKr云（IP地址，端口9999）远端IP 本地IP 远端端口
}


static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	struct ip_info ipConfig;
	struct ip_addr addr;
	os_timer_disarm(&wifilink);
	wifi_get_ip_info(STATION_IF, &ipConfig);//查询wifi_station接口或者soft-ap接口的IP地址
	wifiStatus = wifi_station_get_connect_status();//查询wifi station接口连接AP的状态
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)//已连接上AP
	{
//		sever_init(&ipConfig.ip,80);
		espconn_gethostbyname(&user_tcp_conn,"device.hekr.me", &addr,user_esp_dns_found);//DNS功能（对应网络传输的结构体，域名字符串的指针，IP地址，DNS回调函数）
	}
	else
	{
		if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		{
			os_printf("STATION_WRONG_PASSWORD\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
		{
			os_printf("STATION_NO_AP_FOUND\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
		{
			os_printf("STATION_CONNECT_FAIL\r\n");
			wifi_station_connect();
		}
		else
		{
			os_printf("STATION_IDLE\r\n");
		}
		os_timer_setfn(&wifilink, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&wifilink, 500, 0);
	}
	//	wificb(wifiStatus,&ipConfig,80);
}

void ICACHE_FLASH_ATTR wifi_connect()
{
	struct station_config stationConf;//wifi_station接口
	uint8 *ssid,*password;
//	wificb=func;
	ssid=(uint8 *)os_malloc(32);
	password=(uint8 *)os_malloc(64);
	os_memset(&stationConf,0,sizeof(struct station_config));
	os_memcpy(ssid,"whoisLily",strlen("whoisLily"));
	os_memcpy(password,"4EC24182",strlen("4EC24182"));
	os_memcpy(stationConf.ssid,ssid,strlen("whoisLily"));
	os_memcpy(stationConf.password,password,strlen("4EC24182"));
//  os_memcpy(stationConf.ssid,"whoisLily",strlen("whoisLily"));
//  os_memcpy(stationConf.password,"4EC24182",strlen("4EC24182"));
	os_printf("wifi initialization\r\n");
	wifi_set_opmode_current(STATION_MODE);//设置wifi工作模式，station 模式 不保存到flash
	wifi_station_set_config_current(&stationConf);//设置wifi station接口的配置参数，并保存到flash
	os_timer_disarm(&wifilink);
	os_timer_setfn(&wifilink, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&wifilink, 1000, 0);
	wifi_station_connect();//wifi station接口连接AP
}

