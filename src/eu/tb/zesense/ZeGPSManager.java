package eu.tb.zesense;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Looper;
import android.util.Log;

public class ZeGPSManager {
	
	/* Leu/tb/zesense/ZeGPSManager; */
	
	Context context;
	
	private static final String TAG = "ZeGPSManager";
	
	/* The listener registration will be called by native,
	 * but either a looper is .prepare()ed in the Java thread
	 * section that launches the native, or I have to save here
	 * the looper of the guy that init()s this manager.
	 * Preparing a looper in native does not seem to work. */
	Looper callerLooper;
	
	LocationManager locationManager;
	
	/* As usual we need a blocking queue on the full condition (put)
	 * and a non-blocking on the empty condition (poll) */
	BlockingQueue<Location> locQueue;
	
	/* Location cache. */
	Location lastPolledLocation;
	boolean isStreaming = false;
	
    public void init(Context context) {
    	
    	Log.i(TAG, "ZeGPSManager initializing");
    	
    	this.context = context;
    	
		locationManager = (LocationManager)
				context.getSystemService(Context.LOCATION_SERVICE);
		if (locationManager == null)
			Log.w(TAG, "Location manager failed to initialize");
		
		callerLooper = Looper.myLooper();
		if (callerLooper == null)
			Log.w(TAG, "Caller looper is null...");
    	
    	locQueue = new LinkedBlockingQueue<Location>(20);
    	
    	isStreaming = false;
    }
	
	public void destroy() {
		locationManager.removeUpdates(locationListener);
		Log.i(TAG, "ZeGPSManager destroyed");
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

	/* "getSample", "()Landroid/location/Location;" */
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
	
	/* "startSingle", "()Landroid/location/Location;" */
	public Location startSingle() {
		if (isStreaming) {
			return new Location(lastPolledLocation);
		}
		locationManager.requestSingleUpdate(LocationManager.GPS_PROVIDER, locationListener, callerLooper);
		return null;
	}
	
	/* "startStream", "(J)I" */
	public int startStream(/*long minTime*/) {
		Log.w(TAG, "Starting GPS stream");
		locQueue.clear();
		try {
			locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER,
				0, 0, locationListener, callerLooper);
		} catch (Exception e) { 
			e.printStackTrace();
		}
		isStreaming = true;
		return 1;
	}
	
	/* "stopStream", "()I" */
	public int stopStream() {
		Log.w(TAG, "Stopping GPS stream");
		Log.w(TAG, "There are "+Integer.toString(locQueue.size())+"elements in the queue");
		Location l = locQueue.peek();
		Log.w(TAG, "The last one is"+l.toString());
		locationManager.removeUpdates(locationListener);
		isStreaming = false;
		locQueue.clear();
		return 1;
	}
	
	/* "changeFrequency", "(J)I" */
	public int changeFrequency(long minTime) {
		stopStream();
		startStream(/*minTime*/);
		return 1;
	}

}
