# qButton
qButton is a simple ESP8266-based WiFi-connected button. Each button sends a pre-programmed command through the Google Assistant API, which may be used for smart home control.

This is not an officially supported Google product.

## Setup
1. Build and install the firmware using PlatformIO.
1. Connect the device to power. After a few seconds it will show up as an access point named qButton.
1. Connect to the access point from your phone or laptop.
1. Open http://192.168.0.1/ or http://qbutton.local/ in your browser.
1. Enter your WiFi network details and click 'Update'.
1. Reset the device. It should connect to your WiFi network.
1. Connect to the same WiFi network and open http://qbutton.local/ in a browser, or find the device's IP address (from the serial console or your DHCP server) and use that.
1. Click the 'set account' link and authorize it to access you account.
1. Set the command you want it to run.
1. Reset once more. It should now connect and run your command on each reset.
