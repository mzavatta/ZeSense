package eu.tb.zesense;

import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.Camera;
import android.util.Log;
import android.view.Menu;
import android.widget.Button;
import android.widget.TextView;

public class ZeSense extends Activity {

	private static final String TAG = "ZeSense";
	
	Camera camera;
	ZeCameraPreview zePreview;
	Button buttonClick;
	//int sensorServiceStarted = 0;
	
	// Our custom Wifi manager
	ZeWifiAP zeWifiAP;
	
	//ZeGPSManager gpsManager;
	
	Intent coapServiceIntent;
	Intent GPSServiceIntent;
	
	WakeLock wakeLock;
	
	TextView display;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ze_sense);
        
        display = (TextView) findViewById(R.id.display);
        
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "ZeSense Wakelock");
        wakeLock.acquire();
        
        /*
        ZeSamplingStats s = new ZeSamplingStats(this, display);
        s.runStats();
        */
        
        ///****************************
        // Set-up Wifi Access Point
        zeWifiAP = new ZeWifiAP(this);
        if(zeWifiAP.initialize("zesense")) {
        	if(!zeWifiAP.startAP()) Log.w(TAG, "ZeSense cannot start Wifi AP");
        }
        else Log.w(TAG, "ZeSense cannot initialize Wifi AP");
        //******************************/
        
        /*
        ZeWifiDirectAP wifiAccessPoint = new ZeWifiDirectAP(this);
        wifiAccessPoint.initialize();
        wifiAccessPoint.startAP();
        */
        /*
        GPSServiceIntent = new Intent(this, ZeGPSManager_serv.class);
        startService(GPSServiceIntent);
        */
        // Load native libraries
        //ZeJNIHub.loadNativeLibraries();
        
        ///*********************
        // Start CoAP service
        coapServiceIntent = new Intent(this, ZeCoAPService.class);
        startService(coapServiceIntent);
        //**********************/
        
        /*
        // Start CoAP server
        // TODO: new Service+thread shall be spawned, JNI call is synchronous and will never return!
        ZeJNIHub.ze_coap_server_main();
        */
        
		//zePreview = new ZeCameraPreview(this);
		//((FrameLayout) findViewById(R.id.zePreview)).addView(zePreview);
       
		// Start our SensorService
		//Intent sensorServiceIntent = new Intent(this, ZeSensorService.class);
		
		/*
		if (savedInstanceState != null) { //recreated activity
			if (savedInstanceState.getInt(SERVICE_STARTED) == 0) {
				startService(sensorServiceIntent);
				sensorServiceStarted = 1;
			}
		}
		else { //first time run
			startService(sensorServiceIntent);
			sensorServiceStarted = 1;
		}
		*/
		
		//startService(sensorServiceIntent);
		//TODO: on screen rotation change the service is restarted because
		//the activity is killed and resumed, therefore onCreate is called again
    }
    
    /*
    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        savedInstanceState.putInt(SERVICE_STARTED, sensorServiceStarted);
        super.onSaveInstanceState(savedInstanceState);
    }*/

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_ze_sense, menu);
        return true;
    }
    
    @Override
    public void onStart() {
    	super.onStart();
    	Log.i(TAG, "ZeSense main activity started");
    }
    
    @Override
    public void onResume() {
    	super.onResume();
    	Log.i(TAG, "ZeSense main activity resumed");
    }    
    
    @Override
    public void onPause() {
    	super.onPause();
    	Log.i(TAG, "ZeSense main activity paused");
    }
    
    @Override
    public void onStop() {
    	super.onStop();
    	Log.i(TAG, "ZeSense main activity stopped");
    }
    
    @Override
    public void onDestroy() {
    	
    	//Release wakelock
    	wakeLock.release();
    	
    	//stopService(GPSServiceIntent);
    	
    	// Kill CoAP service
    	stopService(coapServiceIntent);
    	
    	super.onDestroy();
    	Log.i(TAG, "ZeSense main activity destroyed");
    }
    

}
