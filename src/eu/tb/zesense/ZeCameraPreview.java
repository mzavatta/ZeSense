package eu.tb.zesense;

import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.media.MediaRecorder;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class ZeCameraPreview extends SurfaceView implements SurfaceHolder.Callback {

	SurfaceHolder mHolder;
	public Camera camera;
	public MediaRecorder mediaRecorder;

	
	public ZeCameraPreview(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
		
		mHolder = getHolder();
		mHolder.addCallback(this);
		//mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	}

	@Override
	public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
		// TODO Auto-generated method stub
		System.out.println("SurfaceChanged Callback");
		
		Camera.Parameters parameters = camera.getParameters();
		
		parameters.setPreviewSize(arg2,arg3);
		
		List<int[]> supportedFPS = parameters.getSupportedPreviewFpsRange();
		
		for (int i=0; i<supportedFPS.size(); i++) System.out.println(Integer.toString(supportedFPS.get(i)[0])+" "+
				""+Integer.toString(supportedFPS.get(i)[1]));
		 	
		//parameters.setPreviewFpsRange(min, max) must be one of the couples in supportedFPS		
	
		camera.setParameters(parameters);
		
		camera.setPreviewCallback(new ZeCameraCallbacks());
		
		camera.startPreview();
	}

	@Override
	public void surfaceCreated(SurfaceHolder arg0) {
		// TODO Auto-generated method stub
		System.out.println("SurfaceCreated Callback");
		
		camera = Camera.open(findFrontFacingCamera());
		
		try {
			camera.setPreviewDisplay(arg0);			
		} catch (IOException e) {
			e.printStackTrace();			
		} finally { }
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder arg0) {
		// TODO Auto-generated method stub
		System.out.println("SurfaceDestroyed Callback");
		camera.stopPreview();
		camera.setPreviewCallback(null);
		camera.release();
	}
	
    private int findFrontFacingCamera() {
        int cameraId = -1;
        // Search for the back facing camera
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; i++) {
          CameraInfo info = new CameraInfo();
          Camera.getCameraInfo(i, info);
          if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
            System.out.println("Camera found");
            cameraId = i;
            break;
          }
        }
        return cameraId;
      }

}
