package eu.tb.zesense;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;

public class ZeGPSManager {
	
	/* Leu/tb/zesense/ZeGPSManager; */
	
	/**
	 * We have a thread-safety problem here.
	 * Stuff is called from two threads, namely:
	 * - lastPolledLocation (updated by the listener,
	 *   read by the Streaming Manager)
	 * - locQueue (built in thread safe)
	 * - 
	 * 
	 * I fell like also locationManager is used by
	 * the system and our Streaming Manager thread..
	 */
	
	/*
	 * THE INTERFACE OFFERED BY THIS CLASS SHOULD BE 
	 * FUNCTIONALLY THE SAME AS THE ONE OFFERED BY
	 * ANDROID SENSORS IN THE NDK, SO THAT WE CAN USE
	 * BOTH ANDROID SENSORS AND THIS MANAGER WITH
	 * THE SAME LOGIC
	 * The NDK does not offer a function to know
	 * if a particular sensor is streaming or not.
	 * It may keep track of it internally not to
	 * perform the same action twice upon two successive
	 * requests, but it does not export this state in
	 * any way. We mirror this approach here with no
	 * way to export the state but with a check performed
	 * internally (the state info is isStreaming).
	 */
	
	private Context context;
	
	private static final String TAG = "ZeGPSManager";
	
	/* The listener registration will be called by native,
	 * but either a looper is .prepare()ed in the Java thread
	 * section that launches the native, or I have to save here
	 * the looper of the guy that init()s this manager.
	 * Preparing a looper in native does not seem to work. */
	//private Looper callerLooper;
	
	private LocationManager locationManager;
	
	/* As usual we need a blocking queue on the full condition (put)
	 * and a non-blocking on the empty condition (poll) */
	BlockingQueue<Location> locQueue;
	
	/* Location cache. */
	private Location lastPolledLocation;
	private boolean isStreaming = false;
	
	/* Handler thread for our Listener callbacks. */
	private HandlerThread handlerThread;
	
	/* "init", "(Landroid/content/Context;)V" */
    public void init(Context context) {
    	
    	Log.i(TAG, "ZeGPSManager initializing");
    	
    	this.context = context;
    	
		locationManager = (LocationManager)
				context.getSystemService(Context.LOCATION_SERVICE);
		if (locationManager == null)
			Log.w(TAG, "Location manager failed to initialize");
		
		

		/*
		callerLooper = Looper.myLooper();
		if (callerLooper == null)
			Log.w(TAG, "Caller looper is null...");
			*/
    	
    	locQueue = new LinkedBlockingQueue<Location>(20);
    	
    	isStreaming = false;
    }
	
    /* "destroy", "()V" */
	public void destroy() {
		//locationManager.removeUpdates(locationListener);
		if (isStreaming) stopStream();
		Log.i(TAG, "ZeGPSManager destroyed");
	}
	
	LocationListener locationListener = new LocationListener() {

		@Override
		public void onLocationChanged(Location location) {
			// TODO Auto-generated method stub
			Log.i(TAG, "Listener, onLocationChanged "+location.toString());
			
			/* The time that arrives with the fix is in UTC from 1Jan1970
			 * Let's fake it for the moment as we please.. */
			location.setTime(System.nanoTime());
			
			try {
				locQueue.put(location);
				/*synchronized(this) { 
					updateCache(location); //maybe a cache can be kept by the poll()er?
				}*/
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
	
	/* "updateCache", "(Landroid/location/Location;)V" */
	public void updateCache(Location location) {
		lastPolledLocation = location;
	}

	/* Cannot synchronize this, otherwise I cannot make space
	 * in the queue in case it is full and somebody is waiting for
	 * some space.
	 */
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
			//synchronized(this) {
				return new Location(lastPolledLocation);
			//}
		}
		
		handlerThread.run();
		locationManager.requestSingleUpdate(LocationManager.GPS_PROVIDER,
				locationListener, handlerThread.getLooper());
		
		return null;
	}
	
	/* "startStream", "(J)I" */
	public int startStream(/*long minTime*/) {
		Log.w(TAG, "Starting GPS stream");
		locQueue.clear();
		
		Log.w(TAG, "queue cleared");
		
		handlerThread = new HandlerThread("ZeGPSCallbacks");
		Log.i(TAG, "Created "+handlerThread.getName());
		
		try {
			handlerThread.start();
		} catch (Exception e) { 
			e.printStackTrace();
			return 0;
		}
		
		Log.w(TAG, "handler thread running");
		
		try {
			locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER,
				0, 0, locationListener, handlerThread.getLooper());
			Log.w(TAG, "location updates requested");
		} catch (Exception e) { 
			e.printStackTrace();
			return 0;
		}
		
		isStreaming = true;
		return 1;
	}
	
	
	/* "stopStream", "()I" */
	public int stopStream() {
		Log.w(TAG, "Stopping GPS stream");
		Log.w(TAG, "There are "+Integer.toString(locQueue.size())+" elements left in the queue");
		locationManager.removeUpdates(locationListener);
		try {
		handlerThread.quit();
		handlerThread.interrupt();
		} catch (Exception e) { 
			e.printStackTrace();
			return 0;
		}
		isStreaming = false;
		locQueue.clear();
		return 1;
	}
	
	/* "changeFrequency", "(J)I" */
	public int changeFrequency(/*long minTime*/) {
		if (!isStreaming) return 0;
		if ( stopStream()==1 ) {
			if ( startStream(/*minTime*/)==1 ) return 1;
		}
		return 0;
	}
	
}
