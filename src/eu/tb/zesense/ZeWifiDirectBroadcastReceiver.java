package eu.tb.zesense;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import android.net.wifi.p2p.*;
import android.net.wifi.p2p.WifiP2pManager.*;
import android.util.Log;

public class ZeWifiDirectBroadcastReceiver extends BroadcastReceiver {

    private WifiP2pManager mManager;
    private Channel mChannel;
    private ZeSense mActivity;

    public ZeWifiDirectBroadcastReceiver(WifiP2pManager manager, Channel channel, ZeSense activity) {
        super();
        this.mManager = manager;
        this.mChannel = channel;
        this.mActivity = activity;
    }
	
	@Override
	public void onReceive(Context context, Intent intent) {
		// TODO Auto-generated method stub
		
		String action = intent.getAction();
		

        if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
            // Check to see if Wi-Fi is enabled and notify appropriate activity
        	Log.i("ZeSense", "WifiDirectAP broadcast received, P2P_STATE");
        	int extra = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE,-1);
        	if (extra == WifiP2pManager.WIFI_P2P_STATE_DISABLED) Log.i("ZeSense", "WifiDirectAP New state -> Disabled");
        	else if (extra == WifiP2pManager.WIFI_P2P_STATE_ENABLED) Log.i("ZeSense", "WifiDirectAP New state -> Enabled");
        	else Log.w("ZeSense", "WifiDirectAP Cannot recognize new state "+Integer.toString(extra));
        	
        } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
            // Call WifiP2pManager.requestPeers() to get a list of current peers
        	Log.i("ZeSense", "WifiDirectAP broadcast received, P2P_PEERS");
        	
        	/*
        	mManager.discoverPeers(channel, new WifiP2pManager.ActionListener() {
        	    @Override
        	    public void onSuccess() {
        	        ...
        	    }

        	    @Override
        	    public void onFailure(int reasonCode) {
        	        ...
        	    }
        	});
        	*/
        	
        } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
            // Respond to new connection or disconnections
        	Log.i("ZeSense", "WifiDirectAP broadcast received, P2P_CONNECTION");
        	
        } else if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
            // Respond to this device's wifi state changing
        	Log.i("ZeSense", "WifiDirectAP broadcast received, P2P_THIS_DEVICE");
        	
        }

	}

}
