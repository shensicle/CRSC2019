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

/*
 * Probably a little theory is in order, for my future self. This sketch and
 * the libraries it includes, implement an electronic scavenger hunt for the
 * 2019 Canadian Research Software Conference (CRSC). The purpose of this hunt is
 * to encourage the attendees to meet each other.
 * 
 * Everyone is given a pre-programmed NodeMCU board and a USB cable. Each NodeMCU
 * has a unique board ID. The board ID is 4 characters and there are two check
 * characters on the end, calculated using an algorithm I thought up on a bus.
 * 
 * The board ID also maps onto an LED flash code. When powered up, the on-board LED
 * will start to flash a 4-pulse code made up of combinations of long and short pulses. In this
 * code, this pattern of pulses is known as a 'fingerprint'. Although
 * each board has a unique ID, different board IDs can map onto the same fingerprint. For
 * CRSC 2019 I am assuming 100 boards with 10 different fingerprints. 
 * 
 * For the scavenger hunt, a participant must find another board exhibiting the same flash code as
 * theirs, introduce themselves to the owner of that board, and exchange board IDs. They then type this
 * new board ID into their board over the serial interface using the 'A' command. 'H' displays an 
 * online help.
 * 
 * Once someone scavenges 5 board IDs from others, their board's LED turns solid and the board sends
 * an email to us (via ifttt.com). Whoever is first wins.
 * 
 * Board configuration such as the board ID, credentials for the local wifi network and the ifttt API
 * key are store in EEPROM (actually, on the NodeMCU, EEPROM seems to be emulated in flash). When the board is
 * booted the first time after flashing, the contents of EEPROM will be invalid. EEPROM can then
 * be initialized using the 'R' command, which does not appear in the online help. The 'R' command
 * also requires a security code so that participants playing around can't easily reconfigure their boards.
 * 
 * There is also a 'W' command which is intented for production/test use and is similarly
 * undocumented. The 'W' command sends a test message to ifttt.com, validating both the
 * ifttt credentials and wifi connectivity.
*/

// Wifi and IP stack for esp8266 host
#include <ESP8266WiFi.h>

// Software timer to control LED flash rate
#include <Ticker.h>

// Send a message to ifttt.com, to send an email when this board's scavenger hunt is complete
#include "IFTTTMessage.h"

// Other functionality specific to the CRSC scavenger hunt
#include "CRSCConfig.h"
#include "CRSCSerialInterface.h"
#include "CRSCLED.h"

// -------------------------------------------------------

// Remember to update this just before the commit
#define FIRMWARE_VERSION "1.0"

// How often to run out main loop (milliseconds)
#define UPDATE_INTERVAL    50

IFTTTMessageClass IFTTTSender (UPDATE_INTERVAL);   // Object to communicate with ifttt.com

// Messages to send to ifttt when scavenger hunt has been completed or if we are in test mode
const char* DoneMsg = "Scavenger hunt is complete!";
const char* TestMsg = "Connectivity test";
char* CurrMsg;

// LED stuff - use built-in LED connected to D2
const int TheLEDPin = LED_BUILTIN;

