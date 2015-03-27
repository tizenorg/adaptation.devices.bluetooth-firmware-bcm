/*
 * Name: bcmtool_4330b1.c
 *
 * Description: Download a patchram files for the HCD format
 *
 * Copyright (c) 2012-2013, Broadcom Corp., All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <limits.h>

#define N_HCI	15
#define HCI_UART_H4		0
#define HCI_UART_BCSP	1
#define HCI_UART_3WIRE	2
#define HCI_UART_H4DS	3
#define HCI_UART_LL		4
#define HCIUARTSETPROTO		_IOW('U', 200, int)
#define HCIUARTGETPROTO		_IOR('U', 201, int)
#define HCIUARTGETDEVICE	_IOR('U', 202, int)

/* Pre baudrate change for fast download */
#define HIGH_SPEED_PATCHRAM_DOWNLOAD  TRUE

/* Host Stack Idle Threshold */
#define HCILP_IDLE_THRESHOLD          0x01

/* Host Controller Idle Threshold */
#define HCILP_HC_IDLE_THRESHOLD       0x01

/* BT_WAKE Polarity - 0=Active Low, 1= Active High */
#define HCILP_BT_WAKE_POLARITY        1

/* HOST_WAKE Polarity - 0=Active Low, 1= Active High */
#define HCILP_HOST_WAKE_POLARITY      1

/* Local Feature */
#define BCM_DISABLE_RF_PWRCTRL        FALSE

#define RELEASE_DATE "2011.02.07"
#define DEBUG 1

/*  Broadcom AXI patch for BCM4335 chipset only */
#define DEPLOY_4335A_AXI_BRIDGE_PATCH TRUE

/* The fix for AXI bridge contention between BT and WLAN:
 *
 *  Set this TRUE only when
 *      1. the platform is using BCM4335A or BCM4335B0, and
 *      2. Kernel source has implemented AXI BRIDGE lock logic.
 */
#ifndef DEPLOY_4335A_AXI_BRIDGE_PATCH
#define DEPLOY_4335A_AXI_BRIDGE_PATCH   FALSE
#endif

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long UINT32;
typedef signed long INT32;
typedef signed char INT8;
typedef signed short INT16;
typedef unsigned char BOOLEAN;

#define FALSE    0
#define TRUE     (!FALSE)

#define BD_ADDR_LEN     6	/* Device address length */
typedef UINT8 BD_ADDR[BD_ADDR_LEN];	/* Device address */

#define HCI_GRP_LINK_CONTROL_CMDS                 (0x01 << 10)
#define HCI_GRP_LINK_POLICY_CMDS                  (0x02 << 10)
#define HCI_GRP_HOST_CONT_BASEBAND_CMDS           (0x03 << 10)
#define HCI_GRP_INFORMATIONAL_PARAMS              (0x04 << 10)
#define HCI_GRP_STATUS_PARAMS                     (0x05 << 10)
#define HCI_GRP_TESTING_CMDS                      (0x06 << 10)
#define HCI_GRP_L2CAP_CMDS                        (0x07 << 10)
#define HCI_GRP_L2CAP_HCI_EVTS                    (0x08 << 10)
#define HCI_GRP_VENDOR_SPECIFIC                   (0x3F << 10)

#define HCI_RESET                                 (0x0003 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_SET_EVENT_FILTER                      (0x0005 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_READ_LOCAL_NAME                       (0x0014 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_READ_SCAN_ENABLE                      (0x0019 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_WRITE_SCAN_ENABLE                     (0x001A | HCI_GRP_HOST_CONT_BASEBAND_CMDS)

#define HCI_READ_LOCAL_VERSION_INFO               (0x0001 | HCI_GRP_INFORMATIONAL_PARAMS)
#define HCI_READ_LOCAL_FEATURES                   (0x0003 | HCI_GRP_INFORMATIONAL_PARAMS)

#define HCI_ENABLE_DEV_UNDER_TEST_MODE            (0x0003 | HCI_GRP_TESTING_CMDS)

#define HCI_BRCM_SUPER_PEEK_POKE                  (0x000A | HCI_GRP_VENDOR_SPECIFIC)
#define VSC_WRITE_BD_ADDR                         (0x0001 | HCI_GRP_VENDOR_SPECIFIC)
#define VSC_HCI_CMD_SET_LOC_FEATURES_CMD          (0x000B | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_BRCM_UPDATE_BAUDRATE_CMD              (0x0018 | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_BRCM_WRITE_SCO_PCM_INT_PARAM          (0x001C | HCI_GRP_VENDOR_SPECIFIC)
#define VSC_WRITE_PCM_DATA_FORMAT_PARAM           (0x001E | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_BRCM_WRITE_SLEEP_MODE                 (0x0027 | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_BRCM_DOWNLOAD_MINI_DRV                (0x002E | HCI_GRP_VENDOR_SPECIFIC)
#define VSC_WRITE_UART_CLOCK_SETTING              (0x0045 | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_VSC_WRITE_RAM                         (0x004C | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_VSC_LAUNCH_RAM                        (0x004E | HCI_GRP_VENDOR_SPECIFIC)

