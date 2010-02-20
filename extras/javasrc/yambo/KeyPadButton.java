package tv.cineca.apps.yambo;

import java.awt.*;

public class KeyPadButton {

	/* Key size */
	public static int KB_WIDTH = 55;
	public static int KB_HEIGHT = 52;
	
	private int keyNumber = 0;
	private static Font keyNumberFont = null;
	private char[] smallChars = null;
	private int smallChCount = 0;
	private int activeSmallChar = -1;
	private static Font smallChFont = null;
	private boolean focused = false;
	private int offsetX = 0;
	private int offsetY = 0;

	/* Create KeypadButton */
	public KeyPadButton(int keyNum, String keyChars, int _offsetX, int _offsetY) {
		keyNumber = keyNum;
		keyNumberFont = new Font("Tiresias",Font.PLAIN,25);
		smallChars = keyChars.toCharArray();
		smallChCount = keyChars.length();
		smallChFont = new Font("Tiresias", Font.PLAIN,18);
		offsetX = _offsetX;
		offsetY = _offsetY;
	}

	public void paint(Graphics graphics) {
	
		/* Draw button */	
		if (focused) {
			graphics.setColor(Color.red);
		} else {
			graphics.setColor(new Color(0x2a, 0x2a, 0x2a));
		}
		graphics.fillRoundRect(offsetX, offsetY, KB_WIDTH, KB_HEIGHT, 10, 10);
		graphics.setColor(Color.black);
		graphics.fillRoundRect(offsetX + 2, offsetY + 2, KB_WIDTH - 4, KB_HEIGHT - 4, 10, 10);
		
		/* Draws small characters */
		graphics.setFont(smallChFont);
		int charX = (KB_WIDTH - graphics.getFontMetrics().stringWidth(String.valueOf(smallChars))) / 2;
		for (int i=0; i < smallChCount; i++) {
			if (i == activeSmallChar && focused) {
				graphics.setColor(Color.white); 
			} else { 
				graphics.setColor(Color.green);
			}
			graphics.drawString(String.valueOf(smallChars[i]), offsetX + charX, offsetY + 46);
			charX += graphics.getFontMetrics().charWidth(smallChars[i]);
		}
		
		/* Draw key number */
		graphics.setFont(keyNumberFont);
		graphics.setColor(Color.white);
		graphics.drawString(String.valueOf(keyNumber), offsetX + ((KB_WIDTH - graphics.getFontMetrics().stringWidth(String.valueOf(keyNumber))) / 2), offsetY + 26);
	}

	public void press() {
		focused = true;
		activeSmallChar = (activeSmallChar + 1) % smallChCount;
	}
		
	public void unfocus() {
		focused = false;
		activeSmallChar = -1;
	}

	public char getChar() {
		if (smallChars != null && activeSmallChar != -1) { 
		    return smallChars[activeSmallChar];
		}
		return 0;
	}
}

