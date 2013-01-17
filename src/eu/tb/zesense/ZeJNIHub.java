package eu.tb.zesense;

public class ZeJNIHub {
	
    public native static void ze_samplingnative();
    
    public native static void ze_coap_server_main();
	
    public static void loadNativeLibraries() {
    	System.loadLibrary("ZeSense");
    }
    
    static {
        System.loadLibrary("ZeSense");
    }

}
