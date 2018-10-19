/**
 * 创建人: penghui.li
 * 创建时间: 2018/2/21
 * 修改人:penghui.li
 * 修改时间:2018/2/21
 * 修改内容:
 *
 * 功能描述:	串口基础函数
 */
#ifndef __PLCD_UART_H
#define __PLCD_UART_H

#include <jni.h>
#include <stdlib.h>
#include <errno.h>
#include <android/log.h>
#include <string.h>

#define UART_TAG "[UART]"

#define MAX_UART_BUFFER_SIZE	4096

static void plcd_log(char* fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	__android_log_vprint(ANDROID_LOG_ERROR,
						 "uart_C", fmt, ap);
}

#define PLCD_DEBUG_CONSOLE		0
#define PLCD_DEBUG_LOGCAT		0

#define PLCD_DEBUG_UART_ARRAY	0

#if PLCD_DEBUG_CONSOLE
#define MSG(fmt, arg...)	printf(fmt, ##arg)
#elif PLCD_DEBUG_LOGCAT
#define MSG(fmt, arg...)	plcd_log(fmt, ##arg)
#else
#define MSG(fmt, arg...)
#endif

#define bzero(a, b)		memset(a, 0, b)

int set_uart_para(const char* name, int fd);
int init_uart_port(const char* name);
int uart_send_data(const char* name, int fd, unsigned char *buff, unsigned short len, long timeout_ms);
int uart_recv_data(const char* name, int fd, unsigned char *datas, long timeout_ms);
#endif /*__PLCD_UART_H*/