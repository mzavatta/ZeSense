package eu.tb.zesense;

import android.content.Context;
import android.util.Log;

public class ZeRTPThread extends Thread {
	
	/* Leu/tb/zesense/ZeRTPThread; */
	
	private static final String TAG = "ZeSense";
	
	Context callerContext;
	
	ZeSMThread smThread;

	/* Getting commands from the streaming manager.. */
	public native int get_command(ZeRTPCommand command);
	
	/* Posting start, stop stream requests to the
	 * streaming manager.. */
	public native int post_request(/* Request characteristics.. */);
	
	public ZeRTPThread (Context context) {
		callerContext = context;
	}
	
	@Override
	public void run() {
		
		smThread = new ZeSMThread(callerContext);
		smThread.start();
		
		Log.i(TAG, "ZeRTPThread started running");
		
		// Run Streaming Manager thread
		
		while (true) {
			// Get RTP packet
			
			// Examine packet, tell something to the manager
			// though post_request()
			
			// Pick some commands and execute them get_command
			
			//loop
			
		}
		
		
	}
	
    static {
        System.loadLibrary("ZeSense");
    }
    
}
