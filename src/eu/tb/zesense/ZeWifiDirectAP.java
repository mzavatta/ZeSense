package eu.tb.zesense;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.net.wifi.p2p.*;
import android.net.wifi.p2p.WifiP2pManager.*;
import android.util.Log;

/*
 * Android Developers guide
 * http://developer.android.com/guide/topics/connectivity/wifip2p.html
 * 
 * Wifi concepts
 * 
 * Access Point (AP) mode:
 * In a managed wireless network the Access Point acts as the Master device.
 * It holds the network together by managing and maintaining lists of associated STAs.
 * It also manages security policies. The network is named after the MAC-Address (BSSID) of the AP.
 * The human readable name for the network, the SSID, is also set by the AP. 
 * 
 * Ad-Hoc (IBSS) mode:
 * The Ad-Hoc mode aka IBBS (Independent Basic Service Set) mode,
 * is used to create a wireless network without the need of having an AP in the network.
 * Each station in an IBSS network is managing the network itself.
 * Ad-Hoc is useful for connecting two or more computers to each other
 * when no (useful) AP is around for this purpose. 
 */

/*
 * Incomplete, unsafe but working implementation of a manager for Wifi Direct
 * XXX: It is claimed to work with legacy devices but apparently it does not! --
 * -- see http://www.wi-fi.org/knowledge-center/faq/will-wi-fi-direct-work-legacy-devices
 */

public class ZeWifiDirectAP {
	
	// Important for Context, Looper and to register a BroadcastReceiver
	ZeSense callingActivity;
	
	// Wifi infrastructure
	WifiP2pManager mManager;
	Channel mChannel;
	BroadcastReceiver mReceiver;
	IntentFilter mIntentFilter;
	WifiP2pGroup group;
	
	public ZeWifiDirectAP (ZeSense callingActivity) {
		this.callingActivity = callingActivity;
	}
	
	public void initialize() {
		
		Log.i("ZeSense","WifiDirectAP initializing");
		
		// Fetch Manager
		mManager = (WifiP2pManager) callingActivity.getSystemService(Context.WIFI_P2P_SERVICE);
		
		// Initialize Manager
		mChannel = mManager.initialize(callingActivity.getApplicationContext(), callingActivity.getMainLooper(), null);
		if (mChannel == null) {
			Log.w("ZeSense", "mChannel WifiDirectAP is null");
		}
		
		// To receive Wifi state changes notifications
		mReceiver = new ZeWifiDirectBroadcastReceiver(mManager, mChannel, callingActivity);
	    mIntentFilter = new IntentFilter();
	    mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
	    mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
	    mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
	    mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
	    callingActivity.registerReceiver(mReceiver, mIntentFilter);
	}

	public void startWifiDirectAP() {
		
		// I think this should create a Infrastructure BSS (Basic Service Set)
		mManager.createGroup(mChannel, new WifiP2pManager.ActionListener() {
			@Override
			public void onFailure(int reason) {
				// TODO Auto-generated method stub
				if (reason == WifiP2pManager.P2P_UNSUPPORTED) Log.w("ZeSense", "WifiDirectAP cannot create AP group, p2p unsupported");
				else if (reason == WifiP2pManager.BUSY) Log.w("ZeSense", "WifiDirectAP cannot create AP group, busy");
				else if (reason == WifiP2pManager.ERROR) Log.w("ZeSense", "WifiDirectAP cannot create AP group, error");
			}
			@Override
			public void onSuccess() {
				// TODO Auto-generated method stub
				Log.i("ZeSense", "WifiDirectAP succes created Access Point group");
				getGroupInfo();
			}
		});
	}
	
	private void getGroupInfo() {
		mManager. requestGroupInfo(mChannel, new WifiP2pManager.GroupInfoListener() {
			@Override
			public void onGroupInfoAvailable(WifiP2pGroup group) {
				// TODO Auto-generated method stub
				Log.i("ZeSense", group.toString());
			} 
		});
	}
	
	public void stopWifiDirectAP() {
		mManager.removeGroup(mChannel, new WifiP2pManager.ActionListener() {
			@Override
			public void onFailure(int reason) {
				// TODO Auto-generated method stub
				Log.w("ZeSense", "WifiDirectAP cannot stop Wifi Direct group, reason:"+Integer.toString(reason));
			}
			@Override
			public void onSuccess() {
				// TODO Auto-generated method stub
				Log.w("ZeSense", "WifiDirectAP success stopping Wifi Direct group");
			}
		});
		callingActivity.unregisterReceiver(mReceiver);
	}
	
	// To be called in the activity's onPause()
	public void pauseWifiDirectAP() {
		callingActivity.unregisterReceiver(mReceiver);
	}
	
	// To be called in the activity's onResume()
	public void resumeWifiDirectAP() {
		callingActivity.registerReceiver(mReceiver, mIntentFilter);
	}
}