#define VOICE_SETTING_MU_LAW_MD                   0x0100
#define VOICE_SETTING_LINEAR_MD                   0x0060

#define HCI_ARM_MEM_PEEK                          0x04
#define HCI_ARM_MEM_POKE                          0x05

#define BTUI_MAX_STRING_LENGTH_PER_LINE           255
#define HCI_BRCM_WRITE_SLEEP_MODE_LENGTH          10

#define HCI_BRCM_UPDATE_BAUD_RATE_ENCODED_LENGTH    0x02
#define HCI_BRCM_UPDATE_BAUD_RATE_UNENCODED_LENGTH  0x06

#define VSC_WRITE_UART_CLOCK_SETTING_LEN          1

/* print string with time stamp */
#define TDEBUG0(m)                               if(debug_mode) {print_time();fprintf(stderr,m);}
#define TDEBUG1(m,n1)                            if(debug_mode) {print_time();fprintf(stderr,m,n1);}
#define TDEBUG2(m,n1,n2)                         if(debug_mode) {print_time();fprintf(stderr,m,n1,n2);}
#define TDEBUG3(m,n1,n2,n3)                      if(debug_mode) {print_time();fprintf(stderr,m,n1,n2,n3);}
#define TDEBUG4(m,n1,n2,n3,n4)                   if(debug_mode) {print_time();fprintf(stderr,m,n1,n2,n3,n4);}
#define TDEBUG5(m,n1,n2,n3,n4,n5)                if(debug_mode) {print_time();fprintf(stderr,m,n1,n2,n3,n4,n5);}
#define TDEBUG6(m,n1,n2,n3,n4,n5,n6)             if(debug_mode) {print_time();fprintf(stderr,m,n1,n2,n3,n4,n5,n6);}

/* print just string */
#define DEBUG0(m)                               if(debug_mode) {fprintf(stderr,m);}
#define DEBUG1(m,n1)                            if(debug_mode) {fprintf(stderr,m,n1);}
#define DEBUG2(m,n1,n2)                         if(debug_mode) {fprintf(stderr,m,n1,n2);}
#define DEBUG3(m,n1,n2,n3)                      if(debug_mode) {fprintf(stderr,m,n1,n2,n3);}
#define DEBUG4(m,n1,n2,n3,n4)                   if(debug_mode) {fprintf(stderr,m,n1,n2,n3,n4);}
#define DEBUG5(m,n1,n2,n3,n4,n5)                if(debug_mode) {fprintf(stderr,m,n1,n2,n3,n4,n5);}
#define DEBUG6(m,n1,n2,n3,n4,n5,n6)             if(debug_mode) {fprintf(stderr,m,n1,n2,n3,n4,n5,n6);}

#define STREAM_TO_UINT8(u8, p)   {u8 = (UINT8)(*(p)); (p) += 1;}
#define STREAM_TO_UINT16(u16, p) {u16 = ((UINT16)(*(p)) + (((UINT16)(*((p) + 1))) << 8)); (p) += 2;}
#define STREAM_TO_UINT32(u32, p) {u32 = (((UINT32)(*(p))) + ((((UINT32)(*((p) + 1)))) << 8) + ((((UINT32)(*((p) + 2)))) << 16) + ((((UINT32)(*((p) + 3)))) << 24)); (p) += 4;}

#define ROTATE_BD_ADDR(p1, p2)    \
									do		   \
									{							\
										p1[0] = p2[5];		\
										p1[1] = p2[4];		\
										p1[2] = p2[3];		\
										p1[3] = p2[2];		\
										p1[4] = p2[1];		\
										p1[5] = p2[0];		\
									} while (0)

UINT8 vsc_for_pcm_config[5] = { 0x00, 0x00, 0x03, 0x03, 0x00 };

/*
                    Byte1 -- 0 for MSb first
                    Byte2 -- 0 Fill value
                    Byte3 -- 1 Fill option (0:0's, 1:1's , 2:Signed, 3:Programmable)
                    Byte4 -- 1 Number of fill bits
                    Byte5 -- 1 Right justified (0 for left justified)
*/

UINT8 vsc_for_sco_pcm[5] = { 0x00, 0x01, 0x00, 0x01, 0x01 };

