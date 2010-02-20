package tv.cineca.apps.yambo;

import javax.tv.xlet.*;
import java.util.*;
import java.awt.*;
import java.io.*;
import java.lang.*;

/* Manage images as a container*/
class Painter {

	public static Vector Paints = null;
	public static MediaTracker Tracker = null;

	public static void configure(Vector Items, Wizard _wizard) {

		if (Tracker == null) {
			Tracker = new MediaTracker(_wizard);
		}

		Vector newPaints = new Vector();
		for(int i = 0; i < Items.size(); i++) {
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("GraphicItem")) {
				Paint paint = new Paint();
				paint.X = Integer.parseInt(item.LocationX);
				paint.Y = Integer.parseInt(item.LocationY);
				paint.Visible = item.Visible.equals("Yes");
				paint.URL = item.URL;
				/* we keep an eye for images already loaded in this page and previous */
				for(int j = 0; j < newPaints.size(); j++) {
					Paint old_paint = (Paint) newPaints.elementAt(j);
					if (old_paint.URL.equals(item.URL)) {
						paint.image = old_paint.image;
						old_paint.unloadable = true;
					}
				}
				if (Paints != null) {
					for(int j = 0; j < Paints.size(); j++) {
						Paint old_paint = (Paint) Paints.elementAt(j);
						if (old_paint.URL.equals(item.URL)) {
							paint.image = old_paint.image;
							old_paint.unloadable = true;
						}
					}
				}
				if (paint.image == null) {
					paint.Load(_wizard);
				}
				newPaints.addElement(paint);
			} 
		}
		if (Paints != null) {
			for(int i = 0; i < Paints.size(); i++) {
				Paint old_paint = (Paint) Paints.elementAt(i);
				if (!old_paint.unloadable) {
					System.out.println("Flush image: " + old_paint.URL);
					// old_paint.image.flush();
					old_paint.image = null;
				}
			}
		}
		Paints = newPaints;

	}

	public static boolean ready() {
		
	    if (Paints == null) {
			System.out.println("Painter ready there are no paints");
			return true;
		} else if (Paints.size() == 0) {
			System.out.println("Painter ready there are no paints");
			return true;
		} else {
			for(int i = 0; i < Paints.size(); i++) {
				Paint paint = (Paint) Paints.elementAt(i);
				if (paint.image == null) {
					System.out.println("Painter not ready, error: paints are not loading");
					return false;
				}
			}
			try {
				Tracker.waitForAll();
				System.out.println("Painter ready");
				return true;
			}
			catch(Exception event) {
				event.printStackTrace();
				return false;
			}
		}
	}

	public static boolean paint(Graphics graphics) {

		System.out.println("Painter.paint:");
		if (Paints != null) {
			for(int i = 0; i < Paints.size(); i++) {
				Paint paint = (Paint) Paints.elementAt(i);
				if (paint.Visible) {
					try {
						System.out.println(paint.URL + " at: " + Integer.toString(paint.X) + "," +  Integer.toString(paint.Y));
						graphics.drawImage(paint.image, paint.X, paint.Y, null);
					}
					catch(Throwable event) {
						event.printStackTrace();
						return false;
					}
				} else {
					System.out.println("	" + paint.URL + " at: " + Integer.toString(paint.X) + "," +  Integer.toString(paint.Y) + " is invisible");
				}
			}
		}
		
		return true;
	}

}


