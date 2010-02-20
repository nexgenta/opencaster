package tv.cineca.apps.yambo;

import javax.tv.xlet.*;
import java.io.*;
import java.lang.*;
import javax.tv.xlet.*;
import java.util.*;

/* It usefull to get files from a carousel different from where the java class are */
public class CarouselManager
{
	private static Hashtable Carousels = null;
	private static org.davic.net.dvb.DvbLocator locator = null;

	public static void configure(org.davic.net.dvb.DvbLocator _locator) {
	
		locator = _locator; 
		Carousels = new Hashtable();
		
	}

	public static org.dvb.dsmcc.ServiceDomain getCarousel(int carousel_id) { 
	
		Object value = null;
		org.dvb.dsmcc.ServiceDomain carousel = null;

		value = Carousels.get((Object)Integer.toString(carousel_id));
		if (value != null) { 
		
			return ((org.dvb.dsmcc.ServiceDomain)value); 	
			
		} else {
		
			carousel = new org.dvb.dsmcc.ServiceDomain();
			try  {
				carousel.attach((org.davic.net.Locator)locator, carousel_id);
			} 
			catch (Exception e) {
 				e.printStackTrace();
				return null;
			}
			Carousels.put((Object)Integer.toString(carousel_id), (Object)carousel);
			return carousel; 
			
		}
	}

}

