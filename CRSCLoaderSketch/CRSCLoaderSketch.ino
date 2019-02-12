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


// Configuration values that are the same for all boards
//#define DEFAULT_WIFI_SSID "SSID goes here"
//#define DEFAULT_WIFI_PASSWORD "Password goes here"
//#define DEFAULT_IFTTT_KEY "Key goes here"

// -------------------------------------------------------
IFTTTMessageClass IFTTTSender;   // Communicates with ifttt.com


// How often to run out main loop (milliseconds)
#define UPDATE_INTERVAL    50

// Configuration object to load/store information in EEPROM
CRSCConfigClass TheConfiguration;

// A string to store our board ID, provided by the user over the serial port
char BoardIDString [BOARD_ID_BYTES+BOARD_ID_CHECK_BYTES+1];  // +1 for the null terminator

// Index into BoardIDString
int BoardIDIndex = 0;

// A flag which, when set, indicates that the user has provided a valid board ID for this board
bool BoardIDSet = false;

// -------------------------------------------------------
void setup() 
{
    // Start serial communication for terminal interface
    Serial.begin(115200); 

    // while (! Serial );

    Serial.print (F("\nWelcome to CANARIE's CRSC Swag Initialization and Self-test\n\n"));

    // Initialize the Configuration structure (except out board ID which will be provided
    // by the user below) and write to non-volatile storage
    TheConfiguration.Initialize (DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD, DEFAULT_IFTTT_KEY);
  
  
    // We can now initialize fields to be sent to IFTTT
    IFTTTSender.Initialize (TheConfiguration.GetIFTTTKey(), TheConfiguration.GetBoardID()); // have to make last one printable

    // Turn LED on so we know it works
    pinMode (LED_BUILTIN, OUTPUT);
    digitalWrite (LED_BUILTIN, 0);

    Serial.print (F("\nMake sure the LED works. It should be on now\n\n"));
    Serial.print (F("\nEnter board ID: "));
    Serial.flush();
}

// -------------------------------------------------------
void loop() 
{
    static bool weAreDone = false;

    if (BoardIDSet && (weAreDone == false))
    {
        if (TheConfiguration.SetBoardID (BoardIDString))
        {
  
            // We're all finished. Print out a final message and disable further operations
            weAreDone = true;

            Serial.print (F("ID for board "));
            Serial.print (TheConfiguration.GetBoardID());
            Serial.println (F(" loaded. Sending message to ifttt.com\n"));

            // Connect to Wifi
            ConnectWifi(TheConfiguration.GetWifiSSID(), TheConfiguration.GetWifiPassword()); 

            // If wifi connected, send a message to ifttt.com
            if (WiFi.status() == WL_CONNECTED)
            {
               // And send an appropriate message to ifttt.com
               IFTTTSender.Initialize(TheConfiguration.GetIFTTTKey(), TheConfiguration.GetBoardID());

               if (IFTTTSender.Send ("Board Configuration and Test Complete") == true)
               {
                   // We have successfully sent our message
                   Serial.println (F("Message successfully sent to ifttt.com\n"));
               }
               else
               {
                    Serial.print (F("*** ERROR: Unable to send message via ifttt.com ***\n")); 
                }
            }
            else
            {
                Serial.print (F("*** ERROR: Unable to connect to WIFI ")); Serial.print(TheConfiguration.GetWifiSSID());
                Serial.println (F("***\n"));
            }

            Serial.println("Please verify that a message has been received from ifttt.com and load conference software.");
            Serial.println(F("Have a nice day\n"));
        }
        else
        {
            // Board string was invalid - reset and prompt user to try again
            Serial.println (F("\n*** ERROR: Oh no, the Board ID you entered is invalid - try again\n"));

            BoardIDString[4] = 0x00;
            Serial.print ("Should be "); Serial.print (BoardIDString);
            char temp[3];
            TheConfiguration.CalculateCheckBytes (BoardIDString,temp); temp[2] = 0x00;
            Serial.print (temp); Serial.println();
            
            BoardIDSet = false;
            BoardIDIndex = 0;
        }
      
    }
    serialEvent();           // Should be called automatically - isn't
    delay (UPDATE_INTERVAL);
}


// -------------------------------------------------------
// This function is automatically called between repetitions of loop()
void serialEvent() 
{

  while (Serial.available() && (BoardIDIndex < BOARD_ID_BYTES+BOARD_ID_CHECK_BYTES)) 
  {
    // Get the new character
    char inChar = (char)Serial.read(); 

    // Skip over termination characters in case the user's terminal emulator sends them
    if ((inChar != '\n') && (inChar != '\r'))
    {
        // And add it to the Board ID
        BoardIDString[BoardIDIndex++] = inChar;
    }
  }

  // If we're done, null terminate the string and tell loop()
  if (BoardIDIndex == BOARD_ID_BYTES+BOARD_ID_CHECK_BYTES)
  {
      BoardIDSet = true;
      BoardIDString[BoardIDIndex] = 0x00;
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
        Serial.println("\n*** ERROR: WIFI Connection Timed Out! ***\r\n");
    }
}
