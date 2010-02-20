import tv.cineca.apps.yambo.ExternalClass;
import tv.cineca.apps.yambo.CGIURL;

public class ExternalExample implements tv.cineca.apps.yambo.ExternalClass {

	/* This class has to stay at the carousel root, some mhp stacks have problems to load if from others points */

	public String getXML(CGIURL url) {
	
		int i = 0;
		int j = 0;
		
		String value = url.getValue("number1");
		if (value != null) {
			i = Integer.parseInt(value);
		}
		value = url.getValue("number2");
		if (value != null) {
			j = Integer.parseInt(value);
		}

		int k = i + j;
	
		return "<Pages>\n<Page>\n<StaticTextItem>\n<LocationX>100</LocationX>\n<LocationY>100</LocationY>\n<Size>25</Size>\n<Color>White</Color>\n<Text>This is a .class output example, result number is: " + k +  "</Text>\n</StaticTextItem>\n</Page>\n</Pages>\n";
	
	}

}
