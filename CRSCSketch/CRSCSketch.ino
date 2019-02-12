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

// Wifi and IP stack for esp8266 host
#include <ESP8266WiFi.h>
//#include <time.h>

// Software timer to control LED flash rate
#include <Ticker.h>

// Send a message to ifttt.com, to send an email when this board's scavenger hunt is complete
#include "IFTTTMessage.h"

// Other functionality specific to the CRSC scavenger hunt
#include "CRSCConfig.h"
#include "CRSCSerialInterface.h"
#include "CRSCLED.h"

// -------------------------------------------------------

IFTTTMessageClass IFTTTSender;   // Object to communicate with ifttt.com

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

  // Seems to minimize garbage characters on reset
  while (! Serial );

  // Compromise with Marketing department
  PrintLogo();

  // Load our configuration here. If anything goes wrong, turn the
  // LED off and give up.
  bool okay = TheConfiguration.Load();


  // If the configuration checksum test passed and all stored board IDs are valid ...
  if (okay == true)
  {
    Serial.print (F("\nWelcome to CANARIE's CRSC Scavenger Hunt\n\n"));

    // We can now initialize fields to be sent to IFTTT that were in the personality
    IFTTTSender.Initialize (TheConfiguration.GetIFTTTKey(), TheConfiguration.GetBoardID(), "CRSCGadget"); 

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
      Serial.print (F("\nWell, this is embarassing! Your board seems to be corrupted. Please contact CANARIE staff\n\n"));
      digitalWrite (TheLEDPin, 1);    // Turns LED off. LED is active low.
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

       // If wifi connected,
       if (WiFi.status() == WL_CONNECTED)
       {
          // Send an appropriate message to ifttt.com
          done = SendToIFTTT("We have a winner!");
       }        
    }

    // This would only be done during a production test
    if (TheConfiguration.WifiTestRequested())
    {
        Serial.println (F("Wifi test initiated\n"));
       
       // Connect to Wifi
       ConnectWifi(TheConfiguration.GetWifiSSID(), TheConfiguration.GetWifiPassword()); 

       // If wifi connected,
       if (WiFi.status() == WL_CONNECTED)
       {
          // Send an appropriate message to ifttt.com
          done = SendToIFTTT("Connectivity test");

          if (done == true)
          {
              Serial.println (F("Wifi test complete - check for email from ifttt\n"));
          }
          else
          {
            Serial.println (F("Unable to send message to ifttt\n"));
          }
       }        
       TheConfiguration.ClearWifiTestMode();
    }

    // serialEvent isn't auto-called on 8266, for some reason, so do it ourselves
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
    // After a timeout, number of millisecons to wait until trying again
    static int millisecondsToRetry = UPDATE_INTERVAL;
    
    byte attempts = 0;   // Counter for the number of attempts to connect to wireless AP

    millisecondsToRetry -= UPDATE_INTERVAL;

    if (millisecondsToRetry <= 0)
    {
      Serial.print(F("Connecting to ")); Serial.println(ssid);
  
      WiFi.begin(ssid, password); // Connect to WiFi network

      while (WiFi.status() != WL_CONNECTED) // Test to see if we're connected
      {
          Serial.print('.');
          attempts++;
    
          if(attempts > 20) // Give up after 20 tries 
          {
            Serial.println (F("\nWifi connection failed. Will try again in 10 seconds."));
            Serial.println (F("In the mean time, please notify one of the CANARIE staff that you have completed the scavenger hunt\n\n"));
            millisecondsToRetry = 10000;
            break; 
          }
          else 
          {
            delay(500);      // Check again after 500ms
          }
      }
  
      if (WiFi.status() == WL_CONNECTED)  // We're connected
      {
         Serial.println(F("\nWiFi connected ...\n"));
      }
      else  // Unable to connect
      {
         WiFi.disconnect();
      }
   }
}

// --------------------------------------------------------------------------------------------------------
// Attempt to send a message to IFTTT and return a flag which, when set, indicates success
bool SendToIFTTT(char* theMessage)
{
  static int millisecondsToRetry = UPDATE_INTERVAL;
  bool returnValue = false;

  millisecondsToRetry -= UPDATE_INTERVAL;

  if (millisecondsToRetry <= 0)
  {
    returnValue = IFTTTSender.Send (theMessage);

    // If this was not successful ...
    if (returnValue == false)
    {
      millisecondsToRetry = 10000;
      Serial.println (F("\nConnection to ifttt.com failed. Will try again in 10 seconds."));
      Serial.println (F("In the mean time, please notify one of the CANARIE staff that you have completed the scavenger hunt\n\n"));
    }
  }  
  return (returnValue);
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
  Serial.println(F("        _______\\/////////__\\///________\\///____\\///////////___________\\/////////________\n"));                                         

}
