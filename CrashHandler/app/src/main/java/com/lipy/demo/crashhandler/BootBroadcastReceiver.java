//BootBroadcastReceiver.java
package com.lipy.demo.crashhandler;

/**
 * 创建人: lipy
 * 创建时间: 2018/4/23
 * 修改人:qicheng.deng
 * 修改时间:2018/4/23
 * 修改内容:
 *
 * 功能描述:开机自启
 */

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class BootBroadcastReceiver extends BroadcastReceiver {
	@Override
	public void onReceive(Context context, Intent intent) {
		final String action_boot ="android.intent.action.BOOT_COMPLETED";

		if (intent.getAction().equals(action_boot)){

			Intent intent2 = new Intent(context, MainActivity.class);
			intent2.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			context.startActivity(intent2);
		}
	}
}


