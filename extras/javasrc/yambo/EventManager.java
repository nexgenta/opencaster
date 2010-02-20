package tv.cineca.apps.yambo;

import java.io.*;
import java.awt.*;
import java.util.*;
import javax.tv.xlet.*;
import javax.tv.graphics.*;
import org.dvb.dsmcc.*;
import javax.tv.service.selection.*;
import javax.media.Player;
import javax.tv.media.AWTVideoSizeControl;
import javax.tv.media.AWTVideoSize;
import java.awt.Rectangle;
import java.awt.Dimension;
import javax.tv.locator.*;
import javax.tv.xlet.XletContext;
import java.util.*;

/* wait for stream event messages, if received change page to the event data */
class EventManager {

	private static Wizard wizard = null;

	public static void configure(Vector Items, Wizard _wizard) {

		wizard = _wizard;
		
		for(int i = 0; i < Items.size(); i++) {
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("EventItem")) {
				try {
					CGIURL CGI_URL = new CGIURL(item.URL);
					if (CGI_URL.carousel_id > -1) {
						org.dvb.dsmcc.DSMCCStreamEvent stream_event_object = new org.dvb.dsmcc.DSMCCStreamEvent((CarouselManager.getCarousel(CGI_URL.carousel_id)).getMountPoint() + "/" + CGI_URL.valued_path);
						String[] event_list = stream_event_object.getEventList();
						for (int j = 0; j < event_list.length; j++) {
							System.out.println("EventManager:event " + j + " name: " + event_list[j]);
							System.out.println("EventManager:subscribe to event:" + event_list[j]);
							stream_event_object.subscribe(event_list[j], wizard);			
						}
					}
				}			
				catch(Exception e) {
					System.out.println("EventManager exception: " + e);
				}
			}
		}
		
	}


	public static void receiveEvent(org.dvb.dsmcc.StreamEvent event) {
	
		String event_print = "EventId: " + event.getEventId() + "\nEventName: " + event.getEventName() + "\nEventNPT: " + event.getEventNPT() + "\nEventData: "; 
		/*
		byte[] data = event.getEventData();
		String URL = "";
		for (int i = 0; i < data.length; i++) {
			URL += data[i];
		}
		*/
		String URL = new String(event.getEventData());
		System.out.println(event_print + URL);
		wizard.ChangePage(URL, 1);	

	}


}
