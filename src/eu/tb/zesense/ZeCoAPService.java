package eu.tb.zesense;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class ZeCoAPService extends Service {
	
	private static final String TAG = "ZeSense";

	@Override
	public void onCreate() {
		Log.i(TAG, "ZeCoAPService created");
	}
	
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
    	Log.i(TAG, "ZeCoAPService onStartCommand");
    	
    	// Avoid blocking the main UI thread in which the service is running
    	// Remember that Dalvik uses a pthread-like implementation
    	ZeCoAPThread sensorThread = new ZeCoAPThread();
    	sensorThread.start();
    	
		return START_NOT_STICKY;
    }
	
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public void onDestroy() {
		Log.i(TAG, "ZeCoAPService destroyed");
	}
	
	// Avoid blocking the main UI thread in which the service is running
	private final class ZeCoAPThread extends Thread {
		
		private static final String TAG = "ZeSense";
		
		@Override
		public void run() {
			Log.i(TAG, "ZeCoAPThread started running");
			
			// Enter the native world..
			ZeJNIHub.ze_coap_server_root();
			Log.i(TAG, "ZeCoAPThread native call returned");
			
			// When it returns, bye bye..
			stopSelf();
		}
	}

}
