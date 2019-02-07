#ifndef _BOARDID_H
#define _BOARDID_H

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

// Number of bytes in a boardID. 4 bytes of data and two check bytes.
#define BOARD_ID_BYTES 4
#define BOARD_ID_CHECK_BYTES 2

// Structure to store a board ID
typedef boardID_t char[BOARD_ID_BYTES];


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


// Implements validation and flash code for BoardID
class CRSCBoardID
{
  private:
  	  const int LEDShortPulse = 250;    // milliseconds
  	  const int LEDLongPulse = 500;     // milliseconds
  	  const int LEDOffPulse = 500;      // milliseconds - off time betweeen on pulses
  	  const int LEDGapPulse = 1000;     // milliseconds - off time between sequences

  protected:
    // Array to store all LED states to implement flashing. Each
    // byte in the board ID requires two states (on and off) and there
    // is a single state at the end to provide a longer "off" gap between
    // flash sequences.
 	FlashEntry_t FlashList[BOARD_ID_BYTES*2+1];
  	  
   // Storage for board ID
   String IDString;
  	  
   // Index into our FlashList
   int FlashListIndex;
  	  
   // A fingerprint for this ID. All IDs with the same flash sequence have the
   // same fingerpring
   unsigned long Fingerprint;
  	  
   // Calculate the check bytes of a board ID
   void CalculateCheckBytes (CRSCBoardID theID, char* checkBytes);
  	    	      
   // Build up the list of LED flash sequences
   void BuildFlashSequence(void);
	
  public:
  	  
    // Constructor 
    CRSCBoardID (void);
  	  
    // Set the ID of the of this board. Return true if board ID is valid and false otherwise.
    bool Set(char* boardIDString);
    
    // Return a pointer to the array of flash codes for this board. Returns null if board ID
    // hasn't been set yet.
    FlashEntry_t* GetNextFlashEntry(void);
    
};

#endif
