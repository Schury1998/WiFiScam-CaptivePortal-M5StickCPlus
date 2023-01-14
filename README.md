<img src="D:\Desktop\Projekte\M5SitckCPlus\WiFiScam\M5.png" alt="M5StickC" width="350">

## Description

**Open a fake Free WiFi with the M5StickC PLUS and phish emails and possible passwords:** 
You can change the SSID over the Server. 
The email addresses and passwords are stored in the EEPROM.
With the Button A you refresh the local available WiFi's and shut down the AP.
With the Button B you activate the AP. A second click on the Button B turns off the Display to save energy. You can turn on the display with clicking the Button B again or press Button A (If you use the A Button, the available networkscan is trigger again)
With the other sidebutton from the M5StickCPlus you can turn off and turn on the M5StickCPlus.

**To implement the M5CStickCPlus to the Arduino IDE:**
1. Open the IDE
2. File - Preferences - Additional boards manager URLs: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
3. Tools - Board - Boards Manager... and type in M5Stack - install the Boards
4. Tools - Manage Libarys... - Type in M5StickCPlus and insatll the M5StickCPlus by M5Stack
5. Tools - Board and select the M5StickCPlus
6. Plug in the M5StickCPlus - Tools - Port and selct the Port
7. Complie and Upload

For testing and home use only. I assume no liability for illegal use.