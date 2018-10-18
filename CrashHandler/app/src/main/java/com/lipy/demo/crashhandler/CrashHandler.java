package com.lipy.demo.crashhandler;

import android.app.AlarmManager;
import android.app.Application;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.io.PrintWriter;
import java.io.RandomAccessFile;
import java.io.StringWriter;
import java.io.Writer;
import java.text.SimpleDateFormat;

/**
 * 创建人: lipy
 * 创建时间: 2017/8/4
 * 修改人:lipy
 * 修改时间:2017/8/4
 * 修改内容:
 *
 * 功能描述:捕获奔溃并重启
 */
public class CrashHandler implements Thread.UncaughtExceptionHandler {
	private final String PATH = "/storage/sdcard0/crash_logger.txt";

	public static CrashHandler mAppCrashHandler;

	private Thread.UncaughtExceptionHandler mDefaultHandler;

	private Application mAppContext;

	public static CrashHandler getInstance() {
		if (mAppCrashHandler == null) {
			mAppCrashHandler = new CrashHandler();
		}
		return mAppCrashHandler;
	}

	public void initCrashHandler(Application application) {
		this.mAppContext = application;
		// 获取系统默认的UncaughtException处理器
		mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();
		Thread.setDefaultUncaughtExceptionHandler(this);
	}

	@Override
	public void uncaughtException(Thread thread, Throwable ex) {
		if (!handleException(ex) && mDefaultHandler != null) {
			// 如果用户没有处理则让系统默认的异常处理器来处理
			//mDefaultHandler.uncaughtException(thread, ex);
		}

		//设置重启定时器
		AlarmManager mgr = (AlarmManager) mAppContext.getSystemService(Context.ALARM_SERVICE);

		//1秒后重启
		Intent intent = new Intent(mAppContext, MainActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		PendingIntent restartIntent = PendingIntent.getActivity(mAppContext, 0, intent, PendingIntent.FLAG_ONE_SHOT);
		mgr.set(AlarmManager.RTC, System.currentTimeMillis() + 1000, restartIntent); // 1秒钟后重启应用

		//杀死进程
		android.os.Process.killProcess(android.os.Process.myPid());
		System.exit(0);
		System.gc();
	}

	/**
	 * 错误处理,收集错误信息、发送错误报告等操作均在此完成.
	 *
	 * @param ex 错误信息
	 * @return true:如果处理了该异常信息;否则返回false.
	 */
	private boolean handleException(Throwable ex) {
		if (ex == null) {
			return false;
		}

		//收集奔溃日志
		Writer writer = new StringWriter();
		PrintWriter printWriter = new PrintWriter(writer);
		ex.printStackTrace(printWriter);
		Throwable cause = ex.getCause();
		while (cause != null) {
			cause.printStackTrace(printWriter);
			cause = cause.getCause();
		}
		printWriter.close();
		String result = writer.toString();

		//保存日志
		saveInfo(result);

		// 自定义处理错误信息
		return true;
	}

	/**
	 * 保存错误信息到本地文件
	 *
	 * @param msg String
	 */
	private void saveInfo(String msg) {
		try {
			String time = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(System.currentTimeMillis());
			String info = time + ":\n\r" + msg + "\n\r\n\r";
			RandomAccessFile log = new RandomAccessFile(PATH, "rw");
			log.skipBytes((int) log.length());
			log.write(info.getBytes());
			log.close();
		} catch (Exception e) {
			Log.e("CrashHandler", "save crash info err !!" + e.toString());
		}
	}
}
