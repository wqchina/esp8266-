/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "eagle_soc.h"
#include "ip_addr.h"
#include "espconn.h"
#include "ets_sys.h"
#include "os_type.h"
#include "user_interface.h"
//#include "mqtt.h"
#include "wifi.h"
//#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
//#include "sntp.h"
#include "c_types.h"
#include "hal_rgb_led.h"
#include "hw_timer.h"
#include "hal_temp_hum.h"
#include "http_sever.h"
#include "tcp_client.h"

uint8_t ds18b20temp;
uint8_t ds18b20humi;
ETSTimer timer;
extern bool is_connected;

void hw_test_timer_cb(void)
{
	system_os_post(0,0,0);//向任务发送消息
}
void test_task (os_event_t *e)
{
		switch (e->sig)
	{
			case 0:
				dh11Read(&ds18b20temp,&ds18b20humi);
//				ds18b20temp=ds18b20ReadTemp();
				os_printf("temp:%d\r\n",ds18b20temp);
				if(is_connected)//login success.
				{
					os_printf("-----\r\n");
					char buffer[255]={0};
					os_sprintf(buffer,"(mcastTermFormatState \"tmp\" %d)\n",ds18b20temp);//C3-6 终端向云端上报状态
					espconn_sent(&user_tcp_conn,buffer,strlen(buffer));
				}
				break;
			default:
				break;
	}
}

void delay_ms(uint32_t ms)
{
	for(;ms>0;ms--)
	{
		os_delay_us(1000);
	}
}

uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

//void wifi_callback(uint8 status,struct ip_info *ip,uint32 port)
//{
//	struct ip_info *local_ip_addr=ip;
//	uint32 port_addr=port;
//	if(status==STATION_GOT_IP)
//	{
//		sever_init(&local_ip_addr->ip,port_addr);
//	}
//}

void user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);//串口初始化
    os_event_t *testQueue;//消息队列指针
    is_connected=false;
    dh11Init();
    os_delay_us(1000);
    os_printf("\r\n==SDK version : %s==\r\n",system_get_sdk_version());
    testQueue=(os_event_t *)os_malloc(sizeof(os_event_t)*4);
    system_os_task(test_task,0,testQueue,4);//创建系统任务（任务函数，任务优先级，消息队列指针，消息队列深度）
	os_timer_disarm(&timer);//软件定时器
	os_timer_setfn(&timer,hw_test_timer_cb,NULL);//设置定时器回调函数（定时器结构，定时器回调函数，回调函数的参数）
	os_timer_arm(&timer,1000,TRUE);//使能ms定时器（定时器结构，定时时间，定时器是否重复）
    wifi_connect();
}
