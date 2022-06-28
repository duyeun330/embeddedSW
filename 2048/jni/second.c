#include <jni.h>
#include "android/log.h"
#include <unistd.h>
#include <fcntl.h>

#define MAXB 9

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

jint JNICALL Java_com_example_hnb_PlayGame_movXY(JNIEnv *env, jobject this)
{
	int i;
	int dev;
	int size;

		unsigned char sw[MAXB];

		dev = open("/dev/switch_driver", O_RDWR);
		if (dev<0){
			close(dev);
			return -1;
		}

		size = sizeof(sw);
		read(dev, &sw, size);
		close(dev);
		if (sw[1] == 1)
			return (1);
		else if (sw[3] == 1)
			return (2);
		else if (sw[5] == 1)
			return (3);
		else if (sw[7] == 1)
			return (4);
		else
			return (0);
}

void JNICALL Java_com_example_hnb_PlayGame_printFND(JNIEnv *env, jobject this, jint n)
{
	int i;
	int dev;
	int size;
	unsigned char data[4];
	unsigned char retval;

	dev = open("/dev/fnd_driver", O_RDWR);
	if (dev<0){
		close(dev);
		return ;
	}
	size = sizeof(data);
	for (i = 3; i >= 0; i--){
		data[i] = n % 10;
		n /= 10;
	}
	retval = write(dev, &data, 4);
	close(dev);
}

void JNICALL Java_com_example_hnb_PlayGame_printDOT(JNIEnv *env, jobject this, jint n)
{
	unsigned char blank[10] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	unsigned char left[10] = {
		0x0c, 0x18, 0x30, 0x60, 0x7f, 0x7f, 0x60, 0x30, 0x18, 0x0c
	};

	unsigned char right[10] = {
		0x18, 0x0c, 0x06, 0x03, 0x7f, 0x7f, 0x03, 0x06, 0x0c, 0x18
	};

	unsigned char downn[10] = {
		0x08, 0x08, 0x08, 0x08, 0x49, 0x49, 0x6b, 0x3e, 0x1c, 0x08
	};

	unsigned char upp[10] = {
		0x08, 0x1c, 0x3e, 0x6b, 0x49 ,0x49, 0x08, 0x08, 0x08, 0x08
	};
	int i;
	int dev;
	int size;

	dev = open("/dev/dot_driver", O_RDWR);
	if (dev<0){
		close(dev);
		return ;
	}
	size = sizeof(upp);
	if (n == 1) {
		write(dev, upp, size);
	}
	else if (n == 2){
		write(dev, left, size);
	}
	else if (n == 3){
		write(dev, right, size);
	}
	else if (n == 4) {
		write(dev, downn, size);
	}
	else if (n == 5) {
		write(dev, blank, size);
	}
	close(dev);
}


