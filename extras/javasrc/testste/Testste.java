import java.io.*;
import java.awt.*;
import java.util.*;
import javax.tv.xlet.*;
import javax.tv.graphics.*;
import org.dvb.dsmcc.*;


public class Testste extends Component implements Xlet, StreamEventListener {

	private static XletContext context = null;
	private static Container rootContainer = null;
	private static String toDraw = "waiting...";
	private static DSMCCStreamEvent stream_event_object = null;

	public void initXlet(XletContext xletContext) throws XletStateChangeException { 
	
		context = xletContext;

		try {
			rootContainer = TVContainer.getRootContainer(xletContext);
			rootContainer.setLayout(new GridLayout(0,1));
			rootContainer.setSize(new Dimension(720, 576));
			rootContainer.setVisible(true);
		}
		catch (Exception e) {
			e.printStackTrace();
		}

		
	}

	public void startXlet() throws XletStateChangeException {
	
		rootContainer.add(this);
		setVisible(true);
		rootContainer.validate();

		toDraw = "start xlet";
		repaint();
		
		
		try {
		
/*			stream_event_object = new DSMCCStreamEvent("test.event"); */
			DSMCCObject file_object = new DSMCCObject("test.event");
			file_object.synchronousLoad();
			stream_event_object = new DSMCCStreamEvent(file_object);
			String[] event_list = stream_event_object.getEventList();
			int i = 0;
			for (i = 0; i < event_list.length; i++) {
				System.out.println("Testste: event " + i + " name: " + event_list[i]);
			}
			System.out.println("Testste:subscribe to event:" + event_list[0]);
			stream_event_object.subscribe(event_list[0], this);
			
			
			Thread.sleep(60000);
		
			
		}
		catch (Exception e) {
			toDraw = "got an exception:\n" + e.toString() + "\n";
			repaint();
			try { Thread.sleep(60000); } catch (Exception ex) {}
		}
		
		
	
	}

	public void receiveStreamEvent(StreamEvent event) {
		toDraw = "EventId: " + event.getEventId() + "\nEventName: " + event.getEventName() + "\nEventNPT: " + event.getEventNPT() + "\nEventData: "; 
		byte[] data = event.getEventData();
		int i = 0;
		for (i = 0; i < data.length; i++) {
			toDraw = toDraw + data[i];
		}
		repaint();
	}



	public void paint(Graphics g) {
	
		FontMetrics fontMetrics = g.getFontMetrics();
		g.setColor(SystemColor.red);
		StringTokenizer st = new StringTokenizer(toDraw, "\n");
		int j = 1;
		while (st.hasMoreTokens()) {
			g.drawString(st.nextToken(), 100, 100 + j * (fontMetrics.getAscent() + 3));
			j++;
		}
		
	}

	public void pauseXlet() {
	
	}

	public void destroyXlet(boolean flag) throws XletStateChangeException {
	
		rootContainer.setVisible(false);
		rootContainer.removeAll();
		rootContainer = null;
		context.notifyDestroyed();
	}


}

