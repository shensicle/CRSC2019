/*
Copyright 2019 CANARIE Inc. All Rights Reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY CANARIE Inc. "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <WiFiServerSecure.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiType.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFiAP.h>
#include <WiFiClient.h>
#include <BearSSLHelpers.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#include <WiFiServerSecureBearSSL.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiServerSecureAxTLS.h>
#include <Ticker.h>

#include <IFTTTMessage.h>

#include "CRSCConfig.h"
#include "CRSCSerialInterface.h"
#include "CRSCLED.h"


// -------------------------------------------------------
IFTTTMessageClass IFTTTSender;   // Communicates with ifttt.com

// LED stuff - use built-in LED connected to D2
const int TheLEDPin = LED_BUILTIN;

// Define the object that controls the LED
CRSCLED TheLED (TheLEDPin);

// Takes care of running the LED flashing function periodically
Ticker LEDFlasher;

// How often to run out main loop (milliseconds)
#define UPDATE_INTERVAL    50
#define UPDATE_INTERVAL_FP 0.05

// Configuration object to load/store information in EEPROM, including our own Board ID and the other Board IDs we
// have collected.
CRSCConfigClass TheConfiguration;

// Make a serial interface so user can communicate with us from a computer
CRSCSerialInterface TheSerialInterface (&TheConfiguration);



// -------------------------------------------------------
void setup() 
{

  // Start serial communication for terminal interface
  Serial.begin(115200); 

  while (! Serial );

  PrintLogo();

  // Load our configuration here. If anything goes wrong, turn the
  // LED off and give up.
  bool okay = TheConfiguration.Load();


  // If the configuration checksum test passed and all stored board IDs are valid ...
  if (okay == true)
  {
    Serial.print ("\nWelcome to CANARIE's CRSC Scavenger Hunt\n\n");

    // We can now initialize fields to be sent to IFTTT that were in the personality
    IFTTTSender.Initialize (TheConfiguration.GetIFTTTKey(), TheConfiguration.GetBoardID()); 

    // If our board ID has not yet been set ...
    if (memcmp (TheConfiguration.GetBoardID(), UninitializedID, BOARD_ID_LEN) == 0) 
    {
        Serial.print(F("*** I'm so sorry. It seems that your board ID is missing. Please get help from CANARIE staff - but only the techies\n\n"));
    }
    // Tell the LED object about our fingerprint so it can flash accordingly
    unsigned long thePrint = TheConfiguration.GetFingerprint();
    TheLED.SetFingerprint (thePrint);

    // Attach the LED update function to the ticker object that periodically calls it
    LEDFlasher.attach (UPDATE_INTERVAL_FP, ServiceLED);   // 50 milliseconds
  }
  else
  {
      Serial.print ("\nWell this is embarassing! Your board seems to be corrupted. Please contact CANARIE staff\n\n");
      digitalWrite (TheLEDPin, 1);    // Turns LED Off
  }

  Serial.flush();
}

// -------------------------------------------------------
void loop() 
{
    // Flag which, when set, indicates that we have sent a message to ifttt.com and
    // don't need to do it again
    static bool done = false;

    // Check the serial interface for a complete command and, if there is one, execute it
    TheSerialInterface.Update();

    // If we now have all the scavenged board ID's we need ...
    if ((TheConfiguration.GetNumScavengedBoardIDs() == SCAVENGED_BOARD_LIST_LEN) && (done == false))
    {
        // Disconnect the LED flasher
        LEDFlasher.detach();

        // Set LED on as an indication to the user
        digitalWrite (TheLEDPin, 0);

       // Connect to Wifi
       ConnectWifi(TheConfiguration.GetWifiSSID(), TheConfiguration.GetWifiPassword()); 

       // If wifi connected, send a message to ifttt.com
       if (WiFi.status() == WL_CONNECTED)
       {
          // And send an appropriate message to ifttt.com
          IFTTTSender.Initialize(TheConfiguration.GetIFTTTKey(), TheConfiguration.GetBoardID());

          if (IFTTTSender.Send ("Task Complete") == true)
          {
              // We have successfully sent our message
              done = true;
          }
       }
        
    }

    // serialEvent isn't auto-called on 8266 so do it ourselves
    serialEvent();
    delay (UPDATE_INTERVAL);
}


// -------------------------------------------------------
// Flash the LED either a short pulse or a long pulse, depending
// on the next byte in the board ID
void ServiceLED (void)
{ 
   // LED object does the work
   TheLED.Update();      
}

// -------------------------------------------------------
// This function is automatically called between repetitions of loop()
void serialEvent() 
{
  while (Serial.available()) 
  {
    // Get the new character
    char inChar = (char)Serial.read(); 

    // And add it to the command processor
    TheSerialInterface.Add(inChar);  
  }
}

// --------------------------------------------------------------------------------------------------
void ConnectWifi(char* ssid, char* password)  // Tries to connect to the wireless access point with the credentials provided
{
    bool timeOut = 0; // Change to 1 if connection times out
    byte attempts = 0;   // Counter for the number of attempts to connect to wireless AP
  
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    WiFi.begin(ssid, password); // Connect to WiFi network

    while (WiFi.status() != WL_CONNECTED && (timeOut == 0)) // Test to see if we're connected
    {
        Serial.print('.');
        attempts++;
    
        if(attempts > 60) 
            break; // Give up after ~30 seconds
        else 
            delay(500);      // Check again after 500ms
    }
  
    if (WiFi.status() == WL_CONNECTED)  // We're connected
    {
        Serial.println("\nWiFi connected");
    }
    else  // Unable to connect
    {
        WiFi.disconnect();
        Serial.println("\nConnection Timed Out!\r\n");
    }
}

// --------------------------------------------------------------------------------------------------------
void PrintLogo (void)
{
  Serial.print ("\n\n\n");
  Serial.println(F("________/\\\\\\\\\\\\\\\\\\____/\\\\\\\\\\\\\\\\\\_________/\\\\\\\\\\\\\\\\\\\\\\__________/\\\\\\\\\\\\\\\\\\________"));         
  Serial.println(F(" _____/\\\\\\////////___/\\\\\\///////\\\\\\_____/\\\\\\/////////\\\\\\_____/\\\\\\////////_________"));       
  Serial.println(F("   __/\\\\\\_____________\\/\\\\\\\\\\\\\\\\\\\\\\/______\\////\\\\\\__________/\\\\\\___________________"));      
  Serial.println(F("    _\\/\\\\\\_____________\\/\\\\\\//////\\\\\\_________\\////\\\\\\______\\/\\\\\\___________________"));     
  Serial.println(F("     _\\//\\\\\\____________\\/\\\\\\____\\//\\\\\\___________\\////\\\\\\___\\//\\\\\\__________________"));    
  Serial.println(F("      __\\///\\\\\\__________\\/\\\\\\_____\\//\\\\\\___/\\\\\\______\\//\\\\\\___\\///\\\\\\________________"));   
  Serial.println(F("       ____\\////\\\\\\\\\\\\\\\\\\_\\/\\\\\\______\\//\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/______\\////\\\\\\\\\\\\\\\\\\_______"));  
  Serial.println(F("        _______\\/////////__\\///________\\///____\\///////////___________\\/////////______"));                                         

}
