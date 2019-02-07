#ifndef _CRSCLED_H
#define _CRSCLED_H
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

#include "CRSCConfigDefs.h"


class CRSCLED
{
protected:
	
    // Structure to store the LED flash sequence associated with this board ID
    // Delta list-like structure to describe led flash codes
    typedef struct 
	{
	    // number of milliseconds we should be in this state
	    int StateMilliseconds;

	    // Number of milliseconds we've been in this state so far
	    int MillisecondsSoFar;

	    // The state of LED for this entry (0= off; 1 = on)
	    unsigned char LEDState;      
	} FlashEntry_t;

	// Array to store all LED states to implement flashing. Each
	// byte in the board ID requires two states (on and off) and there
	// is a single state at the end to provide a longer "off" gap between
	// flash sequences.
	FlashEntry_t FlashList[BOARD_ID_BYTES*2+1];
  	  
	// Index into our FlashList
	int FlashListIndex;
  	  
	// The hardware pin the LED is connected to
	int TheLEDPin;
  	
	float UpdateInterval;  // How often LED is updated, in seconds
	
	unsigned long Fingerprint;  // The fingerprint of our board ID, used to determine flash sequence
	
	// Initialize the flash list using the current fingerprint
	void InitializeFlashList (void);
	
public:
	
    CRSCLED (int theLEDPin);
	
    // Set the fingerprint value
    void SetFingerprint (unsigned long newPrint);
	
    // Update the LED. Should be called every UpdateInterval.
    void Update (void);
};

#endif
