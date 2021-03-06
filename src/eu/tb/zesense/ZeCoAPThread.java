package eu.tb.zesense;

import android.content.Context;
import android.util.Log;

// Avoid blocking the main UI thread in which the service is running
public class ZeCoAPThread extends Thread {
	
	/* Leu/tb/zesense/ZeCoAPThread; */
	
	private static final String TAG = "ZeSense";
	
	Context callerContext;
	
	//ZeGPSManager gpsManager;
	
    //public native int ze_coap_server_entry(ZeGPSManager gpsManager);
	
	public native int ze_coap_server_entry(Context context);
    
	/*
    public ZeCoAPThread (ZeGPSManager gpsManager) {
    	this.gpsManager = gpsManager;
    }
    */
	
	public ZeCoAPThread (Context context) {
		callerContext = context;
	}
	
	@Override
	public void run() {
		Log.i(TAG, "ZeCoAPThread started running");
		
		//Looper.prepare();
		
		// Enter the native world..
		int r = ze_coap_server_entry(callerContext);
		
		Log.i(TAG, "ZeCoAPThread native call returned");
		if (r<0) Log.i(TAG, "ZeCoAPThread returned negative..");
		
		//Looper.loop();
		
		// When it returns, bye bye..
		//stop();
		//Thread.currentThread().interrupt();
	}
	
    static {
        System.loadLibrary("ZeSense");
    }

}
