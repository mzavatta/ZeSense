package eu.tb.zesense;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

public class ZeGPSManager_serv extends Service {
	
	/* eu/tb/ZeSense/ZeGPSManager */
	
	private static final String TAG = "ZeGPSManager";
	
	//Looper callerLooper;
	
	LocationManager locationManager;
	
	/* As usual we need a blocking queue on the full condition (put)
	 * and a non-blocking on the empty condition (poll) */
	BlockingQueue<Location> locQueue;
	
	/* Location cache. */
	Location lastPolledLocation;
	boolean isStreaming = false;
	
	@Override
	public void onCreate() {
		Log.i(TAG, "ZeGPSManager created");
	}
	
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
    	Log.i(TAG, "ZeGPSManager onStartCommand");
    	
		locationManager = (LocationManager)
				this.getSystemService(Context.LOCATION_SERVICE);
		if (locationManager == null) {
			Log.w(TAG, "Location manager failed to initialize");
		}
		
		/*
		callerLooper = Looper.myLooper();
		if (callerLooper == null)
			Log.w(TAG, "Caller looper is null...");
		*/
    	
    	locQueue = new LinkedBlockingQueue<Location>(20);
    	
    	isStreaming = false;
    	
		return START_NOT_STICKY;
    }
	
	@Override
	public void onDestroy() {
		locationManager.removeUpdates(locationListener);
		Log.i(TAG, "ZeGPSManager destroyed");
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
	
	LocationListener locationListener = new LocationListener() {

		@Override
		public void onLocationChanged(Location location) {
			// TODO Auto-generated method stub
			Log.i(TAG, "Listener, onLocationChanged");
			try {
				locQueue.put(location);
				lastPolledLocation = location;
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		@Override
		public void onProviderDisabled(String provider) {
			Log.i(TAG, "Listener, onProviderDisabled");
		}

		@Override
		public void onProviderEnabled(String provider) {
			Log.i(TAG, "Listener, onProviderEnabled");
		}

		@Override
		public void onStatusChanged(String provider, int status, Bundle extras) {
			Log.i(TAG, "Listener, onStatusChanged");
		}
	};

	/* "getSample", "()Leu/tb/ZeSense/Location;" */
	public Location getSample() {
		/* As we pass the pointer back to the native and the native
		 * will access its value, the same memory block must not be
		 * overwritten by other Java code.
		 * Is the queue implementation unlinking the retrieved object? Likely.
		 * http://developer.android.com/reference/java/util/concurrent/LinkedBlockingQueue.html#poll%28%29
		 * Otherwise we can make a copy though the constructor
		 * Location(Location copy) built in the Location type.
		 */	
		/* Retrieves and removes the head of this queue,
		 * or returns null if this queue is empty. */
		return locQueue.poll();
	}
	
	/* "startSingle", "()Leu/tb/ZeSense/Location;" */
	public Location startSingle() {
		if (isStreaming) {
			return new Location(lastPolledLocation);
		}
		locationManager.requestSingleUpdate(LocationManager.GPS_PROVIDER, locationListener, null);
		return null;
	}
	
	/* "startStream", "(J)I" */
	public int startStream(long minTime) {
		locQueue.clear();
		locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, minTime, 0, locationListener);
		isStreaming = true;
		return 1;
	}
	
	/* "stopStream", "()I" */
	public int stopStream() {
		locationManager.removeUpdates(locationListener);
		isStreaming = false;
		locQueue.clear();
		return 1;
	}
	
	/* "changeFrequency", "(J)I" */
	public int changeFrequency(long minTime) {
		stopStream();
		startStream(minTime);
		return 1;
	}
}