/*
                    Neverland : PCM, 256, short, master ,master
                    Volance   : PCM, 256, short, master ,master

                    Byte1 -- 0 for PCM 1 for UART or USB
                    Byte2 -- 0 : 128, 1: 256, 2:512, 3:1024, 4:2048 Khz
                    Byte3 -- 0 for short frame sync 1 for long frame sync
                    Byte4 -- 0 Clock direction 0 for same as sync 1 for opposite direction
                    Byte5 -- 0 for slave 1 for master
*/

int fd;				/* HCI handle */

BOOLEAN debug_mode = FALSE;	/* Debug Mode Enable */

BOOLEAN use_two_stop_bits = FALSE;     /* Flag of two stop bits for tty */

unsigned char buffer[1024];

struct termios termios;

void ChangeBaudRate(UINT32 baudrate);

void exit_err(UINT8 err)
{
#if ( HIGH_SPEED_PATCHRAM_DOWNLOAD == TRUE )
	ChangeBaudRate(115200);
#endif
	exit(err);
}

void print_time(void)
{
#if 0
	struct timespec tp;
	int rs;

	rs = clock_gettime(CLOCK_REALTIME, &tp);
	fprintf(stderr, "[%04d : %06d]\n", tp.tv_sec, tp.tv_nsec / 1000);
	return;
#endif
}

void dump(unsigned char *out, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (!(i % 16)) {
			DEBUG0("\n");
		}
		DEBUG1("%02x ", out[i]);
	}
	DEBUG0("\n\n");
}

UINT8 SendCommand(UINT16 opcode, UINT8 param_len, UINT8 * p_param_buf)
{
	UINT8 pbuf[255] = { 0, };
	UINT8 i = 0;

	pbuf[0] = 0x1;
	pbuf[1] = (UINT8) (opcode);
	pbuf[2] = (UINT8) (opcode >> 8);
	pbuf[3] = param_len;

	for (i = 0; i < param_len; i++) {
		pbuf[i + 4] = *p_param_buf++;
	}

	DEBUG1("Send %d", param_len + 4);

	dump(pbuf, param_len + 4);

	write(fd, pbuf, param_len + 4);
	return 0;
}

void expired(int sig)
{
	static UINT8 count = 0;
	DEBUG0("expired try again\n");
	SendCommand(HCI_RESET, 0, NULL);
	alarm(1);
	count++;

	if (count > 3) {
		fprintf(stderr, "[ERR] HCI reset time expired\n");
		exit(1);
	}
}

void read_event(int fd, unsigned char *buffer)
{
	int i = 0;
	int len = 3;
	int count;

	while ((count = read(fd, &buffer[i], len)) < len) {
		i += count;
		len -= count;
	}

	i += count;
	len = buffer[2];

	while ((count = read(fd, &buffer[i], len)) < len) {
		i += count;
		len -= count;
	}

#ifdef DEBUG
	count += i;

	DEBUG1("\nreceived %d", count);
	dump(buffer, count);
#endif
}

INT32 filesize(char *name)
{
	INT32 size;
	int flag;
	struct stat buf;

	flag = stat(name, &buf);
	if (flag == -1)
		return -1;

	size = buf.st_size;
	return (size);
}

void DisplayProgress(int total, int val)
{
#if 0
#define PROGRESS_NUM 20

	int p;
	int i;
	char text[PROGRESS_NUM + 2] = { 0, };

	text[0] = '[';
	text[PROGRESS_NUM + 1] = ']';
	p = (val * PROGRESS_NUM) / total;

	for (i = 1; i <= p; i++) {
		text[i] = '=';
	}

	for (i = p + 1; i <= PROGRESS_NUM; i++) {
		text[i] = ' ';
	}

	for (i = 0; i <= (PROGRESS_NUM + 1); i++) {
		fprintf(stderr, "%c", text[i]);
	}

	if (p >= PROGRESS_NUM)
		fprintf(stderr, "  %6d/%6d\n", val, total);
	else
		fprintf(stderr, "  %6d/%6d\r", val, total);
#else
	if (val == total)
		fprintf(stderr, " %6d/%6d\n", val, total);
	else
		fprintf(stderr, " %6d/%6d\r", val, total);
#endif
}

#if (DEPLOY_4335A_AXI_BRIDGE_PATCH == TRUE)
/* 4335A/B0 AXI BRIDEG lock cookie */
struct btlock {
    int lock;
    int cookie;
};

#define AXI_LOCK_COOKIE ('B' | 'T'<<8 | '3'<<16 | '5'<<24)  /* BT35 */
#define AXI_LOCK_FS_NODE "/dev/btlock"

static const UINT8 bcm4335a_axi_patch_addr[4] =
{
    0x00, 0x02, 0x0d, 0x00
};

