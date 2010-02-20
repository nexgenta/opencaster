package tv.cineca.apps.yambo;

import java.awt.*;


public class TextBox {

	private int width = 0;
	private int height = 35;
	private int offsetX = 0;
	private int offsetY = 0;
	private String editStr = ""; /* The currently edited text */
	private char pendingChar = 0;  /* An character which has not yet been added to editStr */
	
	public TextBox(int _width, int _offsetX, int _offsetY) {
		width = _width;
		offsetX = _offsetX;
		offsetY = _offsetY;
	}

	public void paint(Graphics graphics) {
		graphics.setColor(Color.blue);
		graphics.fillRoundRect(offsetX, offsetY, width, height, height, height);
		graphics.setColor(new Color(0xff, 0xff, 0xff));
		graphics.fillRoundRect(offsetX + 2, offsetY + 2, width - 4, height - 4, (height - 4), (height- 4 ));
		graphics.setColor(Color.black);
		graphics.setFont(new Font("Tireasias", Font.PLAIN, 25));
		if (editStr.length() != 0) {
			// Draw the string
			graphics.drawString(editStr, offsetX + 10, offsetY + 25);
			// Draw the final pending char in red if any
			if (pendingChar != 0) {
				graphics.setColor(Color.red);
				graphics.drawString(String.valueOf(pendingChar), offsetX + 10 + graphics.getFontMetrics().stringWidth(editStr), offsetY + 25);
			}
		} else if (pendingChar != 0) {
			graphics.setColor(Color.red);
			graphics.drawString(String.valueOf(pendingChar), offsetX + 10, offsetY + 25);
		}
	}
	
	/* Changes the current pending char */	
	public void setPending(char c) {
		pendingChar = c;
	}
	
	/* Moves pending character to string */	
	public void finishPending() {
		if (pendingChar != 0) {
			editStr += pendingChar;
		}
		pendingChar = 0;
	}
	
	/* Remove the last character if exists */
	public void backSpace() {
		if (editStr.length() > 0) {
			editStr = editStr.substring(0, editStr.length() - 1);
		}
	}
	
	/* Add a space */
	public void addSpace() {
		editStr = editStr + ' ';
	}
	
	/* Get the current string */
	public String getString() {
		finishPending();
		return editStr;
	}
}

