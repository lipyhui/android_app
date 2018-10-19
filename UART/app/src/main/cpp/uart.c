/**
 * 创建人: penghui.li
 * 创建时间: 2018/2/21
 * 修改人:penghui.li
 * 修改时间:2018/2/21
 * 修改内容:
 *
 * 功能描述:	串口基础函数
 */
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include "uart.h"

/**
  * @brief  set_uart_para 通信端口配置
  * @param  name 串口路径
  * @fd 串口文件描述符
  * @retval None
  */
int set_uart_para(const char *name, int fd) {
	struct termios options;
	int i;

	if (tcgetattr(fd, &options) != 0) {
		MSG("%s %s Setup Serial 1\n", UART_TAG, name);
		return 0;
	}
	bzero(&options, sizeof(options));

	/*修改控制模式，保证程序不会占用串口*/
	options.c_cflag |= CLOCAL;
	/*修改控制模式，使得能够从串口中读取输入数据*/
	options.c_cflag |= CREAD;
	/*屏蔽其他标志位*/
	options.c_cflag &= ~CSIZE;

	/*流控*/
	options.c_cflag &= ~CRTSCTS;

	/*数据位*/
	options.c_cflag |= CS8;

	/*波特率*/
	cfsetispeed(&options, B921600);
	cfsetospeed(&options, B921600);

	/*校验位*/
	options.c_cflag &= ~PARENB;
	//options.c_iflag &= ~INPCK;

	/*停止位*/
	options.c_cflag &= ~CSTOPB;

	/*修改输出模式，原始数据输出*/
	//options.c_oflag &= ~OPOST;

	/*设置等待时间和最小接收字符*/
	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN] = 1;

	/*如果发生数据溢出，接收数据，但是不再读取*/
	tcflush(fd, TCIFLUSH);

	/*激活配置*/
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		MSG("%s %s com set error!\n", UART_TAG, name);
		return 0;
	}

	return 1;
}

/**
  * @brief  init_uart_port 初始化串口
  * @param  name 串口路径
  * @retval None
  */
int init_uart_port(const char *name) {
	int fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY);

	if (fd < 0) {
		MSG("%s %s Open uart port fail.\n", UART_TAG, name);
		return -1;
	}

	if (fcntl(fd, F_SETFL, O_NDELAY) < 0) {
		MSG("%s %s fcntl failed!\n", UART_TAG, name);
		return -2;
	}

	if (isatty(fd) == 0) {
		MSG("%s %s standard input is not a terminal device\n", UART_TAG, name);
		return -3;
	}

	/*配置串口速率、位数等*/
	if (set_uart_para(name, fd) == 0) {
		MSG("%s %s setup com port err!\n", UART_TAG, name);
		return -4;
	}

	return fd;
}

/**
 * 初始化串口（供 Java 调用）
 *
 * @param namePath	串口路径
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_lipy_demo_uart_UartJNI_initUart(JNIEnv *env, jobject obj, jstring namePath) {
	return init_uart_port((*env)->GetStringUTFChars(env, namePath, 0));
}

/**
 * 串口发送数据
 *
 * @param name	串口路径
 * @param fd	串口文件描述符
 * @param buff	发送数据
 * @param len	发送数据长度
 * @param timeout_ms	发送数据超时
 * @return	发送数据状态
 */
int uart_send_data(const char *name, int fd, unsigned char *buff, unsigned short len, long timeout_ms) {
	int ret;
	fd_set write_fds;
	struct timeval tv;
	int i = 0;

	FD_ZERO(&write_fds);
	FD_SET(fd, &write_fds);

	/*串口忙超时时间设置*/
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	ret = select(fd + 1, NULL, &write_fds, NULL, &tv);
	if ((ret <= 0) || !FD_ISSET(fd, &write_fds)) {
		MSG("%s %s uart is busy!!\n", UART_TAG, name);
		return -1;
	}

#if PLCD_DEBUG_UART_ARRAY
	for(i=0; i<len; i++) {
		MSG("%s %s send buff is 0x%02x \n", UART_TAG, name, buff[i]);
	}
#endif

	write(fd, buff, len);

	/*等待UART传输完成*/
	FD_ZERO(&write_fds);
	FD_SET(fd, &write_fds);

	/*串口数据发送超时时间设置，数据发送超时未读忙超时的 1/10 ，最小为 10ms*/
	timeout_ms /= 10;
	if (timeout_ms < 10) {
		timeout_ms = 10;
	}
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	ret = select(fd + 1, NULL, &write_fds, NULL, &tv);
	if ((ret > 0) && FD_ISSET(fd, &write_fds)) {
//		MSG("%s %s Send data success !!!\n", UART_TAG, name);
		return ret;
	}

	MSG("%s %s Send data time out !!!\n", UART_TAG, name);
	return -4;
}

