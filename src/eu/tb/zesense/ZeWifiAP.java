package eu.tb.zesense;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;

import android.app.Activity;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.util.Log;
import android.widget.Toast;

/*
 * Starts and stops a WIfi Access Point
 * Android APIs for this are marked as @hide and therefore not
 * included in the .jar referenced for building but only in the
 * real deployed OSs (emulator too)
 * although their attributes and methods are public
 * so it is anyway possible to access via reflection
 */

public class ZeWifiAP {
	
	// To be kept in sych with @hided definitions in WifiManager.java
    public static final String WIFI_AP_STATE_CHANGED_ACTION =
        "android.net.wifi.WIFI_AP_STATE_CHANGED";
    public static final String EXTRA_WIFI_AP_STATE = "wifi_state";
    public static final String EXTRA_PREVIOUS_WIFI_AP_STATE = "previous_wifi_state";
    public static final int WIFI_AP_STATE_DISABLING = 10;
    public static final int WIFI_AP_STATE_DISABLED = 11;
    public static final int WIFI_AP_STATE_ENABLING = 12;
    public static final int WIFI_AP_STATE_ENABLED = 13;
    public static final int WIFI_AP_STATE_FAILED = 14;
    
	private static final String TAG = "ZeSense";
	
	// Important for Context
	ZeSense callingActivity;
	
	// Wifi infrastructure
	WifiManager wifiManager;
	
	// Reflected methods
	Method setWifiApEnabled, isWifiApEnabled, getWifiApState;
	
	// Wifi network configuration
	WifiConfiguration netConfig;
	
	public ZeWifiAP (ZeSense callingActivity) {
		this.callingActivity = callingActivity;
	}
	
	// Defaults to open authentication
	public boolean initialize(String ssid) {
		
		// Fetch service from the system
		wifiManager = (WifiManager) callingActivity.getSystemService(Activity.WIFI_SERVICE);
		
        // Turn off normal Wifi to close any possible current connection
		if(wifiManager.isWifiEnabled())
    	{
            wifiManager.setWifiEnabled(false);          
        }
		
		// Reflection
		Method[] wmMethods = wifiManager.getClass().getDeclaredMethods();   
        boolean methodSetEnabledFound = false, methodIsEnabledFound = false, methodGetStateFound = false;
        for(Method method: wmMethods) {
	        if(method.getName().equals("setWifiApEnabled")) {
	        	methodSetEnabledFound = true;
	        	setWifiApEnabled = method;
	        }
	        if(method.getName().equals("isWifiApEnabled")) {
	        	methodIsEnabledFound = true;
	        	isWifiApEnabled = method;
            }
	        if(method.getName().equals("getWifiApState")) {
	        	methodGetStateFound = true;
	        	getWifiApState = method;
            }
        }
        if((!methodSetEnabledFound) || (!methodIsEnabledFound)) {
            Log.w(TAG, "ZeWifiAP cannot find methods for reflection");
            return false;
        }
        
        // Configure network
        netConfig = new WifiConfiguration();
        netConfig.SSID = ssid; 
    	netConfig.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
    	
    	return true;
	}
	
	// Return true if successfully started, false otherwise
	public boolean startAP() {
		
    	try {
    		
    		// Enable AP
    		/* 
    		 * From WifiManager.java
    		 * If the radio is already running in
    		 * AP mode, update the new configuration
    		 * Note that starting in access point mode disables station
    		 * mode operation
    		 */
    		boolean success = (Boolean) setWifiApEnabled.invoke(wifiManager, netConfig, true);
    		if (!success) {
    			Log.w(TAG, "ZeWifiAP cannot enable AP");
    		}
    		
    		/*
    		// Return true if Wi-Fi AP is enabled
    		while(!(Boolean)isWifiApEnabled.invoke(wifiManager)) { };
    		*/
    		
    		// Fetch the state as a confirmation
    		int state = (Integer) getWifiApState.invoke(wifiManager);
    		
    		// If enabling, wait..
    		// TODO: cannot loop forever!
    		while (state == WIFI_AP_STATE_ENABLING) {
    		
    		}
    		
    		if(state == WIFI_AP_STATE_ENABLED) {
    			Log.i(TAG, "ZeWifiAP AP created, SSID: "+netConfig.SSID);
    			getLocalIpAddressString();
    			return true;
    		}
            else {
            	Log.w(TAG, "ZeWifiAP AP creation failed");
            	return false;
            }

        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    	
    	return false;
	}
	
	public  void getLocalIpAddressString() {
		
		try {
			for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
				NetworkInterface intf = en.nextElement();
				for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
					InetAddress inetAddress = enumIpAddr.nextElement();
					if (!inetAddress.isLoopbackAddress()) {
	                    // return inetAddress.getHostAddress().toString();
						Toast.makeText(callingActivity.getApplicationContext(), inetAddress.getHostAddress().toString(),
								Toast.LENGTH_SHORT).show();

					}
				}
	     	 }
		} catch (Exception ex) {
	       Log.e("IPADDRESS", ex.toString());
	    }
	   // return null;
	}
	
}
