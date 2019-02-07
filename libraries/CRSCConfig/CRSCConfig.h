#ifndef _CRSC_CONFIG_H
#define _CRSC_CONFIG_H

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

// The definition of the configuration for the current sketch. 
#include <CRSCConfigDefs.h>

class CRSCConfigClass
{
    private:
        // The buffer that stores our configuration, as layed out in EEPROM
        config_t TheConfiguration;
		
        // Return the one's complement checksum of the configuration structure
        unsigned char CalculateChecksum (void);
		
        // This method, which flips the nibbles in the passed-in char and
        // returns to result, is used to calculate check bytes for the board ID
        char FlipNibbles (char theByte);
		
        // Calculate a fingerprint based in the ID string passed in
        unsigned long CalculateFingerprint (char* theID);
	
    protected:
        // Write configuration information to EEPROM, adding a checksum
        void Write (void);
		
        // Read configuration information from EEPROM and validate the checksum
        // Returns true if configuration is valid and false otherwise
        bool Read(void);
		
        // A fingerprint for the ID of this board. All IDs with the same flash sequence have the
        // same fingerpring
        unsigned long Fingerprint;
  	  
        // Calculate the check bytes of a board ID
        void CalculateCheckBytes (char* theID, char* checkBytes);
  	  
        // Returns a value which, when set, indicates that the passed in string
  	    // contains a valid board ID, including the check digits
  	    bool IsValidBoardID(char* theID);  
	
	
  	public:
 	    
  	    // Constructor - allocate EEPROM space
  	    CRSCConfigClass (void);
				
  	    // Return the current number of scavenged board IDs
  	    int GetNumScavengedBoardIDs(void)
  	       { return ((int)TheConfiguration.NumScavengedBoards); }
		  
  	    // Return our fingerprint
  	    unsigned long GetFingerprint (void)
  	       { return (Fingerprint); }
		
  	    // Return a string containing the specified scavenged board ID, including
  	    // the check digit. Valid indices go from 0 to GetNumScavengedBoardIDs() - 1.
  	    // Return value is true if theIndex is valid and false otherwise
  	    void PrintScavengedBoardList(void);
		
  	    // Save new scavenged board ID. Return 0 on success, -1 on error. A -1 would
  	    // be returned if scavenged ID list is already full or if the specified ID
  	    // is already on the list.
  	    bool AddNewScavengedID(char* theID);
		
  	    // Return a pointer to our stored WifiSSID
  	    char* GetWifiSSID(void)
  	       { return (TheConfiguration.WifiSSID); }
		
  	    // Return a pointer to our stored Wifi password
  	    char* GetWifiPassword(void)
  	       { return (TheConfiguration.WifiPassword); }
		
  	    // Return a pointer to our stored IFTTT key
  	    char* GetIFTTTKey(void)
  	       { return (TheConfiguration.IFTTTKey); }
		
  	    // Return a pointer to our own Board ID
  	    char* GetBoardID(void)
  	       { return (TheConfiguration.MyBoardID); }
		
  	    // Load the configuration from EEPROM. This must be called after the object is
  	    // created but before any of the other methods can be used. Returns true on success and false
  	    // if something goes wrong.
  	    bool Load(void);
		
  	    // Returns a value which, when set, indicates that the board ID passed in results in 
        // the same flash code as the ID of this board. Returns false if passed in ID
        // does not match the flash pattern of this board or is invalid.
        bool HasSameFingerprint(char* idString);
        
        // Clears EEPROM and writes the values provided. Intended to be used by
        // the initialization sketch to configure boards. The BoardID is not provided
        // in this call as it has to be entered by the user after the sketch is done
        // it's setup.
        void Initialize (char* theWifiSSID, char* theWifiPassword, char* theIFTTTKey);
        
        // Set the ID of this board. Intended to be called only from the configuration sketch
        // when board is being configured.
        bool SetBoardID(char* newID);
};



#endif