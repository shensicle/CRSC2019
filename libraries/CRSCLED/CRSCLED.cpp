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


#include <Arduino.h>
#include "CRSCLED.h"


const int LEDShortPulse = 100;      // milliseconds
const int LEDLongPulse  = 500;      // milliseconds
const int LEDOffPulse   = 500;      // milliseconds - off time betweeen on pulses
const int LEDGapPulse   = 1500;     // milliseconds - off time between sequences


	
// -----------------------------------------------------------------------------
// Initialize the flash list using the current fingerprint
void CRSCLED::InitializeFlashList (void)
{
	unsigned long thePrint = Fingerprint;
	
  // Going up by 2 here because each character in the ID string corresponds to 
  // the LED being on for an amount of time and off for an amount of time
  for (int i = 0; i < (BOARD_ID_BYTES*2); i+= 2)
  {
  	 // This is the on part. A 1 results in a long pulse and a 0 results in
  	 // a short pulse.
     if (thePrint & 0x01)
     {
          FlashList[i].StateMilliseconds = LEDLongPulse;
     }
     else
     {
          FlashList[i].StateMilliseconds = LEDShortPulse;
     }
 
     FlashList[i].MillisecondsSoFar = 0;
     FlashList[i].LEDState = 0;           // 0 is on

     FlashList[i+1].StateMilliseconds = LEDOffPulse;
     FlashList[i+1].MillisecondsSoFar = 0;
     FlashList[i+1].LEDState = 1;         // 1 is off
     
     thePrint = thePrint >> 1;
  }

  // Last one - the gap between flash sequences
  FlashList[BOARD_ID_BYTES*2].StateMilliseconds = LEDGapPulse;
  FlashList[BOARD_ID_BYTES*2].MillisecondsSoFar = 0;
  FlashList[BOARD_ID_BYTES*2].LEDState = 1;  // 1 is off
    
  // And reset our flash list index to the last element. This causes the
  // flash to start with a gap.
  FlashListIndex = (BOARD_ID_BYTES*2);
}
	
// -----------------------------------------------------------------------------
CRSCLED::CRSCLED(int theLEDPin, float updateInterval)
{ 
	Fingerprint = 0x00; 
	UpdateInterval = updateInterval;
	TheLEDPin = theLEDPin;
	
	// Set up control pin for LED and turn it off
    pinMode (TheLEDPin, OUTPUT);
    SetOff();

	InitializeFlashList();
}
	
// -----------------------------------------------------------------------------
// Set the fingerprint value
void CRSCLED::SetFingerprint (unsigned long newPrint)
{ 
	Fingerprint = newPrint; 
	InitializeFlashList();

}
	
// -----------------------------------------------------------------------------
// Update the LED. Should be called every UpdateInterval.
void CRSCLED::Update (void)
{
    FlashList[FlashListIndex].MillisecondsSoFar += UpdateInterval;

    // If we've been in this state for the appropriate amount of time
    if (FlashList[FlashListIndex].MillisecondsSoFar >= FlashList[FlashListIndex].StateMilliseconds)
    {
        // Reset it for next time
        FlashList[FlashListIndex].MillisecondsSoFar = 0;

        // And move to the next state
        FlashListIndex ++;
        if (FlashListIndex > BOARD_ID_BYTES*2)
            FlashListIndex = 0;

        // Set the LED accordingly
        digitalWrite (TheLEDPin, FlashList[FlashListIndex].LEDState);  
    }     
}

	