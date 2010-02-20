package tv.cineca.parser;

import java.io.Writer;

import tv.cineca.parser.sax.AttributeList;
import tv.cineca.parser.sax.SAXException;


public interface DocumentHandler extends tv.cineca.parser.sax.DocumentHandler {
  Writer startDocument(final Writer writer) throws SAXException;
  Writer startElement(final String name, final AttributeList attributes, final Writer writer)
        throws SAXException;
}
