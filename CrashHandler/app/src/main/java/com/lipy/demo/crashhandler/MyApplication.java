package com.lipy.demo.crashhandler;

import android.app.Application;

/**
 * 创建人: penghui.li
 * 创建时间: 2018/10/18
 * 修改人:penghui.li
 * 修改时间:2018/10/18
 * 修改内容:
 *
 * 功能描述:
 */
public class MyApplication extends Application {

	@Override
	public void onCreate() {
		super.onCreate();

		//异常捕获初始化
		CrashHandler.getInstance().initCrashHandler(this); // 一定要初始化
	}
}
