# qButton
qButton is really three related projects for smart home control in one, all running on ESP8266 boards.

This is not an officially supported Google product.

## Standalone smart button
This is a simple standalone WiFi-connected button. It sends a pre-programmed command through the Google Assistant API, which may be used for smart home control, such as to turn a set of lights on or off, start some music, or anything else that you can do with a Google Assistant command.

It is designed to be low-power so that it can run for a while from a small battery. It remains off or in low-power mode until the button is pressed, then quickly starts up, connects to WiFi, sends the command, and turns off again.

### Setup
1. Set your client ID, client secret and device model ID for the Google Assistant API in `assistant.cpp`.
1. Build and install the firmware from the 'button' env using PlatformIO.
1. Connect the device to power. After a few seconds it will show up as an access point named qbutton.
1. Connect to the access point from your phone or laptop.
1. Open http://192.168.0.1/ or http://qbutton.local/ in your browser.
1. Enter your WiFi network details and click 'Update'.
1. Reset the device. It should connect to your WiFi network.
1. Connect to the same WiFi network and open http://qbutton.local/ in a browser, or find the device's IP address (from the serial console or your DHCP server) and use that.
1. Click the 'set account' link and authorize it to access your Google account.
1. Set the command you want it to run.
1. Reset once more. It should now connect and run your command on each reset.

## RF button bridge
This is replacement firmware for the [Sonoff RF Bridge 433](https://www.itead.cc/wiki/Sonoff_RF_Bridge_433), which lets you pair a large number of 433 MHz RF buttons (of which there are [many available](https://www.aliexpress.com/wholesale?SearchText=433mhz+rf+button)) and have each send a certain Google Assistant command.

### Setup
1. Set your client ID, client secret and device model ID for the Google Assistant API in `assistant.cpp`.
1. Open the Sonoff RF Bridge (screws are under stickers on the base), solder a 4 pin header for the serial port. The [Tasmota wiki](https://github.com/arendst/Sonoff-Tasmota/wiki/Hardware-Preparation) has some more details.
1. Connect a 3.3V USB serial adapter between the serial pins and your computer, remembering to swap to RX and TX lines.
1. Switch the switch on the board towards the side that the serial pins are on.
1. Connect the RF Bridge to power (either via the USB port or from the serial header, not both at the same time) while holding down the button on the side. You can release the button a few seconds after connecting pwoer. This puts it into reflashing mode.
1. Build and install the firmware from the 'rfbridge' env using PlatformIO.
1. Disconnect power and the serial cable. Switch the switch back to the other side.
1. Connect the bridge device to power. After a few seconds it will show up as an access point named qbutton.
1. Connect to the access point from your phone or laptop.
1. Open http://192.168.0.1/ or http://qbutton.local/ in your browser.
1. Enter your WiFi network details and click 'Update'.
1. Reset the device. It should connect to your WiFi network.
1. Connect to the same WiFi network and open http://qbutton.local/ in a browser, or find the device's IP address (from the serial console or your DHCP server) and use that.
1. Set an admin password for the web interface.
1. Click the 'set account' link and authorize it to access your Google account.
1. Enter a command for the first button you want to pair, and click 'Add command'. The bridge should beep.
1. Hold the RF button you are pairing down until the bridge beeps again.
1. Repeat for the rest of the buttons you want to pair.

## Smart switch
This is a simple smart switch for switching on and off whatever devices you can attach to the pins of your ESP8266 board from Google Home, via the [Sinric](https://sinric.com/) service. It's well suited for low voltage devices, such as fairy lights that can run off 5V from a USB power supply.

### Setup
1. Create an account on [Sinric](https://sinric.com/) and follow the instructions there to connect it to Google Home.
1. Add devices there for each device you want to control, with device type 'Switch'.
1. Copy your Sinric API key to `sinric.cpp`.
1. Update `config.h` so that:
    1. `SWITCH_PINS` has all the pins you want to be able to control.
    1. `SWITCH_NAMES` has the names you want to display for them in the web interface. These names don't have to match the names you use on Sinric; the names from Sinric are what Google Home will see.
    1. `SWITCH_INVERTED` should be `true` if the output is active-low, or `false` if it's active-high.
    1. `SWITCH_INITIAL_STATE` is the state you want the pin to have when the device is powered on.
1. Build and install the firmware from the 'switch' env using PlatformIO.
1. Connect the device to power. After a few seconds it will show up as an access point named qswitch.
1. Connect to the access point from your phone or laptop.
1. Open http://192.168.0.1/ or http://qswitch.local/ in your browser.
1. Enter your WiFi network details and click 'Update'.
1. Reset the device. It should connect to your WiFi network.
1. Connect to the same WiFi network and open http://qswitch.local/ in a browser, or find the device's IP address (from the serial console or your DHCP server) and use that.
1. Set an admin password.
1. Enter the switch IDs from your Sinric account for each pin that you want to control.
1. Connect whatever you want to control to the pins, and try controlling it from the Google Home app or Google Assistant.
