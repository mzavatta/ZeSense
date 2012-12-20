package eu.tb.zesense;

import java.io.IOException;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.hardware.Camera;
import android.util.Log;
import android.view.Menu;
import android.widget.Button;
import android.widget.FrameLayout;
import android.os.Process;

public class ZeSense extends Activity {

	private static final String TAG = "ZeSenseSensorService";
	//static final String SERVICE_STARTED = "service_started";
	
	Camera camera;
	ZeCameraPreview zePreview;
	Button buttonClick;
	//int sensorServiceStarted = 0;
    

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ze_sense);
        
		zePreview = new ZeCameraPreview(this);
		((FrameLayout) findViewById(R.id.zePreview)).addView(zePreview);
       
		// Start our SensorService
		Intent sensorServiceIntent = new Intent(this, ZeSenseSensorService.class);
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
		startService(sensorServiceIntent);
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
    }
    
    @Override
    public void onResume() {
    	super.onResume();
    }    
    
    @Override
    public void onPause() {
    	super.onPause();
    }
    
    @Override
    public void onStop() {
    	super.onStop();
    	Log.i(TAG, "ZeSense main activity stopped");
    }
    
    @Override
    public void onDestroy() {
    	super.onDestroy();
    	Log.i(TAG, "ZeSense main activity destroyed");
    }
    

}
