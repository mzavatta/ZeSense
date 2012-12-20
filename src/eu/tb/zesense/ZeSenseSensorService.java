package eu.tb.zesense;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import android.os.Process;

public class ZeSenseSensorService extends Service {
	
	private static final String TAG = "ZeSenseSensorService";
	
	@Override
	public void onCreate() {
		Log.i(TAG, "ZeSenseSensorService created");
	}
	
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
    	Log.i(TAG, "ZeSenseSensorService onStartCommand");
    	
    	// Avoid blocking the main UI thread in which the service is running
    	// Remember that Dalvik uses a pthread-like implementation
    	ZeSenseSensorThread sensorThread = new ZeSenseSensorThread();
    	sensorThread.start();
    	
		return START_NOT_STICKY;
    }
	
	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		Log.i(TAG, "ZeSenseSensorService onBind");
		return null;
	}
	
	@Override
	public void onDestroy() {
		Log.i(TAG, "ZeSenseSensorService destroyed");
	}
	
	// Avoid blocking the main UI thread in which the service is running
	private final class ZeSenseSensorThread extends Thread {
		private static final String TAG = "ZeSenseSensorThread";
		
		@Override
		public void run() {
			Log.i(TAG, "Inside ZeSenseSensorThread!");
			
			// Enter the native world..
			zeSense_SamplingNative();
			Log.i(TAG, "native call returned");
			
			// When it returns, bye bye..
			stopSelf();
		}
	}
	
    public native void zeSense_SamplingNative();
	
    static {
        System.loadLibrary("ZeSenseNativeSensorSampling");
    }

}
