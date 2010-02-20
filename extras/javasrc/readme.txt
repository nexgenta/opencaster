Yambo 1.1 1/12/2005

Introduction:
Yet Another Micro Brower Open is an xlet that parses xml pages as specified below. It's a very simple MHP application implemeting teletext like functionalities.
It's probabily the fastest way to see an image of yours on an java interactive box

To set-up Yambo into a carousel:
It is necessary the specifiy the following parameters from the AIT
1. Yambo application name
2. Phone number to call for dial-up connection
3. Starting URL of the first page
4. Waiting message to display when Yambo engine loads itself

To set-up Yambo into a justdvb-it/creatv carousel check this example:
dvb_j_application_descriptor(paramenters = ["yambo_example", "0516116371","dvb:/1/sample","Wait please..."],),
dvb_j_application_location_descriptor(
    base_directory = "/",
    class_path_extension = "",
    initial_class = "tv.cineca.apps.yambo.Wizard",
),

To build Yambo classes from java:
It's necessary to put into the "source" directory:
    * classes.zip from JDK 1.1.8 (http://java.sun.com/products/archive/index.html)
    * javatv.jar
    * mhp stubs for Mhp (http://www.interactivetvweb.org/resources/code/mhpstubs.jar , http://www.interactivetvweb.org/resources/code/mhpstubs-src.zip)
    * javac from jdk 1.5.0_03 (http://java.sun.com/j2se/1.5.0/download.jsp)
Command line:
$HOME/jdk1.5.0_03/bin/javac -target 1.1 -g:none -source 1.2 -classpath ./:javatv.jar -bootclasspath classes.zip -extdirs "" tv/cineca/apps/yambo/*.java

Yambo 1.1 known bugs:

Yambo 1.1 Change log:
Added new persistent storage tag
Fixed can't change video position twice in a row
All the color keys items don't need to be defined
Fixed input misses to display the first character
Return channel timeout balanced better

Yambo 1.1 specification:

Yambo is a browser of pages made of text, images and audio as defined from the xml specified below.

    * Every XML has at least 1 page
    * Every XML has a tag <Pages> at the begin and at the end
    * The first page is number 1
    * Every page has a tag <Page> at the begin and at the end
    * Yambo moves from a page to another using URL and page number
    * Yambo always exits on 'exit' key press.
    * Text, audio and images are loaded before showing the page, while moving from a page to another if the same images, audio or texts are presents in both of them reload is avoided.
    * dvb:\\ is the default protocol URL and is followed by the carousel's id number, eg: "dvb:\\1\dir\file" will refer to the file into the directory 'dir' in the carousel with carousel's id '1'
    * <URL> tag is defined as: <URL basename=protocol:\\string variable_1=value_variable_1 ... variable_n=value_variable_n'/>', eg: "dvb:\\1\test\sample?name=giovanni&surname=bianchi" becomes <URL basename=dvb:\\1\test\sample? name=giovanni surname=bianchi />. Note that "sample?name=giovanni&surname=bianchi" is a valid file's name.
    * URL with parameters: If an URL with at least a parameter fails to be found YAMBO will search the URL without the paramenters' value, eg: if "dvb:\\1\query?name=giovanni&surname=bianchi" cannot be found, YAMBO will search "dvb:\\1\query?name=&surname=". The next queries, however, will still try to search first the URL with valued parameters, so "dvb:\\1\query?name=giovanni&surname=bianchi&age=27" will be search before "dvb:\\1\query?name=&surname=&age=" if "name" and "surname" were valued at least once.
    * If the URL has a "http" protocol instead of "dvb", a dial-up connection is established and the web server is queried for Yambo XML.
    * The special URL "exit:\\" exits YAMBO.
    * Tags allowed for a page are:


<GraphicItem> (Load and display jpeg, render follows the declartion order)
<LocationX>Pixel</LocationX>
<LocationY>Pixel</LocationY>
<URL> (Graphic resource)
</GraphicItem>

<AudioItem> (Mpeg2 audio is loaded and played)
<URL> (Audio resource)
</AudioItem>

<StaticTextItem> (Texts are loaded and rendered with Teresia following the declaration order)
<LocationX>Pixel</LocationX>
<LocationY>Pixel</LocationY>
<Size>(36|31|26|24)</Size>
<Color>(Red|Blue|Green|Yellow|Black|White)</Color>
<Text>Text</Text>
</StaticTextItem>

<TextItem> (Texts are rendered with Teresia following the declaration order and updates of the text at the URL are observed)
<LocationX>Pixel</LocationX>
<LocationY>Pixel</LocationY>
<Size>(36|31|26|24)</Size>
<Color>(Red|Blue|Green|Yellow|Black|White)</Color>
<URL> (Text is unicode and render is updated if changes)
</TextItem>

<ServiceItem> (Changes service at the specified one)
<URL> (Service, program, channel)
</ServiceItem>

<VideoItem> (Resize and move the video as specified)
<LocationX>Pixel</LocationX>
<LocationY>Pixel</LocationY>
<Width>Pixel</Width>
<Length>Pixel</Length>
</VideoItem>

<TimeItem>(Set a timer to move at the specified page)
<Time>Seconds</Time>
<URL>(URL of the next page)
<URLPage>Page number</URLPage>
</TimeItem>

<RedKeyItem> (Moves at the specified url at the press of the red key)
<URL>(URL of the next page)
<URLPage>Page number</URLPage>
</RedKeyItem>

<YellowKeyItem> (Moves at the specified page at the press of the yellow key)
<URL>(URL of the next page)
<URLPage>Page number</URLPage>
</YellowKeyItem>

<BlueKeyItem> (Moves at the specified page at the press of the blue key)
<URL>(URL of the next page)
<URLPage>Page number</URLPage>
</BlueKeyItem>

<GreenKeyItem> (Moves at the specified page at the press of the green key)
<URL>(URL of the next page)
<URLPage>Page number</URLPage>
</GreenKeyItem>

<InputItem> (If 'ok' key is pressed moves to the page at the address of "URL""VariableName""="TypedText". It's important that "URL" ends with '?' if there are no previous parameters present in the "URL")
<LocationX>Pixel</LocationX>
<LocationY>Pixel</LocationY>
<Width>Number of characters</Width>
<URL>(URL of the next page)
<URLPage>Page number</URLPage>
<InputFlags>(Number|Alphabet)</InputFlags>
<InputTag>VariableName</InputTag>
</InputItem>

<PadHelpItem> (Shows the help-pad for text input at the specified location)
<LocationX>Pixel</LocationX>
<LocationY>Pixel</LocationY>
</PadHelpItem>

<StoreItem> (Save the status before changing URL at the specified filename)
<Text>Filename</Text>
</StoreItem>

<RestoreItem> (If the filename exists then changing URL to the stored URL, page 1)
<Text>Filename</Text>
</RestoreItem>

<FailConnectItem> (Moves at the specified page if the connection fails)
<URL>(URL of the next page)
<URLPage>Page number</URLPage>
<FailConnectItem>

<DisconnectItem> (Close the connection if it is open)
</DisconnectItem>

<GraphicItemPreFetch> (Loads an image, if a GraphicItem in the next page has the same address it won't reload it)
<URL>(Graphic resource)
</GraphicItemPreFetch>

<TextItemPreFetch> (Loads a text, if a TextItem in the next page has the same address it won't reload it)
<URL>(Text resource)
</TextItemPreFetch>

<AudioItemPreFetch> (Loads an audio, if an AudioItem in the next page has the same address it won't reload it)
<URL>(Audio resource)
</AudioItemPreFetch>