// Define the object that controls the LED
CRSCLED TheLED (TheLEDPin, (float)UPDATE_INTERVAL);

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

  // Seems to reduce (but not eliminate) garbage characters on reset
  while (! Serial );

  // Compromise with Marketing department :)
  PrintLogo();

  // Load our configuration here. If anything goes wrong, turn the
  // LED off and give up.
  bool okay = TheConfiguration.Load();

  // If the configuration checksum test passed and all stored board IDs are valid ...
  if (okay == true)
  {
    Serial.print (F("\nWelcome to CANARIE's CRSC Scavenger Hunt (Firmware Version "));Serial.print (FIRMWARE_VERSION); Serial.println(")\n\n");

    // We can now initialize fields to be sent to IFTTT that were in the personality
    IFTTTSender.Initialize (TheConfiguration.GetIFTTTKey(), TheConfiguration.GetBoardID(), "CRSCGadget"); 

    // If our board ID has not yet been set ...
    if (memcmp (TheConfiguration.GetBoardID(), UninitializedID, BOARD_ID_LEN) == 0) 
    {
        Serial.print(F("*** I'm so sorry. It seems that your board ID is missing. Please get help from CANARIE staff - but only the techies\n\n"));
    }
    else
    {
        // Tell the user what they can do
        TheSerialInterface.DisplayHelp();
    }
    
    // Tell the LED object about our fingerprint so it can flash accordingly
    TheLED.SetFingerprint (TheConfiguration.GetFingerprint());
  }
  else
  {
      Serial.print (F("\nWell, this is embarassing! Your board seems to be corrupted. Please contact CANARIE staff - but only the techies\n\n"));
      TheLED.SetOff();
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

    // If we now have all the scavenged board ID's we need, or if we're in production and a Wifi test
    // has been requested ...
    if (((TheConfiguration.GetNumScavengedBoardIDs() == SCAVENGED_BOARD_LIST_LEN) && (done == false)) || TheConfiguration.WifiTestRequested())
    {
        // If we are just testing the wifi
        if (TheConfiguration.WifiTestRequested() == true)
        {
          CurrMsg = (char*)TestMsg;
        }
        else
        {
           // This is real. Scavenger hunt has been completed
           CurrMsg = (char*)DoneMsg;
           
           // Set LED on as an indication to the user
           TheLED.SetOn();   
        }

       // Connect to Wifi
       ConnectWifi(TheConfiguration.GetWifiSSID(), TheConfiguration.GetWifiPassword()); 

       // If wifi connected,
       if (WiFi.status() == WL_CONNECTED)
       {
          // Send an appropriate message to ifttt.com
          done = IFTTTSender.SendMessage(CurrMsg);

          // If send to ifttt failed ...
          if (done == false)
          {
              Serial.println(F("Unable to send to ifttt - will keep trying"));
          }
          else    // send was successful
          {
              // If this was just a Wifi test, set done back to false so the hunt can
              // continue if someone forgets to reboot the board.
              if (TheConfiguration.WifiTestRequested())
              {
                 TheConfiguration.ClearWifiTestMode();
                 done = false;
              }
          }
       }        
    }


    // serialEvent isn't auto-called on 8266 for some reason, so do it ourselves
    serialEvent();
    delay (UPDATE_INTERVAL);
}

