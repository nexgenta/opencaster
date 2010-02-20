package tv.cineca.apps.yambo;

import java.awt.Color;
import javax.tv.xlet.*;
import java.io.*;
import java.lang.*;
import javax.tv.xlet.*;
import java.util.*;

/* Manage text resource status, carousel updates and text web links */
public class Print implements org.dvb.dsmcc.AsynchronousLoadingEventListener, org.dvb.dsmcc.ObjectChangeEventListener,  org.dvb.net.rc.ConnectionListener
{

	public int X = -1;
	public int Y = -1;
	public int size = -1;
	public Color color = null;
	public String text = null;
	public String URL = null;
	public boolean drawable = false;
	public boolean visible = false;

	private org.dvb.dsmcc.DSMCCObject file = null;
	private Wizard wizard = null;

	public Print() {
		drawable = false;
		text = null;
		URL = null;
		file = null;
	}

	public void Load(Wizard _wizard) {

		wizard = _wizard;
		CGIURL CGI_URL = new CGIURL(URL);
		try {
			if (CGI_URL.carousel_id > -1) {
			
				file = new org.dvb.dsmcc.DSMCCObject ((CarouselManager.getCarousel(CGI_URL.carousel_id)).getMountPoint(), CGI_URL.valued_path);
				file.asynchronousLoad(this);
				file.addObjectChangeEventListener(this);
				
			} else  if (CGI_URL.server_name == null) {
			
				file = new org.dvb.dsmcc.DSMCCObject (CGI_URL.valued_path);
				loadFile();
				
			} else {
			
				RCManager.Connect(this);
				
			} 
				
		} 
		catch (Exception e) {
			e.printStackTrace();
		}
	
	}

	public void receiveEvent(org.dvb.dsmcc.AsynchronousLoadingEvent event) {

		System.out.println("Printer synched: " + URL);
		loadFile();
		
	}
	
	public void loadFile() {
	
		try {
			LineNumberReader reader = new LineNumberReader(new InputStreamReader (new FileInputStream(file), "UTF-8"));
			text = "";
			String new_line = reader.readLine(); 
			while(new_line != null) {
				try {
					text = text + new_line + "\n";
					new_line = reader.readLine(); 
				}
				catch (Exception e) {
					break;
				}
			}
			drawable = true;
			System.out.println("Printer loadFile called repaint");
			wizard.repaint();
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
	}
	
	public void connectionChanged(org.dvb.net.rc.ConnectionRCEvent event) {

		System.out.println("connection changed: " + event.toString());
		if (event instanceof org.dvb.net.rc.ConnectionEstablishedEvent) {
			try {
				text = RCManager.ReadHttp(URL);
				drawable = true;
				System.out.println("Printer connectionChanged called repaint");
				wizard.repaint();
			}
			catch (Exception e) {
				e.printStackTrace();
			}			
		} 

	}

	public void removeObjectChangeEvent() {
		if (file != null) {
			file.removeObjectChangeEventListener(this); 
		}
	}

	public void receiveObjectChangeEvent(org.dvb.dsmcc.ObjectChangeEvent event) {
		System.out.println("Printer updated:" + URL);
		try {
			file.asynchronousLoad(this);
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}

	
}