static const UINT8 bcm4335a_axi_patch[] =
{
    0x00, 0x02, 0x0d, 0x00,		/* bcm4335a_axi_patch_addr */
    0x70, 0xb5, 0x0c, 0x49, 0x4c, 0xf6, 0x20, 0x30,
    0x8a, 0xf7, 0xbb, 0xf9, 0x01, 0x28, 0x0d, 0xd1,
    0x8a, 0xf7, 0x80, 0xf9, 0x08, 0xb1, 0x04, 0x25,
    0x00, 0xe0, 0x05, 0x25, 0x00, 0x24, 0x03, 0xe0,
    0x8a, 0xf7, 0x74, 0xf9, 0x64, 0x1c, 0xe4, 0xb2,
    0xac, 0x42, 0xf9, 0xd3, 0xbd, 0xe8, 0x70, 0x40,
    0x8a, 0xf7, 0x7f, 0xb9, 0xb0, 0x9b, 0x04, 0x00
};

/*******************************************************************************
**
** Function         hw_4335_dl_axi_patch
**
** Description      Download 4335Ax/4335B0 AXI BRIDGE patch
**
** Returns          TRUE, if fw patch is sent
**                  FALSE, otherwise
**
*******************************************************************************/
static UINT8 hw_4335_dl_axi_patch(void)
{
	SendCommand(HCI_VSC_WRITE_RAM, sizeof(bcm4335a_axi_patch),
					(UINT8 *) bcm4335a_axi_patch);
	read_event(fd, buffer);

	DEBUG0("hw_4335_dl_axi_patch downloading done");

	SendCommand(HCI_VSC_LAUNCH_RAM, sizeof(bcm4335a_axi_patch_addr),
					(UINT8 *) bcm4335a_axi_patch_addr);
	read_event(fd, buffer);

	DEBUG0("hw_4335_dl_axi_patch launching done");

	return TRUE;
}

/*******************************************************************************
**
** Function         hw_4335_release_axi_bridge_lock
**
** Description      Notify kernel to release the AXI BRIDGE lock which was
**                  acquired earlier in rfkill driver when powering on BT
**                  Controller
**
** Returns          None
**
*******************************************************************************/
void hw_4335_axi_bridge_lock(void)
{
    int fd, ret;
    struct btlock lock;

    lock.cookie = AXI_LOCK_COOKIE;
    lock.lock = 1;

    fd = open(AXI_LOCK_FS_NODE, O_RDWR);
    if (fd >= 0)
    {
        ret = write(fd, &lock, sizeof(lock));
        DEBUG0("4335 AXI BRIDGE lock");
        close(fd);
    }
    else
    {
        DEBUG1("Failed to unlock AXI LOCK -- can't open %s", AXI_LOCK_FS_NODE);
    }
}

/*******************************************************************************
**
** Function         hw_4335_release_axi_bridge_lock
**
** Description      Notify kernel to release the AXI BRIDGE lock which was
**                  acquired earlier in rfkill driver when powering on BT
**                  Controller
**
** Returns          None
**
*******************************************************************************/
void hw_4335_release_axi_bridge_lock(void)
{
    int fd, ret;
    struct btlock lock;

    lock.cookie = AXI_LOCK_COOKIE;
    lock.lock = 0;

    fd = open(AXI_LOCK_FS_NODE, O_RDWR);
    if (fd >= 0)
    {
        ret = write(fd, &lock, sizeof(lock));
        DEBUG0("Releasing 4335 AXI BRIDGE lock");
        close(fd);
    }
    else
    {
        DEBUG1("Failed to unlock AXI LOCK -- can't open %s", AXI_LOCK_FS_NODE);
    }
}
#endif 	/* (DEPLOY_4335A_AXI_BRIDGE_PATCH == TRUE) */

