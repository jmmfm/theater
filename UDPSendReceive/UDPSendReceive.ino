
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
//#define PACKET_MAX_SIZE 64  // it is expected for 5 lamps and DMX one string with 


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];         // buffer to hold incoming packet,
//char ReplyBuffer[] = "acknowledged";             // a string to send back when one UDP packet is received
char ReplyInfo[] = "{'simple_lamps':5,'DMX':1}";   // This is some JSON string to reply in case of info request from the server side (can be "intensity_lamps", "simple_lamps" and "DMX")
byte lamps[MAX_LAMPS] = {4,6,7,8,9};               // PIN 5 CANNOT BE USED BECAUSE IT IS USED BY DMX CONTROLL
int lamps_status[MAX_LAMPS] = {0,0,0,0,0};         // this is used to save the status of the lamps everytime that they change.

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;





char * deblank(char *str)
{
    char *out = str, *put = str;
    for(; *str != '\0'; ++str)
    {
        if(*str != ' ') *put++ = *str;
    }
    *put = '\0';
    return out;
}





void loadEthernetConfigurations(IPAddress newIP)
{
    // Start the Ethernet and UDP
    Ethernet.begin(mac, newIP);
    Udp.begin(localPort);
}




void setup()
{
    // led ports configuration
    for (short i=0; i<MAX_LAMPS; i++) pinMode(lamps[i], OUTPUT);

    //IPAddress ip(192, 168,   1, 177);
    //IPAddress ip( 10,  50,  19, 212);
    IPAddress ip(169, 254, 248, 212);  //  169.254.248.212   (for copying purposes because it is defined with commas instead of points ".")
    
    loadEthernetConfigurations(ip);
    
    Serial.begin(9600);
    Serial.print("UDP_TX_PACKET_MAX_SIZE = ");
    Serial.println(UDP_TX_PACKET_MAX_SIZE);
}



bool DMX_cha = false, DMX_val = false;
byte DEBUG = 0;
short channel, value;

