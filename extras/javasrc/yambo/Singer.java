package tv.cineca.apps.yambo;

import javax.media.*;
import javax.tv.service.selection.*;
import javax.tv.xlet.*;
import java.util.*;
import java.awt.*;
import java.io.*;
import java.lang.*;
import org.havi.ui.*;
import org.dvb.si.*;
import org.dvb.dsmcc.*;
import org.davic.net.dvb.DvbLocator;

class Singer {

	static HSound song = null;
	static String URL = null;
	static boolean visible = false;

	public static void configure(Vector Items) {

		boolean foundItem = false;
		for(int i = 0; i < Items.size(); i++) {
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("AudioItem")) {
				foundItem = true;
				visible = item.Visible.equals("Yes");
				if (URL == null) { 
					load(item.URL);
				} else if (!URL.equals(item.URL)) {
					load(item.URL);
				} 
			} 
		}
		if (!foundItem) {
			song = null;
			URL = null;
			visible = false;
		}
	}

	private static void load(String URLtoLoad) {

		try {
			CGIURL CGI_URL = new CGIURL(URLtoLoad);
                       	if (CGI_URL.carousel_id > -1) {
                       		System.out.println("Singer loading: " + URLtoLoad);
				song = new HSound();
				song.load((CarouselManager.getCarousel(CGI_URL.carousel_id)).getMountPoint() + "/" + CGI_URL.valued_path); /* is flush necessary later ?*/
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

	}	


	public static void play() {
		if (song != null && visible == true) { 
			System.out.println("Singer play");
			song.play();
		} else {
			System.out.println("Singer play failed");
		}
	}

}