/**
 * 串口发送数据(Java 调用)
 *
 * @param env
 * @param obj
 * @param namePath	串口参数
 * @param fd	串口文件描述符
 * @param datas	发送数据
 * @param length	发送数据长度
 * @param timeout_ms	发送超时
 * @return	数据发送状态
 */
JNIEXPORT jint JNICALL
Java_com_lipy_demo_uart_UartJNI_sendUartData(JNIEnv *env, jobject obj, jstring namePath, jint fd, jbyteArray datas,
												  jint length, jlong timeout_ms) {
	unsigned char *sendBuff = NULL;
	jbyte *ba = (*env)->GetByteArrayElements(env, datas, JNI_FALSE); //jbyteArray转为jbyte*

	if (length > 0) {
		sendBuff = (char *) malloc(length + 1);         //"\0"
		memcpy(sendBuff, ba, length);
		sendBuff[length] = '\0';
	}
	(*env)->ReleaseByteArrayElements(env, datas, ba, 0);  //释放掉

	return uart_send_data((*env)->GetStringUTFChars(env, namePath, 0), fd, sendBuff, length, timeout_ms);
}

/**
 * 串口接收数据
 *
 * @param name	串口路径
 * @param fd	串口文件描述符
 * @param datas	接收数据
 * @param timeout_ms	接收数据超时
 * @return	接收数据长度
 */
int uart_recv_data(const char *name, int fd, unsigned char *datas, long timeout_ms) {
	int ret;
	fd_set read_fds;
	struct timeval tv;
	int readLen = 0, allReadLen = 0;
	int loopDelay;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
	if ((ret > 0) && FD_ISSET(fd, &read_fds)) {
		/*计算循环读串口延时时间，最大 2000 * 5 us*/
		if (timeout_ms > 2000) {
			loopDelay = 2000 * 5;
		} else {
			loopDelay = timeout_ms * 5;
		}

		//循环读串口数据
		while (1) {
			//读串口数据
			readLen = read(fd, datas + allReadLen, MAX_UART_BUFFER_SIZE - allReadLen);
			/*打印调试信息*/
			MSG("%s %s readLen %d\n", UART_TAG, name, readLen);

			if (readLen <= 0) {    //判断串口数据是否读完
				break;
			} else if (allReadLen + readLen > MAX_UART_BUFFER_SIZE) {    //串口数据大于串口缓存buff
				return -1;
			} else {
				//计算串口数据总长度
				allReadLen += readLen;
			}

			//休眠一段时间
			usleep(loopDelay);
		}
		/*打印调试信息*/
		MSG("%s %s allReadLen %d\n", UART_TAG, name, allReadLen);
#if PLCD_DEBUG_UART_ARRAY
		for(i=0; i<allReadLen; i++) {
			MSG("%s %s send buff is 0x%02x \n", UART_TAG, name, buff[i]);
		}
#endif
		return allReadLen;
	}

	return -1;
}

/**
 * 串口接收数据(Java 调用)
 *
 * @param env
 * @param obj
 * @param namePath	串口参数
 * @param fd	串口文件描述符
 * @param recvDatas	接收数据
 * @param timeout_ms	接收数据超时
 * @return	接收数据长度
 */
JNIEXPORT jint JNICALL
Java_com_lipy_demo_uart_UartJNI_recvUartData(JNIEnv *env, jobject obj, jstring namePath, jint fd,
												  jbyteArray recvDatas,
												  jlong timeout_ms) {
	int readLen = 0;
	unsigned char readBuff[MAX_UART_BUFFER_SIZE];

	memset(readBuff, 0, MAX_UART_BUFFER_SIZE);

	readLen = uart_recv_data((*env)->GetStringUTFChars(env, namePath, 0), fd, readBuff, timeout_ms);

	//读失败
	if (readLen < 0) {
		return readLen;
	}

	(*env)->SetByteArrayRegion(env, recvDatas, 0, readLen, readBuff);

	return readLen;
}

/**
 * 获取串口最大缓存大小
 *
 * @param env
 * @param obj
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_lipy_demo_uart_UartJNI_getMaxUartBUffSize(JNIEnv *env, jobject obj) {
	return MAX_UART_BUFFER_SIZE;
}

/**
 * 关闭串口
 *
 * @param env
 * @param obj
 * @param fd 串口文件描述符
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_lipy_demo_uart_UartJNI_closeUart(JNIEnv *env, jclass type, jint fd) {
	return close(fd);
}