UINT8 DownloadPatchram(char *patchram1)
{
	UINT32 len;
	char prm[128] = { 0, };
	FILE *pFile = NULL;

	INT32 FileSize = 0;
	INT32 SentSize = 0;

	DEBUG1("\n%s\n", patchram1);

	/* HCI reset */
	DEBUG0("HCI reset\n");
	SendCommand(HCI_RESET, 0, NULL);
	alarm(1);
	read_event(fd, buffer);
	alarm(0);

#if (DEPLOY_4335A_AXI_BRIDGE_PATCH == TRUE)
	char *p_tmp;

	SendCommand(HCI_READ_LOCAL_NAME, 0, NULL);
	read_event(fd, buffer);

	p_tmp = strstr((char *)(buffer + 7), "BCM4335");
	DEBUG1( "chip_name [%s]\n", p_tmp);

	if ((p_tmp != NULL) &&
	    ((p_tmp[7] == 'A') /* 4335A */||
	     ((p_tmp[7] == 'B') && (p_tmp[8] == '0')) /* 4335B0 */)) {
		hw_4335_dl_axi_patch();
	}
	hw_4335_release_axi_bridge_lock();
#endif

#if ( HIGH_SPEED_PATCHRAM_DOWNLOAD == TRUE )
	ChangeBaudRate(921600);
#endif

	strncpy(prm, patchram1, 127);

	fprintf(stderr, "Download Start\n");

	if ((pFile = fopen(prm, "r")) == NULL) {
		fprintf(stderr, "file %s could not be opened, error %d\n", prm,
			errno);
		exit_err(1);
	}
	FileSize = filesize(prm);

	SendCommand(HCI_BRCM_DOWNLOAD_MINI_DRV, 0, NULL);
	read_event(fd, buffer);

	usleep(50000);

	while (fread(&buffer[1], sizeof(UINT8), 3, pFile)) {
		buffer[0] = 0x01;

		len = buffer[3];

		fread(&buffer[4], sizeof(UINT8), len, pFile);

		write(fd, buffer, len + 4);

		/* dispaly progress */
		SentSize += (len + 3);
#if 0
		DisplayProgress(FileSize, SentSize);
#endif
		/* dispaly progress */

		read_event(fd, buffer);

	}
	fclose(pFile);

	usleep(100000);		/*100ms delay */

	tcflush(fd, TCIOFLUSH);
	tcgetattr(fd, &termios);
	cfmakeraw(&termios);
	termios.c_cflag |= CRTSCTS;

	if (use_two_stop_bits)
		termios.c_cflag |= CSTOPB;

	tcsetattr(fd, TCSANOW, &termios);
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &termios);
	tcflush(fd, TCIOFLUSH);
	tcflush(fd, TCIOFLUSH);
	cfsetospeed(&termios, B115200);
	cfsetispeed(&termios, B115200);
	tcsetattr(fd, TCSANOW, &termios);

	/* Send HCI_RESET Command and process event */
	DEBUG0("HCI reset\n");
	SendCommand(HCI_RESET, 0, NULL);
	alarm(1);
	read_event(fd, buffer);
	alarm(0);
	fprintf(stderr, "Download Complete\n");

	return 0;
}

void SetScanEnable(void)
{
	UINT8 scan_data[1];

	/* 0x00: No scan enabled                            */
	/* 0x01: Inquiry scan enabled  | Page scan disabled */
	/* 0x02: Inquiry scan disabled | Page scan enabled  */
	/* 0x03: Inquiry scan enabled  | Page scan enabled  */

	scan_data[0] = 0x03;
	SendCommand(HCI_WRITE_SCAN_ENABLE, 1, &scan_data[0]);
	read_event(fd, buffer);
}

void SetAudio(void)
{
	fprintf(stderr, "Write Audio parameter\n");

	DEBUG5("vsc_for_sco_pcm = {%d,%d,%d,%d,%d}\n", vsc_for_sco_pcm[0],
	       vsc_for_sco_pcm[1], vsc_for_sco_pcm[2],
	       vsc_for_sco_pcm[3], vsc_for_sco_pcm[4]);

	SendCommand(HCI_BRCM_WRITE_SCO_PCM_INT_PARAM, 5,
		    (UINT8 *) vsc_for_sco_pcm);
	read_event(fd, buffer);

	DEBUG5("vsc_for_pcm_config = {%d,%d,%d,%d,%d}\n", vsc_for_pcm_config[0],
	       vsc_for_pcm_config[1], vsc_for_pcm_config[2],
	       vsc_for_pcm_config[3], vsc_for_pcm_config[4]);

	SendCommand(VSC_WRITE_PCM_DATA_FORMAT_PARAM, 5,
		    (UINT8 *) vsc_for_pcm_config);
	read_event(fd, buffer);
}

void SetPcmConf(UINT8 p0, UINT8 p1, UINT8 p2, UINT8 p3, UINT8 p4)
{
	vsc_for_pcm_config[0] = p0;
	vsc_for_pcm_config[1] = p1;
	vsc_for_pcm_config[2] = p2;
	vsc_for_pcm_config[3] = p3;
	vsc_for_pcm_config[4] = p4;
}

void SetScoConf(UINT8 p0, UINT8 p1, UINT8 p2, UINT8 p3, UINT8 p4)
{
	vsc_for_sco_pcm[0] = p0;
	vsc_for_sco_pcm[1] = p1;
	vsc_for_sco_pcm[2] = p2;
	vsc_for_sco_pcm[3] = p3;
	vsc_for_sco_pcm[4] = p4;
}

