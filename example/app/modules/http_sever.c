/*
 * http_sever.c
 *
 *  Created on: 2019年3月21日
 *  Author: xiamu
 *	html网页通过烧写的方式进去 tmp_html通过data_send直接发送出去
 */
#include "http_sever.h"

#define n_data_flash_sec 0x100
//#define tmp_data_flash_sec 0x101 //html数组
#define FLASH_SEC_SIZE 4096

extern uint8_t ds18b20temp;
extern uint8_t ds18b20humi;

char html[]="\
<html xmlns=\"http://www.w3.org/1999/xhtml\">\
<head>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\
<title></title>\
</head>\
<body>\
<form action=\"tmp.php\" method=\"post\" enctype=\"application/x-www-form-urlencoded\">\
<br>\
<br>\
<br>\
<center>\
    SSID:\
    <input name=\"SSID\" type=\"text\" />\
</center>\
<br>\
<center>\
    PASSWORD:\
    <input name=\"PASSWORD\" type=\"password\" />\
</center>\
<br>\
<center>\
<input name=\"Submit\" type=\"submit\" value=\"Submit\"/>\
</center>\
</form>\
</body>\
</html>\
";

char tmp_html[]="\
<html xmlns=\"http://www.w3.org/1999/xhtml\">\
<head>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\
<title></title>\
</head>\
<body>\
<p><b>URL:</b>\
<span id=\"A0\"></span>\
</p>\
<p><b>Status:</b>\
<span id=\"A1\"></span>\
</p>\
<p><b>Status text:</b>\
<span id=\"A2\"></span>\
</p>\
<p><b>Temperature:</b>\
<span id=\"A3\"></span>\
</p>\
<p><b>Humidity:</b>\
<span id=\"A4\"></span>\
</p>\
<script type=\"text/javascript\">\
var xmlhttp=null;\
var host=location.hostname;\
var URL=\"http://\"+host+\"/txt.php\";\
if (window.XMLHttpRequest)\
{\
  xmlhttp=new XMLHttpRequest();\
}\
else if (window.ActiveXObject)\
{\
  xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");\
}\
if(xmlhttp==null)\
{\
  alert(\"Your browser does not support XMLHTTP.\");\
}\
window.setInterval(\"loadXMLDoc(URL)\",3000);\
function loadXMLDoc(url)\
{\
  if (xmlhttp!=null)\
{\
  xmlhttp.onreadystatechange=state_Change;\
  xmlhttp.open(\"GET\",url,true);\
  xmlhttp.send(null);\
}\
}\
function state_Change()\
{\
if (xmlhttp.readyState==4)\
  {\
  if (xmlhttp.status==200)\
    {\
	var b=xmlhttp.responseText;\
	var myobject=eval(\"(\"+b+\")\");\
	document.getElementById('A0').innerHTML=URL;\
    document.getElementById('A1').innerHTML=xmlhttp.status;\
    document.getElementById('A2').innerHTML=xmlhttp.statusText;\
    document.getElementById('A3').innerHTML=myobject.success.value2;\
    document.getElementById('A4').innerHTML=myobject.success.value1;\
    }\
  else\
    {\
    }\
  }\
}\
</script>\
</body>\
</html>\
";

static write_data1()
{
	spi_flash_erase_sector(n_data_flash_sec);
	spi_flash_write((uint32_t)(n_data_flash_sec*FLASH_SEC_SIZE),(uint32*)html,sizeof(html));
}

//static write_data2()
//{
//	spi_flash_erase_sector(tmp_data_flash_sec);
//	spi_flash_write((uint32)(tmp_data_flash_sec*FLASH_SEC_SIZE),(uint32*)tmp_html,sizeof(tmp_html));
//}

LOCAL bool ICACHE_FLASH_ATTR
parse_url(char *data,URLFrame *purlframe)
{
	char *pdata=NULL;
	char *str=NULL;
	uint8_t length=0;

	if(data==NULL || purlframe==NULL)
	{
		os_printf("parse_url error \n");
		return FALSE;
	}

	pdata=(char *)os_strstr(data,"Host: ");
	if(pdata!=NULL)
	{
		os_memset(purlframe->pFilename,0,URLSize);
		if(os_strncmp(data,"GET ",4)==0)
		{
			purlframe->protocoltype=GET;
			pdata=data+4;
		}
		else if(os_strncmp(data,"POST ",5)==0)
		{
			purlframe->protocoltype=POST;
			pdata=data+5;
		}
		else
		{
			os_printf("cant find GET or POST \n");
			return FALSE;
		}
		pdata++;
		str=(char *)os_strstr(data," HTTP");
		if(str==NULL)
		{
			os_printf("cant find HTTP \n");
		}
		else
		{
			length=str-pdata;
			os_memcpy(purlframe->pFilename,pdata,length);
		}
	}
	else
	{
		os_printf("cant find Host \n");
		return FALSE;
	}

}

LOCAL void ICACHE_FLASH_ATTR
data_send(void *arg,bool responce,char *psend)
{
	struct espconn *ptrespconn =(struct espconn *)arg;
	char *pbuf=NULL;
	char httphead[512]={0};//头部
	uint16 length=0;
	os_memset(httphead,0,sizeof(httphead));
	if(responce)
	{
		os_sprintf(httphead,"HTTP/1.1 200 OK\r\n\
Content-Length: %d\r\nServer: lwip/1.4.0\r\n\n",psend ?os_strlen(psend):0);
		if(psend)
		{
//			os_printf("%s\n",psend);
//			os_sprintf(httphead+os_strlen(httphead),"Content-Type: text/html; charset=utf-8\r\nPragma: no-cache\r\n\r\n");
			length=os_strlen(httphead)+os_strlen(psend);
//			os_printf("httphead length:%d\n",os_strlen(httphead));
//			os_printf("psend length:%d\n",os_strlen(psend));
//			os_printf("length:%d\n",length);
			pbuf=(char *)os_malloc(length);//存头部和网页文本数据
			os_memset(pbuf,0,os_strlen(pbuf));
			os_memcpy(pbuf,httphead,os_strlen(httphead));
			os_memcpy(pbuf+os_strlen(httphead),psend,os_strlen(psend));
//			os_printf("%s\n",pbuf);
		}
		else
		{
			os_sprintf(httphead+os_strlen(httphead),"\r\n");
			length=os_strlen(httphead);
		}
	}
	else
	{
		os_sprintf(httphead,"HTTP/1.1 400 BadRequest\r\n\
Content-Length: 0\r\nServer: lwIP/1.4.0\r\n\n");
		length=os_strlen(httphead);
	}
	if(psend)
	{
		espconn_sent(ptrespconn,pbuf,length);
	}
	else
	{
		espconn_sent(ptrespconn,httphead,length);
	}
}



void server_recv(void *arg, char *pdata, unsigned short len)
{
	struct espconn *esp=(struct espconn *)arg;
	char index1[4096];//存网页文本数据
	char data[50]={0};
//	char index2[4096];

	URLFrame *urlframe=NULL;
	urlframe=(URLFrame *)os_malloc(sizeof(URLFrame));

	char *precvdata=(char *)os_zalloc(len);
	os_memcpy(precvdata,pdata,len);
	parse_url(pdata,urlframe);

//	write_data1();	//将网页文本数据写入用户数据区
//	write_data2();
	spi_flash_read((uint32)(n_data_flash_sec*FLASH_SEC_SIZE),(uint32*)index1,4096);//读网页文本数据到index数组中
//	spi_flash_read((uint32)(tmp_data_flash_sec*FLASH_SEC_SIZE),(uint32*)index2,4096);
	index1[HTML_SIZE]=0;
//	index2[TMP_SIZE]=0;

	if(urlframe->protocoltype==GET)
	{
		os_printf("%s \n",pdata);
		if(strncmp(urlframe->pFilename,"txt.php",7)==0)
		{
			os_sprintf(data,tempjson,ds18b20humi,ds18b20temp);
			data_send(arg,true,data);
		}
		else
		{
			data_send(arg,true,index1);	//发送登陆界面
		}
	}
	else if(urlframe->protocoltype==POST)
	{
		char *ssid=NULL;
		char *password=NULL;
		char SSID[5]={0};
		char PASSWORD[6]={0};
		os_printf("%s \n",pdata);
		ssid=(char *)os_strstr(precvdata,"SSID=");
		ssid+=5;
		password=(char *)os_strstr(ssid,"&");
		os_memcpy(SSID,ssid,password-ssid);
		ssid=(char *)os_strstr(precvdata,"PASSWORD=");
		ssid+=9;
		password=(char *)os_strstr(ssid,"&");
		os_memcpy(PASSWORD,ssid,password-ssid);
		if(os_strncmp(SSID,"admin",5)==0 && os_strncmp(PASSWORD,"public",6)==0)
		{
//			os_printf("Ok TRUE \n");
			if(strncmp(urlframe->pFilename,"tmp.php",7)==0)
			{
				data_send(arg,true,tmp_html);
//				data_send(arg,true,tmp_html);
//				os_printf("tempreture is %d \n",ds18b20temp);
//				os_printf("humidity is %d \n",ds18b20humi);
//				data_send(arg,true,tmp_html);
			}
		}
		else
		{
				os_printf("wrong ssid or wrong password \n");
				data_send(arg,true,index1);
		}
	}
	os_free(urlframe);
	urlframe=NULL;
	os_free(precvdata);
	precvdata=NULL;
//	os_printf("%s \n",index);
//	os_printf("received message\n");
//	os_printf("%s \n",pdata);
//	data_send(arg,true,index);
}


void server_sent(void *arg)
{
	os_printf("sent normally \n");
}


void server_discon(void *arg)
{
	os_printf("disconnect normally \n");
}

void server_listen(void *arg)
{
	struct espconn *esp=(struct espconn *)arg;
	espconn_regist_recvcb(esp,server_recv);	//注册接收成功的回调函数
	espconn_regist_sentcb(esp,server_sent);	//注册发送成功的回调函数
	espconn_regist_disconcb(esp,server_discon);	//注册断开连接的回调函数
}

void server_recon(void *arg,sint8 err)
{
	os_printf("disconnect unusually,error code:%d \n",err);
}

void sever_init(struct ip_addr *local_ip,uint32 port)
{
	LOCAL struct espconn esp_conn;	//网络传输的结构体
	esp_conn.type=ESPCONN_TCP;		//TCP
	esp_conn.state=ESPCONN_NONE;	//当前连接状态
	esp_conn.proto.tcp=(esp_tcp *)os_malloc(sizeof(esp_tcp));//分配TCP结构体内存
	os_memcpy(esp_conn.proto.tcp->local_ip,local_ip,4);//http服务器IP地址
	esp_conn.proto.tcp->local_port=port;//端口号80

	espconn_regist_connectcb(&esp_conn,server_listen);//注册TCP连接成功建立后的回调函数
	espconn_regist_reconcb(&esp_conn,server_recon);//注册TCP异常断开时回调函数

	espconn_accept(&esp_conn);//创建TCP sever，建立侦听
}

