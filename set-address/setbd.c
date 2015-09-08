/*
 * setbd.c
 *
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
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
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>

#define	BD_ADDR_PATH	"/csa/bluetooth"
#define	BD_ADDR_TEMP_PATH	"/opt/var/lib/bluetooth"

#define	BD_ADDR_FILE	"/csa/bluetooth/.bd_addr"
#define	BD_ADDR_TEMP_FILE	"/opt/var/lib/bluetooth/.bd_addr"

#define	BD_ADDR_LEN	14
#define BD_PREFIX	"0002\n"

void makeRandomBD(unsigned char *buf)
{
	int ran;
	int i;
	unsigned int seed;
	struct timeval tv;

	memcpy(buf, BD_PREFIX, 5);
	i = gettimeofday(&tv, NULL);

	if (i < 0) {
		perror("Fail to call gettimeofday()");
		seed = time(NULL);
	} else
		seed = (unsigned int)tv.tv_usec;

	for (i = 5; i < BD_ADDR_LEN; i++) {
		if (7 == i) {
			buf[i] = '\n';
			continue;
		}
		ran = rand_r(&seed) % 16;
		if (10 > ran)
			ran += 0x30;
		else
			ran += 0x57;
		buf[i] = ran;
	}
	printf("Random number is\r\n");
	for (i = 0; i < BD_ADDR_LEN; i++) {
		printf("%c", buf[i]);
	}
	printf("\r\n");
}

int make_bt_address_folder(char *path)
{
	DIR *dp;
	int err;

	dp = opendir(path);
	if (dp == NULL) {
		if (mkdir(path, 0755) < 0) {
			err = -errno;
			printf("mkdir: %s(%d)", strerror(-err), -err);
			return -1;
		}
		return 0;
	}

	closedir(dp);
	return 0;
}

int make_bt_address(char *path, char *file)
{
	int fd;
	unsigned char txt[BD_ADDR_LEN];
	char nap[4 + 1], uap[2 + 1], lap[6 + 1];
	int ret;

	if (make_bt_address_folder(path) < 0)
			return -1;

	fd = open(file, O_RDONLY | O_SYNC);

	if (fd < 0) {
		printf("File not exist\n");

		fd = open(file, O_RDWR | O_CREAT | O_TRUNC | O_SYNC,
			  0644);

		if (fd < 0) {
			printf("Can't open address file\n");
			return -1;
		}

		makeRandomBD(txt);

		ret = write(fd, txt, BD_ADDR_LEN);
		if (ret != BD_ADDR_LEN) {
			printf("Unable to write device address\n");
			close(fd);
			unlink(file);
			return -1;
		}

		lseek(fd, 0, SEEK_SET);
	} else {
		printf("%s is already existed\n", file);
	}

	ret = read(fd, nap, 5);
	if (ret != 5)
		goto err;

	ret = read(fd, uap, 3);
	if (ret != 3)
		goto err;

	ret = read(fd, lap, 7);
	if (ret != 6)
		goto err;

	close(fd);

	/* Unfortunately 00023fbf0a1a address is duplicated from the
	 * previous IMEI logic. So this address should be updated with
	 * random value.
	 *
	 * This is temporal code. And this would be reverted around a few week
	 * after the wrong addressed device has proper address.
	 */
	if (strncmp(nap, "0002", 4) == 0 &&
	    strncmp(uap, "3f", 2) == 0 && strncmp(lap, "bf0a1a", 6) == 0) {
		printf("%s has wrong address\n", file);
		fd = open(file, O_RDWR | O_CREAT | O_TRUNC | O_SYNC,
			  0644);

		if (fd < 0) {
			printf("Can't open address file\n");
			return -1;
		}

		makeRandomBD(txt);
		ret = write(fd, txt, BD_ADDR_LEN);
		if (ret != BD_ADDR_LEN) {
			printf("Unable to write device address\n");
			close(fd);
			unlink(file);
			return -1;
		}

		lseek(fd, 0, SEEK_SET);
		close(fd);
	}

	return ret;
 err:
	printf("read() failed, ret = %d\n", ret);
	close(fd);
	unlink(file);
	return -1;
}

int main()
{
	printf("Bluetooth Address Setting\n");
	if (make_bt_address(BD_ADDR_PATH, BD_ADDR_FILE) < 0) {
		if (make_bt_address(BD_ADDR_TEMP_PATH, BD_ADDR_TEMP_FILE) < 0) {
			return -1;
		} else {
			return 1;
		}
	}
	return 0;
}
