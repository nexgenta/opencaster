package tv.cineca.apps.yambo;

import java.awt.*;
import javax.tv.xlet.*;
import java.io.*;
import java.lang.*;
import java.util.*;
import java.net.*;

/* Manage images loading and status */ 
public class Paint implements org.dvb.net.rc.ConnectionListener 
{

	public int X = -1;
	public int Y = -1;
	public boolean Visible = false;
	public String URL = null;
	public boolean unloadable = false;
	public Image image = null;

	public Paint() {
	
		unloadable = false;
		image = null;
		
	}

	public void Load(Wizard _wizard) {

		CGIURL CGI_URL = new CGIURL(URL);
		try {
			if (CGI_URL.carousel_id > -1) {
				System.out.println("Painter loading: " + URL);
				image = Toolkit.getDefaultToolkit().getImage((CarouselManager.getCarousel(CGI_URL.carousel_id)).getMountPoint() + "/" + CGI_URL.valued_path );
				Painter.Tracker.addImage(image, 0);
			} else if (CGI_URL.server_name == null) {
				System.out.println("Painter loading: " + URL);
				image = Toolkit.getDefaultToolkit().getImage( CGI_URL.valued_path );
				Painter.Tracker.addImage(image, 0);
			} else {
				RCManager.Connect(this);
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}

	}
	
	public void connectionChanged(org.dvb.net.rc.ConnectionRCEvent event) {

		System.out.println("Painter connection changed: " + event.toString() + " for:" + URL);
		if (event instanceof org.dvb.net.rc.ConnectionEstablishedEvent) {
			try {
				System.out.println("Painter loading: " + URL);
				image = Toolkit.getDefaultToolkit().getImage(new URL(URL));
				Painter.Tracker.addImage(image, 0);
				Painter.Tracker.waitForAll();
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
}