void HCILP_Enable(BOOLEAN on)
{
	fprintf(stderr, "Set Low Power mode %d\n", on);
	UINT8 data[HCI_BRCM_WRITE_SLEEP_MODE_LENGTH] = {
		0x01,		/* Sleep Mode algorithm 1 */
		HCILP_IDLE_THRESHOLD,	/* Host Idle Treshold in 300ms */
		HCILP_HC_IDLE_THRESHOLD,	/* Host Controller Idle Treshold in 300ms *//* this should be less than scan interval. */
		HCILP_BT_WAKE_POLARITY,	/* BT_WAKE Polarity - 0=Active Low, 1= Active High */
		HCILP_HOST_WAKE_POLARITY,	/* HOST_WAKE Polarity - 0=Active Low, 1= Active High */
		0x01,		/* Allow host Sleep during SCO */
		0x01,		/* Combine Sleep Mode and LPM - The device will not sleep in mode 0 if this flag is set to 1, */
		0x00,		/* UART_TXD Tri-State : 0x00 = Do not tri-state UART_TXD in sleep mode */
		0x00,		/* NA to Mode 1 */
		0x00,		/* NA to Mode 1 */
	};

	if (on) {
		data[0] = 0x01;
	} else {
		data[0] = 0x00;
	}

	SendCommand(HCI_BRCM_WRITE_SLEEP_MODE, HCI_BRCM_WRITE_SLEEP_MODE_LENGTH,
		    (UINT8 *) data);
	read_event(fd, buffer);
}

UINT32 uart_speed(UINT32 Speed)
{
	switch (Speed) {
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	case 921600:
		return B921600;
	case 1000000:
		return B1000000;
	case 1500000:
		return B1500000;
	case 2000000:
		return B2000000;
	case 2500000:
		return B2500000;
	case 3000000:
		return B3000000;
	case 4000000:
		return B4000000;
	default:
		return B115200;
	}
}

void ChangeBaudRate(UINT32 baudrate)
{
	UINT8 hci_data[HCI_BRCM_UPDATE_BAUD_RATE_UNENCODED_LENGTH] =
	    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	UINT8 uart_clock_24 = 0x2;	/* 0x1 - UART Clock 48MHz, 0x2 - UART Clock 24MHz */
	UINT8 uart_clock_48 = 0x1;	/* 0x1 - UART Clock 48MHz, 0x2 - UART Clock 24MHz */

	switch (baudrate) {
	case 115200:
	case 230400:
	case 460800:
	case 921600:
	case 1000000:
	case 1500000:
	case 2000000:
	case 2500000:
		/* Write UART Clock setting of 24MHz */
		DEBUG0("Change UART_CLOCK 24Mhz\n");
		SendCommand(VSC_WRITE_UART_CLOCK_SETTING,
			    VSC_WRITE_UART_CLOCK_SETTING_LEN,
			    (UINT8 *) & uart_clock_24);
		read_event(fd, buffer);
		break;

	case 3000000:
	case 4000000:
		/* Write UART Clock setting of 48MHz */
		DEBUG0("Change UART_CLOCK 48Mh\nz");
		SendCommand(VSC_WRITE_UART_CLOCK_SETTING,
			    VSC_WRITE_UART_CLOCK_SETTING_LEN,
			    (UINT8 *) & uart_clock_48);
		read_event(fd, buffer);
		break;

	default:
		fprintf(stderr, "Not Support baudrate = %ld\n", baudrate);
		exit_err(1);
		break;
	}

	hci_data[2] = baudrate & 0xFF;
	hci_data[3] = (baudrate >> 8) & 0xFF;
	hci_data[4] = (baudrate >> 16) & 0xFF;
	hci_data[5] = (baudrate >> 24) & 0xFF;

	DEBUG1("Change Baudrate %ld\n", baudrate);

	SendCommand(HCI_BRCM_UPDATE_BAUDRATE_CMD,
		    HCI_BRCM_UPDATE_BAUD_RATE_UNENCODED_LENGTH,
		    (UINT8 *) hci_data);
	read_event(fd, buffer);

	tcflush(fd, TCIOFLUSH);
	tcgetattr(fd, &termios);
	cfmakeraw(&termios);
	termios.c_cflag |= CRTSCTS;

	if (use_two_stop_bits)
		termios.c_cflag |= CSTOPB;

	tcsetattr(fd, TCSANOW, &termios);
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &termios);
	tcflush(fd, TCIOFLUSH);
	tcflush(fd, TCIOFLUSH);
	cfsetospeed(&termios, uart_speed(baudrate));
	cfsetispeed(&termios, uart_speed(baudrate));
	tcsetattr(fd, TCSANOW, &termios);

}

