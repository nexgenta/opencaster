package tv.cineca.apps.yambo;

import java.io.*;
import java.net.*;
import java.util.*;

/* it manages remote connections and always on connections in the same way */
public class RCManager {

	public static boolean connected = false;
	public static String user = null;
	public static String password = null;	
	public static String phone_number = null;

	private static RCResourceClient resource_client = null;
	private static org.dvb.net.rc.ConnectionRCInterface rc_connection_interface = null;
	private static org.dvb.net.rc.ConnectionListener listener = null;

	public static void configure(String _user, String _password, String _phone_number) {
		user = _user;
		password = _password;
		phone_number = _phone_number;
	}

	/* set the listener for the next http request, if already connected, connectionChanged callback is forced */ 
	public static void Connect(org.dvb.net.rc.ConnectionListener _listener) {

		listener = _listener;		
		if (!connected) {
			try {
				Connect(phone_number, user, password);
				connected = true;
			}
			catch(Exception ex) {
				ex.printStackTrace();
				listener = null;
			}
			
        	} else {
			listener.connectionChanged(new org.dvb.net.rc.ConnectionEstablishedEvent(resource_client));
		}
		
	}

	/* setup a dialup connection */
	private static void Connect(String magic_number, String username, String password) throws IOException {

		int interfaces_length;

		if(listener != null)  {
			rc_connection_interface = null;
			org.dvb.net.rc.RCInterfaceManager rc_interface_manager = org.dvb.net.rc.RCInterfaceManager.getInstance();
			org.dvb.net.rc.RCInterface rc_interfaces[] = rc_interface_manager.getInterfaces();

			if (rc_interfaces != null) {
				interfaces_length = rc_interfaces.length; 
    			} else {
				interfaces_length = 0;
				return;
    			}
			System.out.println("RC interfaces number is " + interfaces_length);
    	
			for(int i = 0; i < interfaces_length; i++) {
				if(rc_interfaces[i] instanceof org.dvb.net.rc.ConnectionRCInterface) {
    					rc_connection_interface = (org.dvb.net.rc.ConnectionRCInterface) rc_interfaces[i];
    					i = interfaces_length;
				}
    			}
			
			if (rc_connection_interface != null) {
				try {
					resource_client = new RCResourceClient();
					rc_connection_interface.reserve(resource_client, "what?");
					rc_connection_interface.setTarget(new org.dvb.net.rc.ConnectionParameters(magic_number, username, password));
					rc_connection_interface.addConnectionListener(listener);
					rc_connection_interface.connect();
        			}
        			catch(org.dvb.net.rc.PermissionDeniedException e) {
					throw new IOException("PermissionDeniedException:" + e.getMessage());
				}
				catch(org.dvb.net.rc.IncompleteTargetException e) {
					throw new IOException("IncompleteTargetException:" + e.getMessage());
				}
				catch(IOException e) {
					throw new IOException("IOException:" + e.getMessage());
				}
			} else {
				resource_client = new RCResourceClient();
				connected = true;
				listener.connectionChanged(new org.dvb.net.rc.ConnectionEstablishedEvent(resource_client)); /* probably a always on interface */
			}
		}
	}

	public static void Disconnect() throws IOException {

		if(rc_connection_interface != null) {
			try {
				rc_connection_interface.disconnect();
				if (listener != null) {
					rc_connection_interface.removeConnectionListener(listener);
				}
				rc_connection_interface.release();
				rc_connection_interface = null;
				resource_client = null;
				connected = false;
			}
			catch(org.dvb.net.rc.PermissionDeniedException e) {
				throw new IOException("PermissionDeniedException:" + e.getMessage());
			}
		}

	}

	public static String ReadHttp(String address) throws Exception {

		String response = null;

		if (connected) {
			try {
				URL url = new URL(address);
				HttpURLConnection http_connection = (HttpURLConnection)url.openConnection();
				http_connection.setDoInput(true);
				LineNumberReader reader = new LineNumberReader(new InputStreamReader (http_connection.getInputStream(), "UTF-8"));
				response = "";
				String new_line = reader.readLine(); 
				while(new_line != null) {
					try {
						response = response + new_line + "\n";
						new_line = reader.readLine(); 
					}
					catch (Exception e) {
						break;
					}
				}
				System.out.println("Http response: " + response);
				http_connection.disconnect();
        		}
			catch(Exception ex) {
				ex.printStackTrace();
				throw ex;
			}
		}
		
		return response;
	}

}

