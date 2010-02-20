package tv.cineca.apps.yambo;

import java.awt.*;
import java.awt.event.*;
import java.util.Vector;

public class KeyPad implements Runnable {

	private class Key {

		private int key = -1;

		public Key() { 
		}

		public void set(int i) { 
		    key = i; 
		}

		public int get() { 
		    return key; 
		}

	}

	public boolean stop = false;

	private static final int KP_WIDTH = 190;
	private static final int KP_HEIGHT = 237;
	private TextBox textBox = null;
	private KeyPadButton[] buttons;
	private Wizard wizard = null;
	private int timer = 0;
	private Key keyPressed = null;
	private Key keyActive = null;	
		
	public KeyPad(boolean isNumpad, int _offsetX, int _offsetY, Wizard _wizard) {

		/* Create buttons */
		wizard = _wizard;
		buttons = new KeyPadButton[10];
		buttons[0] = new KeyPadButton(0, "0", 69 +  _offsetX, 176 + _offsetY);
		buttons[1] = new KeyPadButton(1, "1", 9 + _offsetX, 5 + _offsetY);
		if (isNumpad) { 
			buttons[2] = new KeyPadButton(2, "2", 69 + _offsetX, 5 + _offsetY);
			buttons[3] = new KeyPadButton(3, "3", 129 + _offsetX, 5 + _offsetY);
			buttons[4] = new KeyPadButton(4, "4", 9 + _offsetX, 62 + _offsetY);
			buttons[5] = new KeyPadButton(5, "5", 69 + _offsetX, 62 + _offsetY);
			buttons[6] = new KeyPadButton(6, "6", 129 + _offsetX, 62 + _offsetY);
			buttons[7] = new KeyPadButton(7, "7", 9 + _offsetX, 119 + _offsetY);
			buttons[8] = new KeyPadButton(8, "8", 69 + _offsetX, 119 + _offsetY);
			buttons[9] = new KeyPadButton(9, "9", 129 + _offsetX, 119 + _offsetY);
		} else { 
			buttons[2] = new KeyPadButton(2, "abc2", 69 + _offsetX, 5 + _offsetY);
			buttons[3] = new KeyPadButton(3, "def3", 129 + _offsetX, 5 + _offsetY);
			buttons[4] = new KeyPadButton(4, "ghi4", 9 + _offsetX, 62 + _offsetY);
			buttons[5] = new KeyPadButton(5, "jkl5", 69 + _offsetX, 62 + _offsetY);
			buttons[6] = new KeyPadButton(6, "mno6", 129 + _offsetX, 62 + _offsetY);
			buttons[7] = new KeyPadButton(7, "pqrs7", 9 + _offsetX, 119 + _offsetY);
			buttons[8] = new KeyPadButton(8, "tuv8", 69 + _offsetX, 119 + _offsetY);
			buttons[9] = new KeyPadButton(9, "wxyz9", 129 + _offsetX, 119 + _offsetY);
		} 
		keyPressed = new Key();
		keyActive = new Key();

	}

	public void setOutput (TextBox _textBox) {
		textBox = _textBox;
	}

	public void paint(Graphics graphics) {
		for (int i=0; i<10; i++) { 
			buttons[i].paint(graphics);
		}
	}
	
	public void keyPressed (KeyEvent key) {

		synchronized (keyPressed) { /* key can also be changed by the running thread method! */		
			int code = key.getKeyCode();
			if ((code >= KeyEvent.VK_0) && (code <= KeyEvent.VK_9)) {
				keyPressed.set(code - KeyEvent.VK_0);
				if ((keyActive.get() != -1) && (keyActive.get() != keyPressed.get())) {
					buttons[keyActive.get()].unfocus();
					textBox.finishPending();
				}
				buttons[keyPressed.get()].press();
				textBox.setPending(buttons[keyPressed.get()].getChar());	
				System.out.println("KeyPad called repaint");
				wizard.repaint();
				keyActive.set(keyPressed.get());
				keyPressed.set(-1);
				timer = 10;
			} else if(code == KeyEvent.VK_LEFT) {
				textBox.finishPending();
				textBox.backSpace();
				if (keyActive.get() != -1) {
					buttons[keyActive.get()].unfocus();
				}
				System.out.println("KeyPad called repaint");
				wizard.repaint();
				keyActive.set(-1);
				keyPressed.set(-1);
				timer = 0;
			} else if(code == KeyEvent.VK_RIGHT) {
				textBox.finishPending();
				textBox.addSpace();
				if (keyActive.get() != -1) {
					buttons[keyActive.get()].unfocus();
				}
				System.out.println("KeyPad called repaint");
				wizard.repaint();
				keyActive.set( -1);
				keyPressed.set(-1);
				timer = 0;
			}
		}
	}

	public void run() {

		while (!stop) {

			/* Every times milliseconds wake up and check */
			try { 
				Thread.sleep(200); 
				synchronized (keyPressed) {
					if (timer >= 0) {
						timer--;
					}
					if (timer == 0) {
						/* If timer reaches 0 it means that it has gone at least timer x (sleep) milliseconds = seconds since 	*/
						/* the active key was pressed. The active key is now added to the edited string 			*/
						if (keyActive.get() != -1) {
							textBox.finishPending();
							buttons[keyActive.get()].unfocus();
							System.out.println("KeyPad called repaint");
							wizard.repaint();
							keyActive.set(-1);
							keyPressed.set(-1);
						}
					}
				}
			} 
			catch (Exception e) { 
				System.out.println(e); 
			}
		}
	}

}

