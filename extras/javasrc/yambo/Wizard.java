package tv.cineca.apps.yambo;

import javax.tv.graphics.*;
import javax.tv.xlet.*;
import java.io.*;
import java.io.File.*;
import java.lang.*;
import java.lang.System.*;
import javax.tv.service.selection.*;
import java.awt.*;
import java.awt.event.*;
import java.util.Vector;


public class Wizard extends Component implements Xlet, Runnable, KeyListener, org.dvb.dsmcc.AsynchronousLoadingEventListener, org.dvb.net.rc.ConnectionListener, org.dvb.dsmcc.StreamEventListener {

/*******************************************************************************************************************/
	/* globals */
/*******************************************************************************************************************/
	
	public XletContext context = null;
	public Container root_container = null;
	public ServiceContext service_context = null;
	public org.davic.net.dvb.DvbLocator locator = null; 
	public CGIURL StatusURL = null;
	public int StatusPage = -1;
	public String currentPath = null;
	public String localRootDir = null;

	private String waitText = null;
 	private Thread StartThread = null;
	private org.dvb.dsmcc.DSMCCObject xmlDSMCC = null;
	private File xmlHTTP = null;
	private Vector Pages = null;
	private Vector XMLPage = null;
	private Timer timer = null;
	private boolean displayReady = false;	
	private boolean rendered = false;	

/*******************************************************************************************************************/




/*******************************************************************************************************************/
	/* xlet life cycle functions */
/*******************************************************************************************************************/
	public void initXlet(javax.tv.xlet.XletContext _context) throws javax.tv.xlet.XletStateChangeException {

		context = _context;
		
		try {
		
			service_context = ServiceContextFactory.getInstance().getServiceContext(context);
			locator = (org.davic.net.dvb.DvbLocator) service_context.getService().getLocator();
			CarouselManager.configure(locator);
			System.out.println("locator: " + locator.toString());	
			
		}
		catch (Exception e) { 
			System.out.println("service context not available, pc-emultation or something wrong ?");
		}
				
		root_container = TVContainer.getRootContainer(context);
		root_container.setLayout(new BorderLayout(0 ,0));
		root_container.add(this, "Center");
	}


	public void startXlet() throws javax.tv.xlet.XletStateChangeException {
	
		root_container.setVisible(true);
		requestFocus();
		
		StartThread = new Thread(this);
		StartThread.start();
	}


	public void pauseXlet() {
	
	}


	public void destroyXlet(boolean unconditional) throws javax.tv.xlet.XletStateChangeException {
	
		if ( unconditional) {
			try {
				RCManager.Disconnect();
			}	
			catch (Exception e) { 
				e.printStackTrace();
			}
			
			if (StartThread != null) {
			
				StartThread.interrupt();
				StartThread = null;
				
			}
			
			context.notifyDestroyed(); 

		} else {
			throw new XletStateChangeException("No");
		}
	}


