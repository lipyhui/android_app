package com.lipy.demo.uart;

/**
 * 创建人: lipy
 * 创建时间: 2017/8/29
 * 修改人:lipy
 * 修改时间:2017/8/29
 * 修改内容:
 *
 * 功能描述:	heartbeat 与 JNI 建立连接的类，提供可以供Java调用方法
 */

public class UartJNI {
	static {
		System.loadLibrary("uart");
	}

	/**
	 * 串口初始化
	 *
	 * @param namePath 串口路径
	 * @return    串口文件描述符
	 */
	public static native int initUart(String namePath);

	/**
	 * 串口发送数据
	 *
	 * @param namePath   串口路径
	 * @param fd         串口文件描述符
	 * @param datas      待发送数据
	 * @param length     发送数据长度
	 * @param timeout_ms 数据发送超时，单位是 ms
	 * @return 数据发送状态
	 */
	public static native int sendUartData(String namePath, int fd, byte[] datas, int length, long timeout_ms);

	/**
	 * 串口接收数据
	 *
	 * @param namePath   串口路径
	 * @param fd         串口文件描述符
	 * @param recvDatas  接收数据
	 * @param timeout_ms 数据接收超时，单位是 ms
	 * @return 接收数据长度
	 */
	public static native int recvUartData(String namePath, int fd, byte[] recvDatas, long timeout_ms)
			throws ArrayIndexOutOfBoundsException;

	/**
	 * 获取串口最大缓冲大小
	 *
	 * @return 串口缓冲区大小
	 */
	public static native int getMaxUartBUffSize();

	/**
	 * 关闭串口
	 *
	 * @param fd 串口文件描述符
	 * @return 串口关闭状态
	 */
	public static native int closeUart(int fd);
}
