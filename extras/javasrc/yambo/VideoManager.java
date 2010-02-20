package tv.cineca.apps.yambo;

import javax.tv.service.selection.*;
import javax.media.Player;
import javax.tv.media.AWTVideoSizeControl;
import javax.tv.media.AWTVideoSize;
import java.awt.Rectangle;
import java.awt.Dimension;
import javax.tv.locator.*;
import javax.tv.xlet.XletContext;
import java.util.*;

/* Change the video layout */
class VideoManager {

	private static String program = null;
	private static int X = 0;
	private static int Y = 0;
	private static int width;
	private static int height;
	private static int old_X;
	private static int old_Y;

	public static void configure(Vector Items) {

		program = null;
		for(int i = 0; i < Items.size(); i++) {
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("VideoItem")) {
				old_X = X;
				X = Integer.parseInt(item.LocationX);
				old_Y = Y;
				Y = Integer.parseInt(item.LocationY);
				width = Integer.parseInt(item.Width);
				height = Integer.parseInt(item.Height);
			} else if (item.Name.equals("ServiceItem")) {
				program = item.URL;
			}
		}

	}

	public static void zap(ServiceContext context) {

		if (program != null && context != null) {	
			try {
				System.out.println("VideoManager change program"); 
				LocatorFactory locatorFactory = LocatorFactory.getInstance();
				Locator[] locator = new Locator[1];
				locator[0] =locatorFactory.createLocator(program);
				context.select(locator);
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}	
	}

	public static void resize(ServiceContext context) {
		
		ServiceContentHandler[] handlers;
		AWTVideoSizeControl awtVideoSizeControl; 
		Dimension video_size;
		Rectangle video_src;
		Rectangle video_dst;
		
		if (context != null) {

			System.out.println("VideoManager resizing video"); 
			handlers = context.getServiceContentHandlers();
			for(int i=0; i < handlers.length ; i++) {
				if (handlers[i] instanceof ServiceMediaHandler) {
					javax.media.Player player = (javax.media.Player) handlers[i];
					awtVideoSizeControl = (AWTVideoSizeControl) player.getControl("javax.tv.media.AWTVideoSizeControl");
					video_size = awtVideoSizeControl.getSourceVideoSize(); 
					/* video_src = new Rectangle(old_X, old_Y,  old_X + video_size.width, old_Y + video_size.height); */
					video_src = new Rectangle(0, 0,  video_size.width, video_size.height);
					video_dst = new Rectangle(X, Y, X + width, Y + height);
					awtVideoSizeControl.setSize( new AWTVideoSize(video_src, video_dst)); 
					return;	
				}
			}
			
		}

	}

}
