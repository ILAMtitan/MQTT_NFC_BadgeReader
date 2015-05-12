/*
Continuously publish analog sensor readings from pin24.
This example reads analog pin 24, converts the int into a char array and publishes it over MQTT.
  - read analog input on pin 24
  - typecase sensor reading to a character array
  - connects to an MQTT broker
  - publishes sensor reading to the topic "outTopic"
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h> //only required if using an MCU LaunchPad + CC3100 BoosterPack. Not needed for CC3200 LaunchPad
#include <OneMsTaskTimer.h>
#include <trf7970BoosterPackEnergia.h>

//Wifi Network Client
WiFiClient wclient;

//Dynamic tag information
uint8_t Tag_Count;

//Variable and defs for in/out mode
int mode = 0;
#define MODE_IN 0
#define MODE_OUT 1

//LED and Button defines
#define MODE_LED    RED_LED
#define MODE_BUTTON     PUSH2

//Server address
byte server[] = { 192, 168, 1, 135 };

//WiFi Network Information
#define       WIFI_SSID         "SiliconVariant"
#define       WIFI_PASS         "DigitalDistraction"
#define       CLIENT_ID         "1234"

//Setup MQTT Client Class
PubSubClient client(server, 1883, callback, wclient);

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

//Callback when a subscribed topic is published to (WIP)
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Received message for topic ");
  Serial.print(topic);
  Serial.print("with length ");
  Serial.println(length);
  Serial.println("Message:");
  Serial.write(payload, length);
  Serial.println();
}


void setup()
{
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    
    //Setup Button(s)
    pinMode(MODE_BUTTON, INPUT_PULLUP);    
    
    //Setup indicator IOs
    pinMode(MODE_LED, OUTPUT);
    digitalWrite(MODE_LED, LOW);  
    
    //Init NFC Reader
    Trf7970.begin();
    Serial.println("Energia NFC Reader enabled.");
    Serial.println(); 
    
    //Setup WiFi
    stand_alone_flag = 0;   // set 1 to enable LEDs on BB (not for CC3200)
    
    Serial.println("Starting WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while(WiFi.localIP() == INADDR_NONE)
        {
            Serial.print(".");
            delay(300);
        }
    Serial.println("");
    
    printWifiStatus();
    
    Serial.println("Setup Complete");
    Serial.println("Ready");

}

void loop()
{
    //Reset tag count
    Tag_Count = 0;
    
    //Read pushbutton state and set mode
    if(digitalRead(MODE_BUTTON)){
      if(mode == MODE_IN){
        mode = MODE_OUT;
        Serial.println("Mode Set to OUT");
        digitalWrite(MODE_LED, HIGH);
      }
      else if(mode == MODE_OUT){
        mode = MODE_IN;
        Serial.println("Mode Set to IN");
        digitalWrite(MODE_LED, LOW);
      }
      
      //A little debouce for user experience
      delay (300);
    }
    
    //Turn on NFCReader
    Trf7970.Trf7970Enable();
    
    //Read current number of present 14443a tags
    Tag_Count = Iso14443a.Iso14443aFindTag();   // Scan for 14443A tags
    
    //If a tag is present...
    if(Tag_Count > 0)
        {
            // publish data to MQTT broker
            if (client.connect("LaunchPadClient"))
                {
                    //Read tag UID and store to string
                    String str = String(tag_complete_uid[0], HEX);
                    for(int i = 1; i<14; i++)
                      str += String(tag_complete_uid[i], HEX);
                    
                    //Set string length, plus a nul terminator
                    int str_len = str.length() + 1;
                    
                    // Prepare the character array (the buffer) 
                    char char_array[str_len];
                    
                    // Copy it over
                    str.toCharArray(char_array, str_len);
                    
                    //Publish UID to server depending on selected mode
                    if(mode == MODE_IN)
                      client.publish("1234/BADGE/IN", char_array);
                    if(mode == MODE_OUT)
                      client.publish("1234/BADGE/OUT", char_array);
                    
                    //User feedback for successfull publish
                    Serial.println("Publishing successful!");
                    
                    //Disconnect from server
                    client.disconnect();
                }
                
                //Turn off NFC reader
                Trf7970.Trf7970Disable();
                
                //Wait 1sec for to prevent multiple reads
                delay(1000);
        }
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}