	public void run() {
	
		try {
	
			/* Get input parameters */ 
			String[] args = (String[]) context.getXletProperty(XletContext.ARGS);
			for(int i = 0; i < args.length; i++) {
				System.out.println("xlet argument " + Integer.toString(i) + ": " + args[i] );
			}
			
			if (args.length > 1) {
				waitText = args[1];
			} else {
				waitText = "";
			}
			
			if (args.length > 5) { /* connection parameters */
				RCManager.configure(args[2], args[3], args[4]);
			}

			/* Receive user keys from now on */
			addKeyListener(this);
		    
			/* Change page from the input parameter */
			timer = null;
			StatusURL = null;
			if (args.length > 0) {
				ChangePage(args[0], 1);
			} else {
				ChangePage("start.xml", 1);			
			}
			
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
	}
/*******************************************************************************************************************/






/*******************************************************************************************************************/
	/* Change page functions */
/*******************************************************************************************************************/
	/* Change page is the main action that occurs                     */
	/* It changes the current page, it can be called from:		  */
	/*	- the very first run at beginning from input params       */
	/*	- after a colored key with a url associated is pressed	  */
	/*	- after a timeout with a url associated is passed	  */
	/*	- after ok is pressed with a url associated to input item */
	/*	- when a stream event is received with a url associated   */
	/* It can take different workflown but in the end calls display() */
	/* function                                                       */
/*******************************************************************************************************************/	
	public void ChangePage(String _URL, int PageNumber) {
	
		CGIURL newURL = null;
		displayReady = false;

		/* Audio feedback */
		Singer.play();
		
		/*  Stop timer if set */
		if (timer != null) {
			timer.interrupt();
	 	}

		/* Generate next URL to get and new status variables */	
		if (StatusURL == null) {
			System.out.println("Wizard first page " + _URL + ": " + Integer.toString(PageNumber));		
			newURL = new CGIURL(_URL);
		} else {
			System.out.println("Wizard change page to " + _URL + ": " + Integer.toString(PageNumber) + ", StatusURL is " + StatusURL.valued_path);
	 		newURL = new CGIURL(StatusURL.Evaluate(_URL));
		}
		
		/* Check if the url is pointing to the current xml file or it's necessary to get a new xml*/
		if (StatusURL != null && newURL.valued_path.equals(StatusURL.valued_path)) { 
			StatusPage = PageNumber;
			XMLPage = (Vector) Pages.elementAt(PageNumber - 1);
			display();
		} else {
			StatusURL = newURL; 
			StatusPage = PageNumber;
			getXMLfromURL();		
		}
		
		
	}
	
	private void getXMLfromURL() {

		System.out.println("Wizard new valued path is " + StatusURL.valued_path );
		if (StatusURL.protocol == CGIURL.dvb_protocol) { /* xml file comes form a carousel, start loading... */
		
			try {
				xmlDSMCC = new org.dvb.dsmcc.DSMCCObject((CarouselManager.getCarousel(StatusURL.carousel_id)).getMountPoint(), StatusURL.valued_path);
				currentPath = StatusURL.valued_path;
				xmlDSMCC.asynchronousLoad(this); /* wait for loaded event */
			}
			catch (Exception e) {
				e.printStackTrace();
			}
			
		} else if (StatusURL.protocol == CGIURL.http_protocol) { /* xml file is on a http server, it's necessary to wait connection to go up if not always on */

			RCManager.Connect(this); /* wait for connectionChanged */		
			
		} else if (StatusURL.protocol == CGIURL.file_protocol) { /* xml file is on the local file system */
		
			try {
				xmlDSMCC = new org.dvb.dsmcc.DSMCCObject(StatusURL.valued_path);
				currentPath = StatusURL.valued_path;
				findXml();
			}
			catch (Exception e) {
				e.printStackTrace();
			}
			
		} else if  (StatusURL.protocol == CGIURL.class_protocol) { /* pass the URL to a class to load or already loaded */
		
			try {
				System.out.println("class loading:" + StatusURL.server_name);
				Class xletclass = this.getClass();
				ClassLoader classloader = xletclass.getClassLoader();
				Class server = classloader.loadClass(StatusURL.server_name);
				Object instance = server.newInstance();
				if (instance instanceof tv.cineca.apps.yambo.ExternalClass) {
					parseXML(((tv.cineca.apps.yambo.ExternalClass)instance).getXML(StatusURL));
				}
			}
			catch (Exception e) {
				e.printStackTrace();
			}

		
		} else { /* the specified url is the exit:// url or not recognized */
			try {
				destroyXlet(true);
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}	
	}

	/* network connection state changed */
	public void connectionChanged(org.dvb.net.rc.ConnectionRCEvent event) {

		System.out.println("connection changed: " + event.toString());
		
		boolean connection_fail = false;
		
		if (event instanceof org.dvb.net.rc.ConnectionEstablishedEvent) {
			try {
				parseXML(RCManager.ReadHttp("http://" + StatusURL.server_name + "/" + StatusURL.valued_path));
			}
			catch (Exception e) {
				e.printStackTrace();
				connection_fail = true;
			}	

		} else {
			connection_fail = true;	
		}
		
		 /* manage connection failure */
		if (connection_fail) {
                	for(int i = 0; i < XMLPage.size(); i++) {
                       		NamedItem item = (NamedItem) XMLPage.elementAt(i);
                       		if (item.Name.equals("FailConnectItem")) {
					System.out.println("Wizard managed failed connection");
					ChangePage(item.URL, Integer.parseInt(item.URLPage));
				}
			}
		}
	}

	/* carousel file is loaded */
	public void receiveEvent(org.dvb.dsmcc.AsynchronousLoadingEvent event) {
	
		findXml();
		
	}

	/* stream event received */
	public void receiveStreamEvent(org.dvb.dsmcc.StreamEvent event) {
	
		EventManager.receiveEvent(event);

	}
	
	/* if the file is not found in the local file system, there is a fallback policy */
	public void findXml() {

		try {
			parseXML(new FileInputStream(xmlDSMCC));
		}
		catch (FileNotFoundException f) {
			try {
				/* Second chance without values */
				if (currentPath != StatusURL.unvalued_path) {
					System.out.println("Wizard failed to found " + StatusURL.valued_path);
					System.out.println("Wizard search for " + StatusURL.unvalued_path);
					currentPath = StatusURL.unvalued_path;		
					if (StatusURL.protocol == CGIURL.dvb_protocol) {
						xmlDSMCC = new org.dvb.dsmcc.DSMCCObject(CarouselManager.getCarousel(StatusURL.carousel_id).getMountPoint(), StatusURL.unvalued_path);
						xmlDSMCC.asynchronousLoad(this);
					} else {
						xmlDSMCC = new org.dvb.dsmcc.DSMCCObject(StatusURL.unvalued_path);
						findXml();
					}
				} 
			} 
			catch (Exception e) {
				e.printStackTrace();
			}	
		}
		catch (Exception e) {
			e.printStackTrace();
		}	
		
	}	

	/* parse the XML file */
	private void parseXML(String _input) {

		try {
			MinMLWizard XMLParser = new MinMLWizard(_input);
			parseXML(XMLParser);
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}

	private void parseXML(FileInputStream _inputStream) {

		try {
			MinMLWizard XMLParser = new MinMLWizard(_inputStream);
			parseXML(XMLParser);
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}	
	
	private void parseXML(MinMLWizard XMLParser) {
	
		try {
			Pages = XMLParser.Pages; 
			XMLPage = (Vector) Pages.elementAt(StatusPage - 1);
			display();	
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
/*******************************************************************************************************************/







/*******************************************************************************************************************/
	/* Display functions */
/*******************************************************************************************************************/
	private void display() {

		for(int i = 0; i < XMLPage.size(); i++) {
			NamedItem item = (NamedItem) XMLPage.elementAt(i);
		} 
		ColorKeyManager.configure(XMLPage);
		Singer.configure(XMLPage);
		IFrameManager.configure(XMLPage);
		VideoManager.configure(XMLPage);
		EventManager.configure(XMLPage, this);
		Painter.configure(XMLPage, this);
		Printer.configure(XMLPage, this);
		InputManager.configure(XMLPage, this);
		for(int i = 0; i < XMLPage.size(); i++) {
			NamedItem item = (NamedItem) XMLPage.elementAt(i);
			if (item.Name.equals("TimeItem")) {
				timer = new Timer(XMLPage, this);
				if (timer.time >= 0) {
					timer.start();
				}
			}
		}
		if (Painter.ready() && Printer.ready()) {
			displayReady = true;
			System.out.println("Wizard display call repaint");
			repaint(); 
		}
		
	}	
	
	public void repaint() {

		System.out.println("Wizard.repaint");
		try {
			synchronized(this) {
				rendered = false;
				super.repaint();
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
	}
	
	protected void waitDraws() {

		synchronized(this) {	
			Toolkit.getDefaultToolkit().sync();
			rendered = true;
		}
	
	}
	
	
	public void paint(Graphics graphics) {

		System.out.println("Wizard.paint:");
		if (displayReady) {
		
			Painter.paint(graphics);
			Printer.paint(graphics);
			InputManager.paint(graphics);
			VideoManager.resize(service_context);
			VideoManager.zap(service_context);
                	for(int i = 0; i < XMLPage.size(); i++) {
                       		NamedItem item = (NamedItem) XMLPage.elementAt(i);
                       		if (item.Name.equals("DisconnectItem")) {
					try {
						System.out.println("Wizard parsed disconnect");
						RCManager.Disconnect();
					}
					catch (Exception e) {
						e.printStackTrace();
					}
				}
			}	
			
		} else { 

			/* paint comes before page is loaded */
			graphics.setFont(new Font("Tiresias", Font.PLAIN, 31));
			graphics.setColor(Color.white);
			System.out.println("Wizard Paint : " + waitText); 
			graphics.drawString(waitText, 60, 60);
		
		}
		
		waitDraws();

		
	}
/*******************************************************************************************************************/






/*******************************************************************************************************************/
	/* Manage user events */ 
/*******************************************************************************************************************/
	public void keyPressed(KeyEvent key) {

		System.out.println("Wizard key pressed: " + key.toString());
		switch(key.getKeyCode()) {

			case KeyEvent.VK_ESCAPE:	// exit, 27
				try {
					destroyXlet(true);
				}
				catch (Exception e) {
					e.printStackTrace();
				}
				break;
			case KeyEvent.VK_UP: 		// up, 38
			case KeyEvent.VK_DOWN: 		// down, 40
				break;
			case KeyEvent.VK_ENTER:		// ok, 10
				ChangePage(InputManager.getURL(), InputManager.Page);
				break;
			case KeyEvent.VK_LEFT: 		// left, 37
			case KeyEvent.VK_RIGHT: 	// right, 39
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
			 	InputManager.keyPressed(key);
				break;
			case 403: 			// Red, VK_COLORED_KEY_0, F1
			case KeyEvent.VK_F1:
				if (ColorKeyManager.RedURL != null) {
					ChangePage(ColorKeyManager.RedURL, ColorKeyManager.RedPage);
				}
				break;
			case 404: 			// Green, VK_COLORED_KEY_1, F2
			case KeyEvent.VK_F2:
				if (ColorKeyManager.GreenURL != null) {
					ChangePage(ColorKeyManager.GreenURL, ColorKeyManager.GreenPage);
				}
				break;
			case 405: 			// Yellow, VK_COLORED_KEY_2
			case KeyEvent.VK_F3:
				if (ColorKeyManager.YellowURL != null) {
					ChangePage(ColorKeyManager.YellowURL, ColorKeyManager.YellowPage);
				}
				break;
			case 406: 			// Blue, VK_COLORED_KEY_3
			case KeyEvent.VK_F4:			
				if (ColorKeyManager.BlueURL != null) {
					ChangePage(ColorKeyManager.BlueURL, ColorKeyManager.BluePage);
				}
				break;
			default:
				break;
		}
	}

	public void keyTyped(KeyEvent keyevent)  {
	}

	public void keyReleased(KeyEvent keyevent) {	
	}



	
}
/*******************************************************************************************************************/
