/*
 * http_sever.h
 *
 *  Created on: 2019年3月21日
 *      Author: xiamu
 */

#ifndef APP_INCLUDE_MODULES_HTTP_SEVER_H_
#define APP_INCLUDE_MODULES_HTTP_SEVER_H_
#include "wifi.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "hal_rgb_led.h"

#define URLSize 125
#define HTML_SIZE 531
//#define TMP_SIZE 88
#define tempjson "{\"success\": {\"value1\": \"%d\",\"value2\": \"%d\"}}"

typedef enum ProtocolType
{
	GET=0,
	POST
}ProtocolType;

typedef struct URLFrame
		{
			ProtocolType protocoltype;
			char pFilename[URLSize];
		}URLFrame;

void sever_init(struct ip_addr *,uint32);
void server_listen(void *arg);
void server_recon(void *arg,sint8 err);
void server_recv(void *arg, char *pdata, unsigned short len);
void server_sent(void *arg);
void server_discon(void *arg);

#endif /* APP_INCLUDE_MODULES_HTTP_SEVER_H_ */
