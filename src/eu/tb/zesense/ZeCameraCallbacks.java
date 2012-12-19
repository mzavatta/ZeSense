package eu.tb.zesense;

import android.hardware.Camera;

public class ZeCameraCallbacks implements Camera.PreviewCallback {

	long previousTime = 0, currentTime;
	
	@Override
	public void onPreviewFrame(byte[] arg0, Camera arg1) {
		// TODO Auto-generated method stub
		//System.out.println("Preview frame callback!");
		currentTime = System.nanoTime();
		//System.out.println("at time "+Long.toString(currentTime));
		//System.out.println("diff "+Long.toString(currentTime-previousTime));
		previousTime = currentTime;
	}

}
