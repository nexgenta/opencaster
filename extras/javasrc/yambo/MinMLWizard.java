package tv.cineca.apps.yambo;

import java.io.*;
import java.lang.*;
import tv.cineca.parser.*;
import tv.cineca.parser.sax.*;
import java.util.Vector;
import java.util.Date;
import java.awt.event.*;
import java.io.*;

public class MinMLWizard extends MinML {

	public Vector Pages = null;

	private Vector Items = null;
	private Vector Stack = null;
	private String characters = null;

	/* Parse from String */
	public MinMLWizard (String input) { 

		parse_input(new StringReader(input));

	}

	/* Parse from file */
	public MinMLWizard (FileInputStream input) { 
	
		parse_input(new InputStreamReader(new BufferedInputStream(input, 1024)));

	}

	/* Parse core */
	private void parse_input (Reader input) { 
		Pages = new Vector();
		Items = null;
		Stack = null;
		try {
			parse(input);
		}
		catch (final IOException e) {
			System.out.println("IOException: " + e);
			e.printStackTrace();
		}
		catch (final SAXException e) {
			System.out.println("SAXException: " + e);
			e.printStackTrace();
		} 
		catch (final Throwable e) {
			System.out.println("Other Exception: " + e);
			e.printStackTrace();
		}
	}
	
	public void startDocument() {
	}

	public void endDocument() {
	}

	public void startElement (String name, AttributeList attributes) {

		characters = null;

		if(name.equals("Page")) { /* Use new Items, get new stack */
			Items = new Vector();
			Stack = new Vector();
		}  else if(name.equals("URL")) { /* Add to stack URL chaining baseurl and parameters */	
			String string = attributes.getValue(0);
			if (attributes.getLength() > 1) {
				string = string + "?";
				for (int i = 1; i < attributes.getLength(); i++) {
					string = string + attributes.getName(i) + "=" + attributes.getValue(i);
					if (i + 1 < attributes.getLength()) {
						string = string + "&";
					}
				}
			}    
			Stack.addElement(string);
		}

	}

	public void endElement (String name) {


		if (characters != null) {
			Stack.addElement(characters);
			characters = null;
		}

		/* Close the page or add to stack the items */
		NamedItem newItem = new NamedItem(name);	
		if (name.equals("Page")) {
			Pages.addElement(Items);
			return;
		} else if (name.equals("GraphicItem")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Visible = "Yes";
			newItem.LocationY = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationX = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("BackgroundItem")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("ServiceItem")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);
		} else if (name.equals("AudioItemPreFetch")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Visible = "No";			
		} else if (name.equals("AudioItem")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Visible = "Yes";
		} else if (name.equals("GraphicItemPreFetch")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Visible = "No";
			newItem.LocationY = "0";
			newItem.LocationX = "0";
			newItem.Name = "GraphicItem";
		} else if (name.equals("StaticTextItem")) {
			newItem.Text = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Color = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Size = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Visible = "Yes";
			newItem.LocationY = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationX = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);			
		} else if (name.equals("TextItem")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Color = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Size = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Visible = "Yes";
			newItem.LocationY = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationX = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("TextItemPreFetch")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Color = "White";
			newItem.Size = "25";
			newItem.Visible = "No";
			newItem.LocationY = "0";
			newItem.LocationX = "0";
			newItem.Name = "TextItem";
		} else if (name.equals("VideoItem")) {
			newItem.Height = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Width = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationY = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationX = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("TimeItem")) {
			newItem.URLPage = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);
			newItem.Time = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("EventItem")) {
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);
		} else if (name.equals("FailConnectItem")) {
			newItem.URLPage = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("RedKeyItem")) {
			newItem.URLPage = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("YellowKeyItem")) {
			newItem.URLPage = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.URL = (String) Stack.lastElement();
 			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("BlueKeyItem")) {
			newItem.URLPage = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("GreenKeyItem")) {
			newItem.URLPage = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("InputItem")) {
			newItem.InputTag = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.InputFlags = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.URLPage = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.URL = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.Width = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationY = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationX = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} else if (name.equals("PadHelpItem")) {
			newItem.LocationY = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
			newItem.LocationX = (String) Stack.lastElement();
			Stack.removeElementAt(Stack.size() - 1);	
		} 
		Items.addElement(newItem);
	}

	public void characters (char ch[], int start, int length) {

		String string = new String(ch, start, length); 
		if (characters != null) {
			characters = characters + string;
		} else {
			characters = string;
		}
	}

	public void fatalError (SAXParseException e) throws SAXException {
		System.out.println("Error: " + e);
		throw e;
	}
}

