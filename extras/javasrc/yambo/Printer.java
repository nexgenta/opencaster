package tv.cineca.apps.yambo;

import javax.tv.xlet.*;
import java.util.*;
import java.awt.Color;
import java.awt.*;

/* Manage text resource that can chage dinamically */
class Printer {

	public static Vector Prints = null;

	public static void configure(Vector Items, Wizard _wizard) {

		if (Prints != null) {
			for(int i = 0; i < Prints.size(); i++) {
				Print print = (Print) Prints.elementAt(i);
				print.removeObjectChangeEvent();			
			}
		}

		Vector newPrints = new Vector();	
		for(int i = 0; i < Items.size(); i++) {
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("TextItem") || item.Name.equals("StaticTextItem")) {
				Print print = new Print();
				print.X = Integer.parseInt(item.LocationX);
				print.Y = Integer.parseInt(item.LocationY);
				print.size = Integer.parseInt(item.Size);
				if(item.Color.equals("White")) {
					print.color = Color.white; 
				} else if (item.Color.equals("Black")) {
					print.color = Color.black; 
				} else if (item.Color.equals("Red")) {
					print.color = Color.red; 
				} else if (item.Color.equals("Blue")) {
					print.color = Color.blue; 
				} else if (item.Color.equals("Yellow")) {
					print.color = Color.yellow; 
				} else if (item.Color.equals("Green")) {
					print.color = Color.green; 
				}
				print.visible = item.Visible.equals("Yes"); 
				print.URL = item.URL;
				print.text = item.Text;
				if (print.URL != null) {
					if ((Prints != null) && (Prints.size() > 0)) {
						for(int j = 0; (j < Prints.size()) && (print.text == null); j++ ) {
							Print old_print = (Print) Prints.elementAt(j);
							if (old_print.URL != null) {
								if (old_print.URL.equals(print.URL)) {
									print.text = old_print.text;
								}
							}
						}
					}
				}
				newPrints.addElement(print);
			}
		}
		Prints = newPrints;
		for(int i = 0; i < Prints.size(); i++) {
			Print print = (Print) Prints.elementAt(i);
			if (print.text != null) {
				print.drawable = true;
			} else {
				print.Load(_wizard);
			}
		}
	}

	public static boolean ready() {

		if (Prints == null) {
			System.out.println("Printer ready anything to print");
			return true;
		} else if (Prints.size() == 0) {
			System.out.println("Printer ready anything to print");
			return true;
		}

		for(int i = 0; i < Prints.size(); i++) {
			Print print = (Print) Prints.elementAt(i);
			if (!print.drawable) {
				System.out.println("Printer not ready, error: prints are not loading");
				return false;
			}
		}
		System.out.println("Printer ready");
		return true;	
	}

	
	public static boolean paint(Graphics graphics) {
 
		System.out.println("Printer.paint:");

		if (Prints != null) {
			for(int i = 0; i < Prints.size(); i++) {
				Print print = (Print) Prints.elementAt(i);
				if (print.visible) {
					graphics.setFont(new Font("Tiresias", Font.PLAIN, print.size));
					graphics.setColor(print.color);
					try {
						System.out.println(print.text + " at: "  + Integer.toString(print.X) + "," +  Integer.toString(print.Y));
						StringTokenizer st = new StringTokenizer(print.text, "\n");
						int j = 0;
						while (st.hasMoreTokens()) {
							graphics.drawString(st.nextToken(), print.X, print.Y + j * (print.size + 3));
							j++;
						}
					}
					catch(Throwable event) {
						event.printStackTrace();
					}
				}
			}
		}
		return true;
	
	}
	
}

