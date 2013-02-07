package eu.tb.zesense;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class ZeCoAPService extends Service {
	
	private static final String TAG = "ZeSense";
	
	ZeCoAPThread sensorThread;

	@Override
	public void onCreate() {
		Log.i(TAG, "ZeCoAPService created");
	}
	
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
    	Log.i(TAG, "ZeCoAPService onStartCommand");
    	
    	// Avoid blocking the main UI thread in which the service is running
    	// Remember that Dalvik uses a pthread-like implementation
    	sensorThread = new ZeCoAPThread();
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
		sensorThread.interrupt();
		Log.i(TAG, "ZeCoAPService destroyed");
	}

}
