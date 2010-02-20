package tv.cineca.apps.yambo;

import javax.tv.xlet.*;
import java.util.*;
import java.awt.Color;
import java.awt.*;
import java.awt.event.*;

class InputManager {

	public static int textX = -1;
	public static int textY = -1;
	public static int padX = -1;
	public static int padY = -1;
	public static int Width = -1;
	public static String URL = null;
	public static int Page = -1;
	public static boolean NumberOnly = false;
	public static String Variable = null;

	private static KeyPad keypad = null;
	private static TextBox textbox = null;

	public static void configure(Vector Items, Wizard _wizard) {

		URL = null;
		padX = -1;
		padY = -1;
		if (keypad != null) {
			keypad.stop = true;
			keypad = null;
			textbox = null;
		}
		for(int i = 0; i < Items.size(); i++) {
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("InputItem")) {
				textX = Integer.parseInt(item.LocationX);
				textY = Integer.parseInt(item.LocationY);
				Width = Integer.parseInt(item.Width);
				NumberOnly = item.InputFlags.equals("Number");
				Variable = item.InputTag;
				URL = item.URL;
				Page = Integer.parseInt(item.URLPage);				
			} else if (item.Name.equals("PadHelpItem")) {
				padX = Integer.parseInt(item.LocationX);
				padY = Integer.parseInt(item.LocationY);
			}
		}

		if (URL != null) {
			keypad = new KeyPad(NumberOnly, padX, padY, _wizard);
			textbox = new TextBox(Width * 20, textX, textY); // more or less the size of each char
			keypad.setOutput(textbox);
			new Thread(keypad).start();
		}

	}

	public static boolean paint(Graphics graphics) {

		System.out.println("InputManager.paint");
		if (keypad != null && textbox != null) {
			keypad.paint(graphics);
			textbox.paint(graphics);
			return true;
		}
		return false;

	}

	public static String getURL() {

		if (URL.indexOf("?") > 0) {
			return	(URL + "&" + Variable + "=" + textbox.getString());
		} else {
			return	(URL + "?" + Variable + "=" + textbox.getString());
		}

	}

	public static void keyPressed(KeyEvent key)
	{
		System.out.println("Wizard InputManager key pressed: " + key.toString());
		switch(key.getKeyCode())
		{
			case KeyEvent.VK_LEFT: 		// left, 37 
			case KeyEvent.VK_RIGHT:		// right, 39 
			case KeyEvent.VK_0: 		// 0, 48
			case KeyEvent.VK_1: 		// 1, 49
			case KeyEvent.VK_2: 		// 2, 50
			case KeyEvent.VK_3: 		// 3, 51
			case KeyEvent.VK_4: 		// 4, 52
			case KeyEvent.VK_5: 		// 5, 53
			case KeyEvent.VK_6: 		// 6, 54
			case KeyEvent.VK_7: 		// 7, 55
			case KeyEvent.VK_8: 		// 8, 56 
			case KeyEvent.VK_9: 		// 9, 57
				keypad.keyPressed(key);	
			default:
				break;
		}
	}

}

