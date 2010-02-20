package tv.cineca.parser;
public interface Parser extends tv.cineca.parser.sax.Parser {
  void setDocumentHandler(DocumentHandler handler);
}