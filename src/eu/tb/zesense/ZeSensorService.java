package eu.tb.zesense;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class ZeSensorService extends Service {
	
	private static final String TAG = "ZeSensorService";
	
	@Override
	public void onCreate() {
		Log.i(TAG, "ZeSensorService created");
	}
	
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
    	Log.i(TAG, "ZeSensorService onStartCommand");
    	
    	// Avoid blocking the main UI thread in which the service is running
    	// Remember that Dalvik uses a pthread-like implementation
    	ZeSenseSensorThread sensorThread = new ZeSenseSensorThread();
    	sensorThread.start();
    	
		return START_NOT_STICKY;
    }
	
	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		Log.i(TAG, "ZeSensorService onBind");
		return null;
	}
	
	@Override
	public void onDestroy() {
		Log.i(TAG, "ZeSensorService destroyed");
	}
	
	// Avoid blocking the main UI thread in which the service is running
	private final class ZeSenseSensorThread extends Thread {
		private static final String TAG = "ZeSensorThread";
		
		@Override
		public void run() {
			Log.i(TAG, "Inside ZeSensorThread!");
			
			// Enter the native world..
			ZeJNIHub.ze_samplingnative();
			Log.i(TAG, "native call returned");
			
			// When it returns, bye bye..
			stopSelf();
		}
	}

}
