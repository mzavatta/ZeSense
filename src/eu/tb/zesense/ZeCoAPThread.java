package eu.tb.zesense;

import android.util.Log;

// Avoid blocking the main UI thread in which the service is running
public class ZeCoAPThread extends Thread {
	
	private static final String TAG = "ZeSense";
	
    public native int ze_coap_server_entry();
	
	@Override
	public void run() {
		Log.i(TAG, "ZeCoAPThread started running");
		
		// Enter the native world..
		int r = ze_coap_server_entry();
		Log.i(TAG, "ZeCoAPThread native call returned");
		if (r<0) Log.i(TAG, "ZeCoAPThread returned negative..");
		
		// When it returns, bye bye..
		//stop();
		Thread.currentThread().interrupt();
	}
	
    static {
        System.loadLibrary("ZeSense");
    }

}
