
/*
 UDPSendReceiveString:
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender

 A Processing sketch is included at the end of file that can be used to send
 and received messages for testing with a computer.

 created 21 Aug 2010
 by Michael Margolis

 This code is in the public domain.


PIN 5 will be used for DMX control, as well as TX and RX (PIN 1 and 0, respectivelly)
PIN 10,11,12,13 are used by the Ethernet (I2C)

 */


#include <SPI.h>          // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <DmxSimple.h>

#define MAX_LAMPS 5
#define PACKET_MAX_SIZE 64  // it is expected for 5 lamps and DMX one string with 


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress ip(192, 168,  1, 177);
IPAddress ip(169,254,248,212);  //  169.254.248.212   (for copying purposes because it is defined with commas instead of points ".")
//IPAddress ip(10,50,19,212);

unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[PACKET_MAX_SIZE];                 // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";                // a string to send back when one UDP packet is received
char ReplyInfo[] = "{'simple_lamps':5,'DMX':1}";    // This is some JSON string to reply in case of info request from the server side (can be "intensity_lamps", "simple_lamps" and "DMX")
short lamps[MAX_LAMPS] = {4,6,7,8,9};               // PIN 5 CANNOT BE USED BECAUSE IT IS USED BY DMX CONTROLL

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup()
{
    // led ports configuration
    for (short i=0; i<MAX_LAMPS; i++) pinMode(lamps[i], OUTPUT);

    // start the Ethernet and UDP:
    Ethernet.begin(mac, ip);
    Udp.begin(localPort);

    Serial.begin(9600);
    Serial.print("PACKET_MAX_SIZE = ");
    Serial.println(PACKET_MAX_SIZE);
}


void loop()
{
    // if there's data available, read a packet
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remote = Udp.remoteIP();
        for (int i = 0; i < 4; i++)
        {
            Serial.print(remote[i], DEC);
            if (i < 3) Serial.print(".");
        }

        Serial.print(", port ");
        Serial.println(Udp.remotePort());

        // read the packet into packetBufffer
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        Serial.println("Contents:");
        Serial.println(packetBuffer);

        if (strcmp(packetBuffer,"info") == 0)
        {
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(ReplyInfo);
            Udp.endPacket();
        }
        else
        {
            // This is the expexted string. The buffer only allows 24 char at most, so the command has to be splitted.
            // "DMX_cha:12,DMX_val:128"     "L1:20,L2:0,L3:1"        "L4:100,L5:0"
            
            // NOTE: Need to split string by comma to get the different commands
            //       For each command read the action and the value

            // Read each command pair 
            char* command = strtok(packetBuffer, ",");
            while (command != NULL)
            {
                // Split the command in two values
                char* separator = strchr(command, ':');
                if (separator != 0)
                {
                    // Actually split the string in 2: replace ':' with 0
                    *separator = 0;
                    String cmd = command;
                    ++separator;
                    String val = separator;
            
                    // Do something with cmd and val
                    Serial.print("command = ");
                    Serial.println(cmd);
                    Serial.print("value = ");
                    Serial.println(val);        
                }
                // Find the next command in input string
                command = strtok(NULL, ",");
            }
            

//            DmxSimple.write(channel, value);

            
            Serial.println("--------------------------");
        }
        



        memset(packetBuffer, 0, UDP_TX_PACKET_MAX_SIZE);
    }
    delay(10);
}


/*
  Processing sketch to run with this example
 =====================================================

// Processing UDP example to send and receive string data from Arduino
// press any key to send the "Hello Arduino" message

import hypermedia.net.*;
UDP udp;  // define the UDP object

void setup()
{
    udp = new UDP( this, 6000 );  // create a new datagram connection on port 6000
    //udp.log( true );            // <-- printout the connection activity
    udp.listen( true );           // and wait for incoming message
}

void draw()
{
}

void keyPressed()
{
    String ip       = "192.168.1.177";  // the remote IP address
    int port        = 8888;             // the destination port

    udp.send("Hello World", ip, port ); // the message to send
}

//void receive( byte[] data, String ip, int port )  // <-- extended handler
void receive( byte[] data )                         // <-- default handler
{
    for(int i=0; i < data.length; i++)
        print(char(data[i]));
    println();
}

*/
