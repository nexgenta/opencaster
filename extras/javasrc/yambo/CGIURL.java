package tv.cineca.apps.yambo;

import java.io.*;
import java.lang.*;
import java.util.*;

public class CGIURL 
{ 
	public static int exit_protocol = 0;
	public static int file_protocol = 1;
	public static int dvb_protocol = 2;
	public static int http_protocol = 3;
	public static int class_protocol = 4;
	
	public static String exit_str = "exit://";
	
	public int carousel_id = -1;
	public boolean exit = false;
	public String server_name = null;
	public String valued_path = null;
	public String unvalued_path = null; 
	public String StringForm = null; 
	public int protocol = -1;

	private Vector names = null;
	private Vector values = null;
	
	public CGIURL(String URL) {
	
		String parsed;
		String variable_name;
		String variable_value;

		/* Parse URL */
		carousel_id = -1;
		server_name = null;
		exit = false;
		StringForm = URL;
		if (URL.startsWith("dvb://")) {
			parsed = URL.substring(6);
			carousel_id = Integer.parseInt(parsed.substring(0, parsed.indexOf('/')));
			protocol = dvb_protocol;
		} else if (URL.startsWith("http://")) {
			parsed = URL.substring(7);
			server_name = parsed.substring(0, parsed.indexOf('/'));
			protocol = http_protocol;
		} else if (URL.startsWith("file://")) {
			parsed = URL.substring(7);
			protocol = file_protocol;
		} else if (URL.startsWith("class://")) {
			parsed = URL.substring(8);
			server_name = parsed.substring(0, parsed.indexOf('/'));
			protocol = class_protocol;
		} else {
			exit = true;
			valued_path = new String(exit_str);
			unvalued_path =  new String(exit_str);
			StringForm =  new String(exit_str);
			return;
		}

		/* Parse CGI like paramenters if any */
		names = new Vector();
		values = new Vector();
		parsed = parsed.substring(parsed.indexOf('/') + 1);
		valued_path = parsed;
		unvalued_path = parsed;
		if (parsed.indexOf('?') != -1) {
			unvalued_path = unvalued_path.substring(0, unvalued_path.indexOf('?') + 1); 
			parsed = parsed.substring(parsed.indexOf('?') + 1);
			while (parsed.length() > 1)  {
				variable_name = parsed.substring(0, parsed.indexOf('='));
				names.addElement(variable_name);
				unvalued_path = unvalued_path + variable_name + "=";
				parsed = parsed.substring(parsed.indexOf('=') + 1);
				if (parsed.indexOf('&') != -1) {
					variable_value = parsed.substring(0, parsed.indexOf('&'));
					values.addElement(variable_value);
					unvalued_path = unvalued_path + "&";
					parsed = parsed.substring(parsed.indexOf('&') + 1);
				} else {
					variable_value = parsed.substring(0, parsed.length());
					values.addElement(variable_value);
					return;
				}
			}
		}
		
	}

	/* Substitue "variable_name=&" with "variable_name=variable_value&" or "variable_name=" with "variable_name=variable_value" if "=" is the last char */ 
	public String Evaluate(String parameters_to_evaluate)  {
	
		int index;
		String name;
		String before;
		String after;

		for (int i = 0;  i < names.size(); i++) {
			name = (String) names.elementAt(i);
			index = parameters_to_evaluate.indexOf(name + "=");
			if ((index + name.length() + 1 == parameters_to_evaluate.length()) || ((index > -1) && (parameters_to_evaluate.charAt(index + name.length() + 1) == '&'))) {
				before = parameters_to_evaluate.substring(0, index + name.length());
				after = parameters_to_evaluate.substring(index + name.length() + 1, parameters_to_evaluate.length());
				parameters_to_evaluate = before + "=" + values.elementAt(i) + after;
			}
		}

		return parameters_to_evaluate;
	}  
	
	public String getValue(String param) {
	
		for (int i = 0;  i < names.size(); i++) {
			String name = (String) names.elementAt(i);
			if (name.equals(param)) {
				return (String) values.elementAt(i);
			}
		}
		
		return null;
	
	}

}

