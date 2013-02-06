package eu.tb.zesense;

public class ZeJNIHub {
	
    public native static void ze_samplingnative();
    
    public native static int ze_coap_server_root();
	
    public static void loadNativeLibraries() {
    	System.loadLibrary("ZeSense");
    }
    
    static {
        System.loadLibrary("ZeSense");
    }

}