#include <ArduinoJson.h>

/*
 Chat Server

 A simple server that distributes any incoming messages to all
 connected clients.  To use, telnet to your device's IP address and type.
 You can see the client's input in the serial monitor as well.
 Using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip      (192, 168,   1, 177);
IPAddress myDns   (192, 168,   1,   1);
IPAddress gateway (192, 168,   1,   1);
IPAddress subnet  (255, 255,   0,   0);

// telnet defaults to port 23
EthernetServer server(23);
boolean alreadyConnected = false; // whether or not the client was connected previously

void setup()
{
    // initialize digital pin 13 as an output.
    pinMode(13, OUTPUT);

    // initialize the ethernet device
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
    // start listening for clients
    server.begin();
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    Serial.print("Chat server address:");
    Serial.println(Ethernet.localIP());
}

char json[500] = "";
StaticJsonBuffer<200> jsonBuffer;
String received_command = "";

void loop()
{
    // wait for a new client:
    EthernetClient client = server.available();

    // when the client sends the first byte, say hello:
    if (client)
    {
        if (!alreadyConnected)
        {
            // clear out the input buffer:
            client.flush();
            Serial.println("We have a new client");
            client.println("Hello, client!");
            alreadyConnected = true;
        }

        if (client.available() > 0)
        {
            // read the bytes incoming from the client:
            char thisChar = client.read();

            strncat(json, &thisChar, (sizeof(json) - strlen(json)) );
            received_command += thisChar;

            // echo the bytes back to the client:
            server.write(thisChar);

            // echo the bytes to the server as well:
            Serial.write(thisChar);

// ------------------ JSON EXAMPLE --------------------
// {
//     "sensor":"gps",
//     "time":1351824120,
//     "data":[48.756080,2.302038]
// }
//
//    const char* sensor = root["sensor"];
//    long time          = root["time"];
//    double latitude    = root["data"][0];
//    double longitude   = root["data"][1];
// ----------------------------------------------------

            if ( thisChar == 13 )   // Carriage return
            {
                JsonObject& root = jsonBuffer.parseObject(received_command);

                short port    = root["port"];
                boolean state = root["command"];
                Serial.print("Port = ");
                Serial.print(port);
                Serial.print(", State = ");
                Serial.print(state);
                
                digitalWrite(port, state);   // turn the LED on (HIGH is the voltage level)
                received_command = "";
            }
        }
    }
}
