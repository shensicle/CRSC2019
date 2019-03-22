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

#include <CRSCConfig.h>
#include <EEPROM.h>
#include <Arduino.h>

// ----------------------------------------------------------------------
// Return the one's complement checksum of the configuration structure. This
// checksum is stored in EEPROM along with the configuration itself. The one's complement
// stops zeroed-out EEPROM from being taken as valid.
unsigned char CRSCConfigClass::CalculateChecksum (void)
{
	unsigned char* configurationBytes = (unsigned char*)&TheConfiguration;
	
	unsigned char returnValue = 0;
	
	for (int i = 0; i < sizeof(TheConfiguration); i++)
	{
		returnValue += *configurationBytes++;
	}
	
	return (0xff - returnValue);
}

// ----------------------------------------------------------------------
// This method, which flips the nibbles in the passed-in char and
// returns to result, is used to calculate check bytes for the board ID
char CRSCConfigClass::FlipNibbles (char theByte)
{
    char temp = theByte;
    char returnValue = theByte;
	
    temp = temp << 4; returnValue = returnValue >> 4;
	
    returnValue = (returnValue & 0x0f) + temp;    // Deal with sign extension in return value shift
    return (returnValue);
}
		
// ----------------------------------------------------------------------
// Calculate a fingerprint based on the board ID passed in
unsigned long CRSCConfigClass::CalculateFingerprint (char* theID)
{   
    unsigned long thePrint = 0;
 	
    for (int i = 0; i < BOARD_ID_BYTES; i++)
    {
        thePrint = thePrint << 1;
        if (*(theID+i) & 0x01)
        {   
            thePrint |= 1;
        }
    }
    	
    return (thePrint);
}

// ----------------------------------------------------------------------
// Constructor - allocate EEPROM space
CRSCConfigClass::CRSCConfigClass (void)
{
    // Clear the configuration structure. Not strictly necessary as it's
    // done in Initialize(), but just in case someone changes the code later ...
    memset (&TheConfiguration, 0, sizeof(TheConfiguration));

    
    // Add 1 for the checksum
    EEPROM.begin (sizeof(TheConfiguration)+1);
    
    WifiTestModeActive = false;

}
		

// ----------------------------------------------------------------------
// Write configuration information to EEPROM, adding a checksum
void CRSCConfigClass::Write (void)
{
    unsigned writeAddr = 0;
	
    unsigned char checksum = CalculateChecksum ();
	
    // Write the data
    EEPROM.put (writeAddr, TheConfiguration);
	
    // Write the checksum
    writeAddr += sizeof (TheConfiguration);
    EEPROM.put (writeAddr, checksum);
	
    EEPROM.commit();
}
		

// ----------------------------------------------------------------------
// Read configuration information from EEPROM and validate the checksum
// Returns true if configuration is valid and false otherwise
bool CRSCConfigClass::Read(void)
{
    bool returnValue = true;
    unsigned readAddr = 0;

    // Zero out configuration structure. Helps to null-terminate strings
    memset (&TheConfiguration, 0, sizeof (TheConfiguration));
	
    // Read the data
    EEPROM.get (readAddr, TheConfiguration);
	
    // Calculate the checksum of this data
    unsigned char checksum  = CalculateChecksum ();
	
    // Read the stored checksum
    readAddr += sizeof (config_t);
    unsigned char storedChecksum;
    EEPROM.get (readAddr, storedChecksum);

    if (checksum != storedChecksum)
        returnValue = false;
	
    return (returnValue);
}

// ----------------------------------------------------------------------
// Load the configuration from EEPROM. This must be called after the object is
// created but before any of the other methods can be used. Returns 0 on success and -1
// if something goes wrong.
bool CRSCConfigClass::Load (void)
{
    // Read our configuration from EEPROM
    bool returnValue = Read();
	
    // If all is okay, calculate our fingerprint
    if (returnValue == true)
        Fingerprint = CalculateFingerprint(TheConfiguration.MyBoardID);
	
    return (returnValue);
}

// ----------------------------------------------------------------------
// Return a string containing the specified scavenged board ID, including
// the check digit. Valid indices go from 0 to GetNumScavengedBoardIDs() - 1.
// Return value is true if theIndex is valid and false otherwise
void CRSCConfigClass::PrintScavengedBoardList(void)
{
    Serial.print (F("You have ")); Serial.print ((int)TheConfiguration.NumScavengedBoards);
    Serial.println (F(" scavenged ID(s)\n"));
	
    for (int i = 0; i < TheConfiguration.NumScavengedBoards; i++)
    {
        Serial.println (TheConfiguration.ScavengedBoardList[i]);
    }
    Serial.println();
}
		
// ----------------------------------------------------------------------
// Save new scavenged board ID. Return true on success, false on error. A false would
// be returned if:
//    - scavenged ID list is already full
//    - the specified ID is already on the list
//    - the scavenged ID is not valid (check bytes failure or wrong length
//    - the scavenged ID is actually this board's
//    - the scavenged ID does not match the fingerprint of this board