void EnableTestMode(void)
{
	UINT8 filter_data[] = { 0x02, 0x00, 0x02 };

	/* bt sleep  disable */
	HCILP_Enable(FALSE);

	/* Enable both Inquiry & Page Scans */
	SetScanEnable();

	/* Set Event Filter: Enable Auto Connect */
	SendCommand(HCI_SET_EVENT_FILTER, 0x03, (UINT8 *) filter_data);
	read_event(fd, buffer);

	/* Enable Device under test */
	SendCommand(HCI_ENABLE_DEV_UNDER_TEST_MODE, 0x0, NULL);
	read_event(fd, buffer);

	fprintf(stderr, "Enable Device Under Test\n");
}

void SetLocalFeatures(void)
{
	UINT8 *data = NULL;

	DEBUG0("Read Local Feature\n");
	SendCommand(HCI_READ_LOCAL_FEATURES, 0, NULL);
	read_event(fd, buffer);

	data = &buffer[7];

#if (BCM_DISABLE_RF_PWRCTRL == TRUE)
	fprintf(stderr, "Remove Power Control\n");
	data[2] &= 0xFB;	/* Power contrel */
#endif
	DEBUG0("Write Local Feature\n");
	SendCommand(VSC_HCI_CMD_SET_LOC_FEATURES_CMD, 0x08, (UINT8 *) data);
	read_event(fd, buffer);
}

void EnbleHCI(void)
{
	int i = N_HCI;
	int proto = HCI_UART_H4;

	if (ioctl(fd, TIOCSETD, &i) < 0) {
		fprintf(stderr, "Can't set line discipline\n");
		return;
	}

	if (ioctl(fd, HCIUARTSETPROTO, proto) < 0) {
		fprintf(stderr, "Can't set hci protocol\n");
		return;
	}
	fprintf(stderr, "Done setting line discpline\n");
	return;

}

void print_usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "BRCM BT tool for Linux    release %s\n", RELEASE_DATE);
	fprintf(stderr, "\n");
	fprintf(stderr,
		" Usage: bcmtool <tty Device> <command> [command parameter],....\n\n");
	fprintf(stderr,
		"  -FILE    Patchram file name     EX) -FILE=BCM43xx_xxx.hcd\n");
	fprintf(stderr,
		"  -BAUD    Set Baudrate           EX) -BAUD=3000000\n");
	fprintf(stderr,
		"  -ADDR    BD addr file name      EX) -ADDR=.bdaddr\n");
	fprintf(stderr, "  -SCO     Enable SCO/PCM config  EX) -SCO\n");
	fprintf(stderr,
		"  -SETSCO  SCO/PCM values verify  EX) -SETSCO=0,1,0,1,1,0,0,3,3,0\n");
	fprintf(stderr, "  -LP      Enable Low power       EX) -LP\n");
	fprintf(stderr, "  -FEATURE Set local Feature      EX) -FEATURE\n");
	fprintf(stderr,
		"  -DUT     Enable DUT mode(do not use with -LP) EX) -DUT\n");
	fprintf(stderr,
		"  -ATTACH  Attach BT controller to BlueZ stack  EX) -ATTACH\n");
	fprintf(stderr, "  -DEBUG   Debug message          EX) -DEBUG\n");
	fprintf(stderr, "  -CSTOPB  Set two stop bits for tty EX) -CSTOPB\n");
	fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
	UINT8 i = 0;

	if (argc < 2) {
		print_usage();
		exit(1);
	} else {
		fprintf(stderr, "BRCM BT tool for Linux    release %s\n",
			RELEASE_DATE);
	}

#if (DEPLOY_4335A_AXI_BRIDGE_PATCH == TRUE)
	hw_4335_axi_bridge_lock();
	usleep(50000);
