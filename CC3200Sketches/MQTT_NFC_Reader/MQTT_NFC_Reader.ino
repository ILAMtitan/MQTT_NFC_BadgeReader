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

WiFiClient wclient;

uint8_t Tag_Count;
char tag[14];

byte server[] = { 192, 168, 0, 104 }; //  Public MQTT Brokers: http://mqtt.org/wiki/doku.php/public_brokers

#define       WIFI_SSID         "TI_NoINET"

PubSubClient client(server, 1883, callback, wclient);

void callback(char* inTopic, byte* payload, unsigned int length)
{
// Handle callback here
}

void setup()
{
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    Serial.println("===========================");
    Serial.println("Energia NFC Reader enabled.");
    Serial.println("===========================");
    Serial.println();
    pinMode(RED_LED, OUTPUT);
    stand_alone_flag = 0;   // set 1 to enable LEDs on BB (not for CC3200)
    Trf7970.begin();
    Serial.println("Start WiFi");
    WiFi.begin(WIFI_SSID);
    while(WiFi.localIP() == INADDR_NONE)
        {
            Serial.print(".");
            delay(300);
        }
    Serial.println("");
    printWifiStatus();
}

void loop()
{
    Tag_Count = 0;
    digitalWrite(RED_LED, !digitalRead(RED_LED));
    Trf7970.Trf7970Enable();
    Tag_Count = Iso14443a.Iso14443aFindTag();   // Scan for 14443A tags
    if(Tag_Count > 0)
        {
            // publish data to MQTT broker
            if (client.connect("LaunchPadClient"))
                {
                    String str = String(tag_complete_uid[0], HEX);
                    for(int i = 1; i<14; i++)
                      str += String(tag_complete_uid[i], HEX);
                    
                    int str_len = str.length() + 1;  // Length (with one extra character for the null terminator)
                    char char_array[str_len];  // Prepare the character array (the buffer) 
                    str.toCharArray(char_array, str_len);  // Copy it over
                  
                    client.publish("outTopic", char_array);
                    //client.subscribe("inTopic");
                    Serial.println("Publishing successful!");
                    client.disconnect();
                }
        }
    Trf7970.Trf7970Disable();
    
    delay(1000);
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
