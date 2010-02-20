package tv.cineca.apps.yambo;

import javax.tv.service.selection.*;
import javax.media.Player;
import javax.tv.media.AWTVideoSizeControl;
import javax.tv.media.AWTVideoSize;
import java.awt.Rectangle;
import java.awt.Dimension;
import javax.tv.locator.*;
import javax.tv.xlet.XletContext;
import java.util.*;
import org.havi.ui.*;

/* Change the background */
class IFrameManager {

	private static RCResourceClient resourceClient = new RCResourceClient();

	public static void configure(Vector Items) {

		HStillImageBackgroundConfiguration backImageConfiguration = null;

		for(int i = 0; i < Items.size(); i++) {
			NamedItem item = (NamedItem) Items.elementAt(i);
			if (item.Name.equals("BackgroundItem")) {
				System.out.println("IFrameManager: got item " + item.URL);
				HScreen screen = HScreen.getDefaultHScreen();
				HBackgroundDevice backDevice = screen.getDefaultHBackgroundDevice();
				HBackgroundConfigTemplate backConfigurationTemplate = new HBackgroundConfigTemplate();
				backConfigurationTemplate.setPreference(HBackgroundConfigTemplate.FLICKER_FILTERING, HBackgroundConfigTemplate.PREFERRED);
				HBackgroundConfiguration backConfiguration = backDevice.getBestConfiguration(backConfigurationTemplate);
				if (backDevice.reserveDevice(resourceClient)) {
					try {
						backDevice.setBackgroundConfiguration(backConfiguration);
						if(backConfiguration instanceof HStillImageBackgroundConfiguration) {
							backImageConfiguration = (HStillImageBackgroundConfiguration) backConfiguration;
						} else {
							System.out.println("IFrameManager: no background device available");
							backDevice.releaseDevice();
						}		
					} 
					catch (Exception e) {
						System.out.println("IFrameManager: no background device available");
						backDevice.releaseDevice();
					}
				}

				try {
					CGIURL CGI_URL = new CGIURL(item.URL);
					if (CGI_URL.carousel_id > -1) {
						HBackgroundImage backImage = new HBackgroundImage((CarouselManager.getCarousel(CGI_URL.carousel_id)).getMountPoint() + "/" + CGI_URL.valued_path);
						backImageConfiguration.displayImage(backImage);
					}
				}
				catch (Exception e) {
					System.out.println("IFrameManager: background: " + e);
				}
				
			}
		}

	}

}
