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

#include "CRSCSerialInterface.h"


// Size of buffer for incoming serial characters
#define BUF_SIZE 80

// The code to use to enable host configuration commands - minimal security
const char SecurityCode[] = "XNY556";

	
// --------------------------------------------------------------------------- 
// Constructor
CRSCSerialInterface::CRSCSerialInterface (CRSCConfigClass* theConfiguration)
	: Parser(&InputString) 
{ 
    InputString = ""; 
    TheConfiguration = theConfiguration;

    CommandComplete = false; 
    // Reserve space for incoming commands to minimize heap fragmentation.
    InputString.reserve(BUF_SIZE);
}
	
// --------------------------------------------------------------------------- 
// Add a character to the command currently being built up
void CRSCSerialInterface::Add (char inChar)
{
    InputString += inChar;
	
    if (inChar == '\n')
        CommandComplete = true;
}
	
// ---------------------------------------------------------------------------
// Display our help text
void CRSCSerialInterface::DisplayHelp (void)
{
    Serial.println(F("Available commands:\n"));
    Serial.println(F("H - Help - display this message"));
    Serial.println(F("A <board ID> - Add a new board ID to your scavenged list"));
    Serial.println(F("G - Get - Display the ID of this board"));
    Serial.println(F("L - List - Display the current list of scavenged board IDs\n"));
}

// --------------------------------------------------------------------------- 
// If we have a complete command, parse and act on it
void CRSCSerialInterface::Update (void)
{
    char newID[BOARD_ID_BUF_LEN];
	
    bool okay = true;
	
    // A flag which, when set, indicatest that a board ID passed into the A command
    // was accepted. Compiler doesn't like it when you create local variables inside a
    // switch statement, so here it is.
    bool newIDOkay;
	
    if (CommandComplete == true)
    {
        Parser.Reset();
        		
        char command = Parser.GetChar();
        command = toupper(command);
		    
        switch (command)
        {
            // Run
            case 'H':
                if (Parser.IsMoreCommandLine())
                    Serial.println (F("Warning: Unexepected command line characters encountered.\n"));
                DisplayHelp();
                break;
        
       
            // Add a Board ID
            case 'A':        
                ProcessACommand();
                break;  
        
            // Dump the list of scavenged board IDs
            case 'L':        
                if (Parser.IsMoreCommandLine())
                    Serial.println (F("Warning: Unexepected command line characters encountered. Type 'H' for help.\n"));
                
                // Print scavenged ID list
                TheConfiguration->PrintScavengedBoardList();

                break;  
        
        
            // Display our own board ID
            case 'G':
                if (Parser.IsMoreCommandLine())
                {
                    Serial.println (F("Warning: Unexepected command line characters encountered. Type 'H' for help\n"));
                    DisplayHelp();
                }
                Serial.print (F("Your board ID is ")); Serial.print(TheConfiguration->GetBoardID()); Serial.println(F(" \n"));
                break;
        
            // *** These commands are for production and not announced to users ***    
                
            // Dump the board's current configuration. Not documented in online help as it's intended to be used only by CANARIE staff.
            // Requires security code.
            case 'D':
                
               ProcessDCommand();
                
               break;
            
            
            // Set our board ID. This can only be done when the Board ID in EEPROM has been cleared - ie. 000000. Meant to be used by CANARIE
            // staff only so doesn't appear in help.
            case 'I':
                
                ProcessICommand();   // Warning - this command reboots host
                
                break;
      					
            // Reset EEPROM - Intended to be used by CANARIE during production/testing and so does not appear in help. You would typically
            // use this command followed by the I command to reload the board ID
            case 'R':
      				
                ProcessRCommand();  // Warning - this command reboots host
                break;
                
            // This is for production purposes only. If you type the "W" command and the security code,
            // a message will be sent to ifttt.com. This is used to verify connectivity and the Wifi hardware
            case 'W':
                
                // Not really getting a board ID, but buffer was there so let's reuse it.
                Parser.GetStringToWhitespace(newID, BOARD_ID_BUF_LEN);
                      				 
                if (strcmp(newID, SecurityCode) == 0)
                {
                    TheConfiguration->RequestWifiTest();
                }
                else
                {
                    Serial.println (F("\nWifi test cancelled - invalid security code\n"));
                }
                break;
            
            case 0x00:

                // Just a new line. Let it go and don't bother user with invalid command error message.
                break;
      					 
            default:
                Serial.println (F("Invalid command\n"));
                break;

        }
    
        CommandComplete = false;
        InputString = ""; 
        
    }  
}