void loop()
{
    // if there's data available, read a packet
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        // --- DEBUG LEVEL 1 ---
        if (DEBUG >= 1)
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
        }
        // --- END DEBUG ---

        // read the packet into packetBufffer
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        // --- DEBUG LEVEL 1 ---
        if (DEBUG >= 1)
        {
            Serial.print("Contents:");
            Serial.println(packetBuffer);
            
//            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//            Udp.write("Contents: ");
//            Udp.write(packetBuffer);
//            Udp.endPacket();
        }
        // --- END DEBUG ---


        if (strcmp(packetBuffer,"info") == 0)
        {
            // --- DEBUG LEVEL 2 ---
            if (DEBUG >=2)
            {
                Serial.print("Reply: ");
                Serial.println(ReplyInfo);
            }
            // --- END DEBUG ---

            // info command always returns one UDP answer
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(ReplyInfo);
            Udp.endPacket();
        }
        else if (strcmp(packetBuffer,"status") == 0)
        {
            // --- DEBUG LEVEL 2 ---
            if (DEBUG >=2)
            {
                Serial.print("Status: ");
                Serial.println(ReplyInfo);
            }
            // --- END DEBUG ---
            
            // Status command always returns one UDP answer
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write("{");
            for (int i; i<MAX_LAMPS; i++)
            {
                char small_buffer[6];
                sprintf(small_buffer, "'L%d':%d", i+1,lamps_status[i]);
                Udp.write(small_buffer);
                // Udp.write("L");
                // Udp.write(to_string(i+1));
                // Uddp.write(":");
                // Udp.write(to_string(lamps_status[i]))
                // --- DEBUG LEVEL 2 ---
                if (DEBUG >= 2)
                {
                    Serial.print(small_buffer);
                    if (i < MAX_LAMPS - 1 )
                        Serial.print(",");
                }
                
                if (i < MAX_LAMPS - 1 )
                    Udp.write(",");
            }
            Udp.write("}");
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
            
                    // Do something with cmd and val ---------------------------------------
                    
//                    Serial.print("command = ");
//                    Serial.println(cmd);
//                    Serial.print("value = ");
//                    Serial.println(val);
                    if (command[0] == 'L')
                    {
                        if (atoi(val.c_str()))
                        {
                            // --- DEBUG LEVEL 2 ---
                            if (DEBUG >= 2)
                            {
                                Serial.print("port ");
                                Serial.print(lamps[command[1]-'0'-1]);
                                Serial.println(" HIGH");
                            }
                            // --- END DEBUG ---
                            
                            digitalWrite( lamps[command[1]-'0'-1], HIGH);
                            lamps_status[command[1]-'0'-1] = atoi(val.c_str());

                            // --- DEBUG LEVEL 2 ---
                            if (DEBUG >= 2)
                            {
                                
                              
                            }
                        }
                        else
                        {
                            // --- DEBUG LEVEL 2 ---
                            if (DEBUG >= 2)
                            {
                                Serial.print("port ");
                                Serial.print(lamps[command[1]-'0'-1]);
                                Serial.println(" LOW");
                            }
                            // --- END DEBUG ---
                            digitalWrite( lamps[command[1]-'0'-1], LOW );
                            lamps_status[command[1]-'0'-1] = 0;
                        }
                    }
                    else
                    {
                        if(strcmp(command,"DMX_cha") == 0)
                        {
                            // --- DEBUG LEVEL 2 ---
                            if (DEBUG >= 2)
                            {
                                Serial.print("Selecting DMX channel: ");
                                Serial.println(val);
                            }
                            // --- END DEBUG ---
                            DMX_cha = true;
                            channel = atoi(val.c_str());
                        }
                        else if (strcmp(command,"DMX_val") == 0)
                        {
                            // --- DEBUG LEVEL 2 ---
                            if (DEBUG >= 2)
                            {
                                Serial.print("Selecting DMX value: ");
                                Serial.println(val);
                            }
                            // --- END DEBUG ---
                            DMX_val = true;
                            value = atoi(val.c_str());
                        }
                        else if (strcmp(command,"debug") == 0)
                        {
                            if (atoi(val.c_str()))
                            {
                                Serial.println("DEBUG mode ON.");
                                DEBUG = atoi(val.c_str());
                                Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                                Udp.write("DEBUG mode ON.");
                                Udp.endPacket();
                            }
                            else
                            {
                                Serial.println("DEBUG mode OFF");
                                DEBUG = false;
                                Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                                Udp.write("DEBUG mode OFF.");
                                Udp.endPacket();
                            }
                        }
                        else
                        {
                            // --- DEBUG LEVEL 1 ---
                            if (DEBUG >= 1) Serial.println("Wrong command!!");
                            // --- END DEBUG ---
                            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                            Udp.write("NOK");
                            Udp.endPacket();
                        }

                        // --- DEBUG LEVEL 3 ---
                        if (DEBUG >= 3)
                        {
                            Serial.print("DMX_cha = ");
                            Serial.print(DMX_cha);
                            Serial.print(", DMX_val = ");
                            Serial.println(DMX_val);
                        }
                        // --- END DEBUG ---

                        if (DMX_cha==true && DMX_val==true)
                        {
                            // --- DEBUG LEVEL 2 ---
                            if (DEBUG >= 2)
                            {
                                Serial.print("D");
                                Serial.print(channel);
                                Serial.print(":");
                                Serial.println(value);
                            }
                            // --- END DEBUG ---
                            
                            DmxSimple.write(channel, value);

                            // --- DEBUG LEVEL 2 ---
                            if (DEBUG >= 2)
                            {
                                Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                                Udp.write("D");
                                char buf[3];
                                Udp.write(itoa(channel,buf,10));
                                Udp.write(":");
                                Udp.write(itoa(value,buf,10));
                                Udp.endPacket();
                            }
                            // --- END DEBUG ---
                            
                            DMX_cha = false;
                            DMX_val = false;
                        }
                    }
                }
                
                // Find the next command in input string
                command = strtok(NULL, ",");
            }
        }
        
        memset(packetBuffer, 0, UDP_TX_PACKET_MAX_SIZE);
        
        if (DEBUG) Serial.println("--------------------------");
    }
    delay(5);
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
