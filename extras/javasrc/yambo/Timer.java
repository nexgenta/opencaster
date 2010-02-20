package tv.cineca.apps.yambo;

import java.util.Vector;

/* second fine grain threaded timer */
public class Timer extends Thread {

	public int time = -1;
	public String URL = null;
	public int Page = -1;

	private Wizard wizard;

	public Timer(Vector Items, Wizard _wizard) {

		NamedItem item;	
		wizard = _wizard;
		time = -1;
		for(int i = 0; i < Items.size(); i++) {
			item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("TimeItem")) {
				time = Integer.parseInt(item.Time);
				URL = item.URL;
				Page = Integer.parseInt(item.URLPage);
				return;
			}
		}
	}


	public void run() {
		try {
			sleep(time * 1000);
			wizard.ChangePage(URL, Page);	
		}
		catch (InterruptedException e) {
			System.out.println("Timer interrupted");
			;	
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
}
