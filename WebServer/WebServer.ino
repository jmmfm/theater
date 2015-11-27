/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi

 */

#include <ArduinoJson.h>
#include <SPI.h>
#include <Ethernet.h>

int led3 = 6;
int led2 = 5;
int led1 = 4;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip( 10, 50, 16, 100);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // initialize the digital pin as an output.
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);

    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip);
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
}

StaticJsonBuffer<200> jsonBuffer;
String received_command = "";

void loop()
{
    // listen for incoming clients
    EthernetClient client = server.available();
    if (client)
    {
        Serial.println("new client");
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                Serial.write(c);

                received_command += c;

                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");  // the connection will be closed after completion of the response
                    client.println("Refresh: 5");  // refresh the page automatically every 5 sec
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");
                    // output the value of each analog input pin
                    for (int led = 4; led <= 6; led++)
                    {
                        int reading = digitalRead(led);
                        client.print("led ");
                        client.print(led);
                        client.print(" is ");
                        client.print(reading);
                        client.println("<br />");
                    }
                    client.println("<br />");
                    client.print("<p>Received Command : <b>"); client.print(received_command); client.println("</b></p>");
                    Serial.print("Received command : ");
                    Serial.println(received_command);
                    JsonObject& root = jsonBuffer.parseObject(received_command);

                    if (!root.success())
                    {
                        Serial.println("parseObject() failed");
                        client.println("<p>parseObject() failed!! :-( </p>");
                        received_command = "";
                    }
                    else
                    {
                        short port    = root["port"];
                        boolean state = root["command"];
                        Serial.println("");
                        Serial.print("Port = ");
                        Serial.print(port);
                        Serial.print(", State = ");
                        Serial.print(state);

                        client.print("<p><b>Port</b> = "); client.print(port); client.print(", <b>State</b> = "); client.print(state); client.println("</p>");

                        char p[3];

                        if (state)
                        {
                            digitalWrite(port, HIGH);    // turn the LED on (HIGH is the voltage level)
                            // write back to client
                            client.print("Led ");
                            sprintf(p, "%d", port);        // convert int to string
                            client.write(p);
                            client.write(" ligado.\n");
                            client
                        }
                        else
                        {
                            digitalWrite(port, LOW);    // turn the LED off (LOW is the voltage level)
                            // Write back to client
                            client.write("Led ");
                            sprintf(p, "%d", port);        // convert int to string
                            client.write(p);
                            client.write(" desligado.\n");
                        }

                        received_command = "";
                    }

                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disconnected");
        Ethernet.maintain();
    }
}
