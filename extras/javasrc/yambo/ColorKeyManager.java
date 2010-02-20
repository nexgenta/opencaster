package tv.cineca.apps.yambo;

import java.awt.*;
import java.util.Vector;

public class  ColorKeyManager {

	public static String RedURL = null;
	public static int RedPage = 0;
	public static String GreenURL = null;
	public static int GreenPage = 0;
	public static String BlueURL = null;
	public static int BluePage = 0;
	public static String YellowURL = null;
	public static int YellowPage = 0;

	public static void configure(Vector Items) {

		RedURL = null;
		RedPage = 0;
		GreenURL = null;
		GreenPage = 0;
		BlueURL = null;
		BluePage = 0;
		YellowURL = null;
		YellowPage = 0;

		/* Get URLS from color keys */
		for(int i = 0; i < Items.size(); i++) {
		
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("RedKeyItem") && RedURL == null) {
				RedURL = item.URL;
				RedPage = Integer.parseInt(item.URLPage);				
			} else if (item.Name.equals("GreenKeyItem") && GreenURL == null) {			
				GreenURL = item.URL;
				GreenPage = Integer.parseInt(item.URLPage);				
			} else if (item.Name.equals("BlueKeyItem") && BlueURL == null) {			
				BlueURL = item.URL;
				BluePage = Integer.parseInt(item.URLPage);				
			} else if (item.Name.equals("YellowKeyItem") && YellowURL == null) {
				YellowURL = item.URL;
				YellowPage = Integer.parseInt(item.URLPage);				
			}
			
		}
	}
}

