/*
 *  bluetooth-dev-tool
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Contact:  Hocheol Seo <hocheol.seo@samsung.com>
 *           GirishAshok Joshi <girish.joshi@samsung.com>
 *           DoHyun Pyun <dh79.pyun@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <vconf.h>
#include <glib.h>

#ifdef DEBUG_EN
#define APP_DBG(format, args...)	printf("%s(), line[%d]: " format, __FUNCTION__, __LINE__, ##args)
#define APP_DEBUG(format, args...)	printf(format, ##args)
#else
#define APP_DBG(format, args...)
#define APP_DEBUG(format, args...)
#endif

#define	BD_ADDR_FILE	"/opt/etc/.bd_addr"
#define	PSKEY_TEMP_FILE	"/opt/etc/.bluetooth.psr"
#define PSR_FILE	"/opt/etc/bluetooth/bluetooth.psr"

#define	BD_ADDR_LEN	14
#define BD_PREFIX	"0002\n"

#define PSKEY_LEN	27
#define PSKEY_BDADDR_PREFIX	"&0001 = 0012 "

#define READ_BD_FILE_MAX	300

static int success_make_bt_address_from_imei=-1;
static GMainLoop * loop;
const char *DEFAULT_IMEI="004999010640000";
static gboolean is_default_imei=FALSE;

int addremoveBD(char* path, char* pskey){
	FILE *fd, *new;
	int ret;
	char cmp[READ_BD_FILE_MAX];
	char *result;

	fd = fopen(path, "r");
	if(NULL == fd){
		APP_DBG("Error open psr file\r\n");
		return -21;
	}

	new = fopen(PSKEY_TEMP_FILE, "w");
	if(NULL == new){
		APP_DBG("Error creat temp file\r\n");
		fclose(fd);
		return -22;
	}

	ret = fputs(pskey,new);

	while(1){
		result = fgets(cmp, READ_BD_FILE_MAX, fd);

		APP_DBG("PSR : [%s]\r\n", cmp);

		if((NULL != result) && (0 == strncmp(cmp, "&0001", 5))){
			APP_DBG("Find BD address set script\r\n");
			continue;
		}

		if(NULL == result){
			APP_DBG("EOF reaches\r\n");
			fclose(fd);
			fclose(new);
			return 1;
		}

		ret = fputs(cmp,new);
	}
	
	return 0;
}
void makeRandomBD(unsigned char* buf){
	int ran;
	int i;
	unsigned int seed;
	memcpy(buf, BD_PREFIX, 5);
	seed = time(NULL);
	for(i=5;i<14;i++){
		if(7==i){
			buf[i]='\n';
			continue;
		}
		ran = rand_r(&seed)%16;
		if(10>ran)
			ran+=0x30;
		else
			ran+=0x57;
		buf[i]=ran;
	}
	APP_DEBUG("Random number is\r\n");
	for(i=0;i<BD_ADDR_LEN;i++){
		APP_DEBUG("%c",buf[i]);
	}
	APP_DEBUG("\r\n");
}

int make_bt_address_from_tapi_imei(unsigned char * bt_address)
{
	char * temp=NULL;
	int tapi_state=0;
	int ret=-1;
	int i=0;

	if(bt_address==NULL)
		return -EBADR;

	ret=vconf_get_int(VCONFKEY_TELEPHONY_TAPI_STATE,&tapi_state);
	if(tapi_state==VCONFKEY_TELEPHONY_TAPI_STATE_READY && ret==0){
		temp=vconf_get_str(VCONFKEY_TELEPHONY_IMEI);
		APP_DEBUG("TAPI_IMEI: %s\n",temp);

		if(strcmp(temp,DEFAULT_IMEI)==0){
			APP_DEBUG("TAPI_IMEI is defulat IMEI\n");
			is_default_imei=TRUE;
			return -ENODATA;
		}

		if(strcmp(temp,"")==0)
			return -ENODATA;

		if(strlen(temp)<14)
			return -ENODATA;

		memcpy(bt_address, BD_PREFIX, 5);

		for(i=5 ;i<14;i++){
			if(i==7){
				bt_address[i]='\n';
				continue;
			}

			bt_address[i]=temp[i];
		}

	}else{
		APP_DEBUG("TAPI_IMEI Reading Error\n");
		return -ENODATA;
	}

	APP_DEBUG("Bluetooth Address\n");
	for(i=0;i<BD_ADDR_LEN;i++)
		APP_DEBUG("%c",bt_address[i]);

	APP_DEBUG("\n");

	return 0;

}

int make_bt_address(gboolean overwrite_bt_address)
{
	int fd;
	int i;
	unsigned char txt[BD_ADDR_LEN];
	unsigned char nap[4+1], uap[2+1], lap[6+1];
	char pskey[PSKEY_LEN+3];
	int ret;

	fd=open(BD_ADDR_FILE, O_RDONLY | O_SYNC);

	if(fd<0 || overwrite_bt_address==TRUE){
		if(fd<0)
			APP_DEBUG("File not exist\n");
		else
			close(fd);

		if(overwrite_bt_address)
		{
			APP_DEBUG("Overwrite BT address because TAPI write correct IMEI.\n");
		}

		fd=open(BD_ADDR_FILE, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0644);

		if (fd <0)
		{
			APP_DEBUG("Can't open address file\n");
			return 0;
		}
		success_make_bt_address_from_imei=make_bt_address_from_tapi_imei(txt);

		if(success_make_bt_address_from_imei<0)
			makeRandomBD(txt);

		ret = write(fd, txt, BD_ADDR_LEN);
		lseek(fd, 0, SEEK_SET);
	}else{
		APP_DEBUG("%s is already existed\n",BD_ADDR_FILE);
		success_make_bt_address_from_imei=0;
	}
		
	ret = read(fd, nap, 5);
	ret = read(fd, uap, 3);
	ret = read(fd, lap, 7);
	close(fd);

	return ret;
}

void vconf_cb(keynode_t *key, void * data)
{
	char * key_string=NULL;

	switch(vconf_keynode_get_type(key))
	{
		case VCONF_TYPE_STRING:
			key_string=vconf_keynode_get_str(key);
			if(strcmp(key_string,"")!=0)
			{
				APP_DEBUG("Vconf Call back trial\n");
				/* This case means TAPI writes IMEI correctly */
				/* Because we write BT address which comes from IMEI again  */
				make_bt_address(TRUE);
				g_main_loop_quit(loop);
			}

			break;


		default:
			break;
	}
	return;
}

gboolean exit_cb(gpointer data)
{

	APP_DEBUG("Time out!!!\n");
	g_main_loop_quit(loop);

	return FALSE;
}
int main()
{
	loop=g_main_loop_new(NULL,FALSE);
	APP_DEBUG("Bluetooth Address Setting\n");
	make_bt_address(FALSE);

	/* When we get proper BT address by IMEI or
	 * randome BT address due to default IMEI
	 * We don't need to wait for telephony activation, timeout */
	if(success_make_bt_address_from_imei==0 || is_default_imei==TRUE)
		exit(0);

	vconf_notify_key_changed(VCONFKEY_TELEPHONY_IMEI,vconf_cb,NULL);

	g_timeout_add_seconds(10,exit_cb,NULL);
	g_main_loop_run(loop);

	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_IMEI,vconf_cb);

	return 0;
}
