package eu.tb.zesense;

import java.io.IOException;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.hardware.Camera;
import android.view.Menu;
import android.widget.Button;
import android.widget.FrameLayout;

public class ZeSense extends Activity {

	private static final String TAG = "ZeSenseSensorService";
	
	Camera camera;
	ZeCameraPreview zePreview;
	Button buttonClick;
    

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ze_sense);
        
		//zePreview = new ZeCameraPreview(this);
		//((FrameLayout) findViewById(R.id.zePreview)).addView(zePreview);
       
		// Start our SensorService
		Intent sensorServiceIntent = new Intent(this, ZeSenseSensorService.class);
		startService(sensorServiceIntent);
    }

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
    }
    
    @Override
    public void onDestroy() {
    	super.onDestroy();
    }
    

}
