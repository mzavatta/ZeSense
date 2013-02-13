package eu.tb.zesense;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class ZeCoAPService extends Service {
	
	private static final String TAG = "ZeSense";
	
	ZeCoAPThread serverThread;
	
	//ZeGPSManager gpsManager;

	@Override
	public void onCreate() {
		Log.i(TAG, "ZeCoAPService created");
	}
	
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
    	Log.i(TAG, "ZeCoAPService onStartCommand");
    	
    	/*
    	gpsManager = new ZeGPSManager();
    	gpsManager.init(this);
    	*/
    	
    	// Avoid blocking the main UI thread in which the service is running
    	// Remember that Dalvik uses a pthread-like implementation   	
    	serverThread = new ZeCoAPThread(this);
    	serverThread.start();
    	
		return START_NOT_STICKY;
    }
	
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public void onDestroy() {
		/* It's actually not the right position for this,
		 * the role of graceful quit should be borne by the
		 * Sreaming Manager, who uses the object
		 */
		//gpsManager.destroy();
		
		serverThread.interrupt();
		Log.i(TAG, "ZeCoAPService destroyed");
	}

}
