package eu.tb.zesense;

import android.content.Context;

public class ZeSMThread extends Thread {
	
	Context callerContext;
	
	public native int ze_sm_server_entry(Context context);

	public ZeSMThread (Context context) {
		callerContext = context;
	}
	
	@Override
	public void run() {
		
		
	}
	
}