// -------------------------------------------------------
// This function is called during each repetition of loop()
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
// Tries to connect to the wireless access point with the credentials provided.  The idea is to call
// this multiple times, until connection is established, so that the board can continue to do other
// things. Also, because some of the features of the Wifi class seem to require background processing.
void ConnectWifi(char* ssid, char* password)  
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
void PrintLogo(void)
{
  Serial.print ("\n\n\n");
  Serial.println(F("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@@@&(/#@@@@@@@@@@@@@@@@@@@@@@@@@(/(@@@@@@@@@@@@@@@@@@@@@@@@@@%//%@@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@.       ,@@@@@@@@@@@@@@@@@@@.       *@@@@@@@@@@@@@@@@@@@@#        &@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@&.  &%%%(  ,@@@@@@@@@@@@@@@@@.  ,,,,,   @@@@@@@@@@@@@@@@@@%  *((((.  &@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@&  .%%%%%. .@@@@@@@@@@@@@@@@@   ,,,,,   @@@@@@@@@@@@@@@@@@/  (((((*  &@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@/           .%%%%%.                      ,,,,,                        (((((*          &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/  ,,,,,,,,,,,,,,,,,,,,,,,,,,,  *(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/  ,,,,,,,,,,,,,,,,,,,,,,,,,,,  ,(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/  ,,,,,,,,,,,,,,,,,,,,,,,,,,,  ,(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/       ,,,,,,,,,,,,,,,,,       ,(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%%%%%%%,  ,,,,,,,,,,,,,,,  .(((((((((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   ,,,,,,,,,,,,,   ((((((((((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%%%%%%&(  .,,,,,,,,,,,,,.  *(((((((((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/       ,,,,,,,,,,,,,,,,,       ,(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/  ,,,,,,,,,,,,,,,,,,,,,,,,,,,  ,(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/  ,,,,,,,,,,,,,,,,,,,,,,,,,,,  ,(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@/  (%%%%%%%%%%%%%%%%%%%%%%%/  ,,,,,,,,,,,,,,,,,,,,,,,,,,,  ,(((((((((((((((((((((((*  &@@@@@@@"));
  Serial.println(F("@@@@@#          ,&%%%(                          .,,,,.                     (((((.          &@@@@@@@"));
  Serial.println(F("@@@@@@@@@@&%%%(   ,&%(  *%%%%%%%%@@%,,,,,,,,,,  .,,.   .,,,,/@@&((((((((,  (((.   /(((%@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@&%%%%#   /(  *%%%%%%%%&@@*,,,,,,,,,  ..   .,,,,,,&@@(((((((((,  (*   /((((&@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@&%%%%#     *%%%%%%%%%@@#,,,,,,,,,     .,,,,,,,*@@&(((((((((,     /((((%@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@&%%%%%#.../%%%%%%%%%&@@*,,,,,,,,   .,,,,,,,,,%@@((((((((((*.../(((((&@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@&%%%%%&&%%%%%%%%%%%@@#,,,,,,,,,,,,,,,,,,,,*@@#((((((((((((((((((#@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@@&%.      .%%%%%%%%&@&*,,,,,         ,,,,,(@&((((((((/       ./%@@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@%   .(##/.   #%%%%%%&@%,,.    .,,,.    .,,@@(((((((,    *//,    %@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@(  ,@&%%%%%%.  ,%%%%%%@@*   .,,,,,,,,,.   (@%((((((.  .(((((,     (@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@&  .@@@@%%%%%&   #%%%%%&@,  ,,,,,,,,,,,,,  ,@((((((/           *@. .@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@&  .@@@@@&%%%&.  #%%%%%%&   ,,,,,,,,,,,,,   %((((((/      ./&@@@@.  @@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@&  .@@@@@@&%%&.  #%%%%%%%   ,,,,,,,,,,,,,   (((((((/  ,(((&@@@@@@.  @@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@*  (@@@@@@@&(   &%%%%%%%   ,,,,,,,,,,,,,   ((((((((   /#@@@@@@@#  *@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@,   &@@@@&   ,&%%%%%%%%   ,,,,,,,,,,,,,   (((((((((    &@@@@&   ,@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@@(  %@@@@(  (%%%%%%%%%%   ,,,,,,,,,,,,*.  (((((((((#,  (@@@@%  (@@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@@@(  %@@@@(  #@&%%%%%%%%       ,,,,,       ((((((((&@*  (@@@@%  (@@@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@@@@@&,   %@@@@(   ,%@&%%%%%%%%/    ,,,,,    *(((((((((@%.   (@@@@%   ,&@@@@@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@@@@/       &@@@@@@@       *&(.         ,,,         ./(,       &@%  &@&       /@@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@@@.     (@(   /&&&   (@(       ,#%%&        .@((/.       (@   &     .&  .@/     .@@@@@@@@@@@@"));
  Serial.println(F("@@@@@@.   %@@@@@@@(        *@@@&   /&%%%%%%&*,,,,,,,%#((((((%(   &@      &%     .@@@@@%   *@@@@@@@@"));
  Serial.println(F("@@@@@(  *@@@@@@@@@@@@@@@@@@@@@@.  @@@&%%%%%%&,,,,,,*%((((((@@@@  .@@@@@@&&&@@@@@@@@@@@@@*  &@@@@@@@"));
  Serial.println(F("@@@@@/  /@@@@@@@@@@@@@@@@@@@@@@. .@@@@@&%%%%&*,,,,,((((((#@@@@@. .@@@@@@#  %@@@@@@@@@@@@/  &@@@@@@@"));
  Serial.println(F("@@@@@/  /@@@@@@@&&&&&&@@@@@@@@@. .@@@@@@&%%%%%,,,,,#((((&@@@@@@. .@@@@@@#  %@@@@@@@@@@@@/  &@@@@@@@"));
  Serial.println(F("@@@@@/  /@@@@@@@.    *@@@@@@@@@. .@@@@@@@@%%%&/,,,/((((@@@@@@@@. .@@@@@@#  %@@@@@@@@@@@@/  &@@@@@@@"));
  Serial.println(F("@@@@@/  /@@%%%@@@@@@@@@@@@@@@@@. .@@%%%&@@@&%%%,,,#((%@@@&%%&@@. .@@@@@@#  %@@@@@@@%%%@@/  &@@@@@@@"));
  Serial.println(F("@@@@@/  /@@  *@@@@@@@@@@@@@@@@@. .@@,  %@@@@&%%/,*((@@@@@&  /@@. .@@@@@@#  %@@@@@@@   @@/  &@@@@@@@"));
}