bool CRSCConfigClass::AddNewScavengedID(char* theID)
{
    bool returnValue = false;

    // If the list is full ...
    if (TheConfiguration.NumScavengedBoards < SCAVENGED_BOARD_LIST_LEN)
    {
        // List not full. Is this a valid board ID?
        if (IsValidBoardID (theID) == true)
        {
            // For the more creative among us, make sure it's not our Board ID
            if (memcmp(&TheConfiguration.MyBoardID, theID, BOARD_ID_BUF_LEN) != 0)
	    	{
	    	    // Valid board ID. Does it match our fingerprint?
	    	    if (HasSameFingerprint(theID))
	    	    {
	    	        // Matches our fingerprint. Make sure it's not already on our list
	    	        returnValue = true;
	    	        int i = 0;
	    	        while ((i < (int)TheConfiguration.NumScavengedBoards) && (returnValue == true))
	    	        {
	    	            if (strcmp(TheConfiguration.ScavengedBoardList[i], theID) == 0)
	    	                returnValue = false;
	    	            else
	    	                i++;
	    	        }
	    	    }
	    	}
	    }
	}
	
	// If we get here and returnValue is true, the board ID is new and valid, so add it
	if (returnValue == true)
	{
	    // Save new ID
	    memcpy (TheConfiguration.ScavengedBoardList[TheConfiguration.NumScavengedBoards], theID, BOARD_ID_BUF_LEN);
	    TheConfiguration.NumScavengedBoards++;
	
	    // And update the configuration in EEPROM
	    Write();
	}
	return (returnValue);
}

// ------------------------------------------------------------------------------
// Calculate the check bytes of a board ID
void CRSCConfigClass::CalculateCheckBytes (char* theID, char* checkBytes)
{

    // String representation of check bytes
    char checkString[BOARD_ID_CHECK_BYTES+1];   // +1 for null terminator
	
    // Storage for the sum of the mangled ID bytes
    unsigned short mangledSum = 0;
	
    // I just made this algorithm up. First, flip the nibbles in each of the ID bytes
    // and add the bytes up
    for (int i = 0; i < BOARD_ID_BYTES; i++)
    {
        mangledSum += FlipNibbles(theID[i]);
    }
	
    // Then clamp the result to be between [0 .. 99]
    mangledSum = mangledSum % 100;
	
    // Convert to a string
    sprintf (checkString, "%02d", mangledSum);
	
    // And save the result
    memcpy (checkBytes, checkString, BOARD_ID_CHECK_BYTES);
}
 
// ------------------------------------------------------------------------------
// Returns a value which, when set, indicates that the passed in string
// contains a valid board ID, including the check digits
bool CRSCConfigClass::IsValidBoardID(char* theID)
{
    char checkBytes[BOARD_ID_CHECK_BYTES];
	
    bool returnValue = false;

    // If the ID is the correct length ...
    if (strlen(theID) == (BOARD_ID_LEN))
    {
        CalculateCheckBytes(theID, checkBytes);
		
        if (memcmp (checkBytes, theID + BOARD_ID_BYTES, BOARD_ID_CHECK_BYTES) == 0)
        {
            returnValue = true;
        }
    }
	
    return (returnValue);	
}
	    
// ------------------------------------------------------------------------------
// Returns a value which, when set, indicates that the board ID passed in results in 
// the same flash code as the ID of this board. Returns false if passed in ID
// does not match the flash pattern of this board or is invalid.
bool CRSCConfigClass::HasSameFingerprint (char* idString)
{
	
    bool returnValue = false;
	
    unsigned long newFingerprint = 0;
	
    // First, make sure the ID passed in is valid
    if (IsValidBoardID(idString))
    {
        for (int i = 0; i < BOARD_ID_BYTES; i++)
        {
            newFingerprint = newFingerprint << 1;

            if (*(idString+i) & 0x01)
                newFingerprint |= 1;
    		
        }
        if (newFingerprint == Fingerprint)
            returnValue = true;
    }
	
    return (returnValue);
}
    
// ------------------------------------------------------------------------------
// Clears EEPROM and writes the values provided. Intended to be used by
// the initialization sketch to configure boards. The BoardID is not provided
// in this call as it has to be entered by the user after the sketch is done
// it's setup.
void CRSCConfigClass::Initialize (char* theWifiSSID, char* theWifiPassword, char* theIFTTTKey)
{
    // Clear the entire configuration structure
    memset (&TheConfiguration, 0, sizeof(TheConfiguration));
    
    // Load the parameters
    strncpy (TheConfiguration.WifiSSID, theWifiSSID, WIFI_SSID_LEN);
    strncpy (TheConfiguration.WifiPassword, theWifiPassword, WIFI_PASSWORD_LEN);
    strncpy (TheConfiguration.IFTTTKey, theIFTTTKey, IFTTT_KEY_LEN);
	
    // BoardID, scavenged board information and ScavengeComplete were zeroed by the memset()
    // at the top of this method.
    
    // And save
    Write();
}

// ------------------------------------------------------------------------------
// Set the ID of this board. Intended to be called only from the configuration sketch
// when board is being configured.
bool CRSCConfigClass::SetBoardID(char* newID)
{
    bool returnValue = false;
	
    if (IsValidBoardID (newID))
    {
        strncpy (TheConfiguration.MyBoardID, newID, BOARD_ID_BUF_LEN);
        
        // If we're setting the board ID, should erase any existing scavenged IDs,
        // I think.
        TheConfiguration.NumScavengedBoards = 0;
        
        // Save changes to EEPROM
        Write();
        returnValue = true;
    }
    return (returnValue);
}

// ------------------------------------------------------------------------------
// Return the current fingerprint in the string provided - used for diagnostics only.
// String returned consists of 4 characters that are either 0 or 1, plus the
// terminator.
void CRSCConfigClass::GetFingerprint (char* thePrint)
{
    unsigned long temp = Fingerprint;
    
    for (int i = 0; i < 4; i++)
    {
        if (temp & 0x01)
            thePrint[i] = '1';
        else
            thePrint[i] = '0';
       
        temp = temp >> 1;
    }
    thePrint[4] = 0x00;
}
		