// -----------------------------------------------------------------------------
void CRSCSerialInterface::ProcessACommand (void)
{          
    char newID[BOARD_ID_BUF_LEN];

    Parser.GetStringToWhitespace(newID, BOARD_ID_BUF_LEN);
                
    if (Parser.IsMoreCommandLine())
       Serial.println (F("Warning: Unexepected command line characters encountered.\n"));

		    			
    // Figure out where the next avaiable space is and add this, along with check digit
    bool newIDOkay = TheConfiguration->AddNewScavengedID(newID);
    if (newIDOkay)
    {
        Serial.print (F("Addition successful - you now have "));
        Serial.print (TheConfiguration->GetNumScavengedBoardIDs());
        Serial.println (F(" scavenged ID(s)\n"));
    }
    else
    {
        Serial.println(F("Oh no!! Scavenged board ID could not be added"));
        Serial.println(F("This could be because:"));
        Serial.println(F(" - It's not a valid board ID - there are check bytes :)"));
        Serial.println(F(" - It's from a board that doesn't match your flash code"));
        Serial.println(F(" - It's the ID of your board"));
        Serial.println(F(" - This board has already been added to your scavenged list\n"));
    }
 
}

// -----------------------------------------------------------------------------
void CRSCSerialInterface::ProcessDCommand (void) 
{
          
    char buf[BOARD_ID_BUF_LEN];

    Parser.GetStringToWhitespace(buf, BOARD_ID_BUF_LEN);
    
    if (strcmp(buf, SecurityCode) == 0)
    {
        Serial.println (F("\n\nDump configuration:\n"));
        Serial.print (F("Wifi SSID: ")); Serial.println (TheConfiguration->GetWifiSSID());                    
        Serial.print (F("Wifi Password: ")); Serial.println (TheConfiguration->GetWifiPassword());
        Serial.print (F("IFTTT Key: ")); Serial.println (TheConfiguration->GetIFTTTKey());
        Serial.print (F("Board ID: ")); Serial.println (TheConfiguration->GetBoardID());
        Serial.print (F("Scavenged boards: ")); Serial.println (TheConfiguration->GetNumScavengedBoardIDs());
                    
        Serial.print (F("Fingerprint: ")); 

        TheConfiguration->GetFingerprint(buf);
        Serial.println (buf);
                   
        Serial.print (F("\n\n"));    
    }
    else
    {
        Serial.println (F("Command cancelled - invalid security code\n"));
    }
}

// -----------------------------------------------------------------------------
void CRSCSerialInterface::ProcessICommand (void) 
{
    char buf[BOARD_ID_BUF_LEN];

    if (memcmp (TheConfiguration->GetBoardID(), UninitializedID, BOARD_ID_LEN) == 0)
    {
        Parser.GetStringToWhitespace(buf, BOARD_ID_BUF_LEN);
        bool okay = TheConfiguration->SetBoardID(buf);
                    
        if (okay)
        {
             Serial.print(F("\nYour board ID is now ")); Serial.print(TheConfiguration->GetBoardID()); Serial.println(F("\n"));
             Serial.println (F("Rebooting...There's a bug where reboots fail first time after flashing board"));
             Serial.println (F("If board doesn't reboot, push reset button\n"));
             ESP.restart();
        }
        else
        {
             Serial.println(F("\nCommand cancelled - invalid board ID\n"));
        }
    }
    else
    {
        Serial.println(F("\nCommand cancelled - board ID already set\n"));
    }
}


// -----------------------------------------------------------------------------
void CRSCSerialInterface::ProcessRCommand (void) 
{
    
    char buf[BOARD_ID_BUF_LEN];

    Parser.GetStringToWhitespace(buf, BOARD_ID_BUF_LEN);
                      				 
    if (strcmp(buf, SecurityCode) == 0)
    {
         Serial.println (F("Resetting EEPROM"));
         TheConfiguration->Initialize(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD, DEFAULT_IFTTT_KEY);
                    
         // Now, if there's a board ID on the command line as well, set it to be our board ID
         Parser.GetString (buf, BOARD_ID_BUF_LEN);
         if (TheConfiguration->SetBoardID(buf))
         {
              Serial.print (F("\nYour board ID is now ")); Serial.print(TheConfiguration->GetBoardID()); Serial.println(F("\n"));
              Serial.println (F("Rebooting...There's a bug where reboots fail first time after flashing board"));
              Serial.println (F("If board doesn't reboot, push reset button\n"));
              ESP.restart();
         }
         else
         {
              Serial.println (F("\nNo new board ID specified - use the 'I' command to set a new board ID"));
         }

    }
    else
    {
        Serial.println (F("\nEEPROM reset failed - invalid security code"));
    }
   
}