#endif

	/* Open dev port */
	if ((fd = open(argv[1], O_RDWR | O_NOCTTY)) == -1) {
		fprintf(stderr, "port %s could not be opened, error %d\n",
			argv[1], errno);
		exit(2);
	}

	tcflush(fd, TCIOFLUSH);
	tcgetattr(fd, &termios);
	cfmakeraw(&termios);
	termios.c_cflag |= CRTSCTS;

	if (use_two_stop_bits)
		termios.c_cflag |= CSTOPB;

	tcsetattr(fd, TCSANOW, &termios);
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &termios);
	tcflush(fd, TCIOFLUSH);
	tcflush(fd, TCIOFLUSH);
	cfsetospeed(&termios, B115200);
	cfsetispeed(&termios, B115200);
	tcsetattr(fd, TCSANOW, &termios);

	signal(SIGALRM, expired);

	for (i = 2; i < argc; i++) {
		char *ptr = argv[i];

		if (strstr(ptr, "-DEBUG")) {
			debug_mode = TRUE;
			DEBUG0("DEBUG On\n");
			break;
		}

		if (strstr(ptr, "-CSTOPB")) {
			use_two_stop_bits = TRUE;
			DEBUG0("Use two stop bits for tty\n");
			break;
		}
	}

	for (i = 2; i < argc; i++) {
		char *ptr = argv[i];

		if (ptr == NULL)
			continue;

		fprintf(stderr, "[%d] %s\n", i - 1, ptr);

		if (strstr(ptr, "-FILE=")) {
			char prm_name[128];

			ptr += 6;

			strncpy(prm_name, ptr, 127);
			DownloadPatchram(prm_name);

		} else if (strstr(ptr, "-BAUD=")) {
			UINT32 baudrate;

			ptr += 6;
			baudrate = atoi(ptr);

			ChangeBaudRate(baudrate);
		} else if (strstr(ptr, "-ADDR=")) {
			char *bdaddr_filename;
			FILE *pFile = NULL;

			int bdaddr[10];	/* Displayed BD Address */

			BD_ADDR local_addr;	/* BD Address for write */

#if 0
			ptr += 6;
			if (sscanf
			    (ptr, "%02X:%02X:%02X:%02X:%02X:%02X", &bdaddr[0],
			     &bdaddr[1], &bdaddr[2], &bdaddr[3], &bdaddr[4],
			     &bdaddr[5]) != 6) {
				fprintf(stderr, "-ADDR: Parameter error");
				exit_err(1);
			}
			bte_write_bdaddr(bdaddr);
#endif
			ptr += 6;
			bdaddr_filename = ptr;

			if (bdaddr_filename) {
				pFile = fopen(bdaddr_filename, "r");
			}

			if (pFile) {
				char text[BTUI_MAX_STRING_LENGTH_PER_LINE];

				fgets(text, BTUI_MAX_STRING_LENGTH_PER_LINE,
				      pFile);
				sscanf(text, "%02x%02x", &bdaddr[0],
				       &bdaddr[1]);

				fgets(text, BTUI_MAX_STRING_LENGTH_PER_LINE,
				      pFile);
				sscanf(text, "%02x", &bdaddr[2]);

				fgets(text, BTUI_MAX_STRING_LENGTH_PER_LINE,
				      pFile);
				sscanf(text, "%02x%02x%02x", &bdaddr[3],
				       &bdaddr[4], &bdaddr[5]);

				fprintf(stderr,
					"Writing B/D Address = %02X:%02X:%02X:%02X:%02X:%02X\n",
					bdaddr[0], bdaddr[1], bdaddr[2],
					bdaddr[3], bdaddr[4], bdaddr[5]);

				ROTATE_BD_ADDR(local_addr, bdaddr);

				SendCommand(VSC_WRITE_BD_ADDR, BD_ADDR_LEN,
					    (UINT8 *) local_addr);
				read_event(fd, buffer);
			} else {
				fprintf(stderr, "-ADDR: file open fail\n");
				exit_err(1);
			}

		} else if (strstr(ptr, "-SCO")) {
			SetAudio();

		} else if (strstr(ptr, "-SETSCO=")) {
			ptr += 8;
			int value[10];

			if (sscanf
			    (ptr, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &value[0],
			     &value[1], &value[2], &value[3], &value[4],
			     &value[5], &value[6], &value[7], &value[8],
			     &value[9]) != 10) {
				DEBUG0("PCM / SCO configuration value err\n");
				DEBUG0
				    ("SCO_Routing,PCM_Interface_Rate,Frame_Type,Sync_Mode,Clock_Mode,LSB_First,Fill_bits,Fill_Method,Fill_Num,Right_Justify\n");
				exit_err(1);
			}

			SetScoConf(value[0], value[1], value[2], value[3],
				   value[4]);
			SetPcmConf(value[5], value[6], value[7], value[8],
				   value[9]);
			SetAudio();
		} else if (strstr(ptr, "-LP")) {
			HCILP_Enable(TRUE);
		} else if (strstr(ptr, "-DUT")) {
			EnableTestMode();
		} else if (strstr(ptr, "-FEATURE")) {
			SetLocalFeatures();
		} else if (strstr(ptr, "-ATTACH")) {
			EnbleHCI();
			while (1) {
				sleep(UINT_MAX);
			}
		} else if (strstr(ptr, "-DEBUG")) {
		} else if (strstr(ptr, "-CSTOPB")) {

		} else {
			fprintf(stderr, "Invalid parameter(s)!\n");
			exit_err(1);
		}
	}

	fprintf(stderr, "EXIT\n");
	close(fd);
	exit(0);

	return 0;
}
