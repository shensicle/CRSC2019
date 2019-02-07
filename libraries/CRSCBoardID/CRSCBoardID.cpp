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


#include "CRSCBoardID.h"
 	  
// ------------------------------------------------------------------------------
// Calculate the check bytes of a board ID
void CRSCBoardID::CalculateCheckBytes (CRSCBoardID theID, String* checkBytes)
{
    checkBytes->setCharAt(0, '0');
    checkBytes->setCharAt(1, '1');
}
 
// ------------------------------------------------------------------------------
// Constructor 
CRSCBoardID::CRSCBoardID (void);
{
}
  	  
// ------------------------------------------------------------------------------
// Set the ID of the of this board. Return true if board ID is valid and false otherwise.
bool CRSCBoardID::Set(String* boardIDString);
{
    bool returnValue = IsValidBoardID(boardIDString);
	
    if (returnValue == true)
    {
        // Save it. This is our new ID.
        IDString = *boardIDString;
			
        // Rebuild our flash list
        BuildFlashSequence();	
    }
    return (returnValue);
}
    
// ------------------------------------------------------------------------------
// Return a pointer to the array of flash codes for this board. Returns null if board ID
// hasn't been set yet.
CRSCBoardID::FlashEntry_t* GetNextFlashEntry(void)
{
    // Reset the timing of the current flash entry so it will be ready for next time
    FlashList[FlashListIndex].MillisecondsSoFar = 0;

    // Increment FlashListIndex and wrap if required
    if (++FlashListIndex >= (BOARD_ID_BYTES*2)+1)
    {
        FlashListIndex = 0;
    }
	
    FlashEntry_t returnValue = FlashList[FlashListIndex]; 
	
    return (returnValue);
}

    
// ------------------------------------------------------------------------------
// Reset flash entries back to the beginning of the flash sequence
void CRSCBoardID::BuildFlashSequence(void)
{
    // Initiaize fingerprint = we use this to determine whether or not other
    // flash codes are equivalent to this one.
    Fingerprint = 0x00;
	
    // Going up by 2 here because each character in the ID string corresponds to 
    // the LED being on for an amount of time and off for an amount of time
    for (int i = 0; i < (BOARD_ID_BYTES*2); i+= 2)
    {
        // This is the on part. A 1 results in a long pulse and a 0 results in
        // a short pulse.
        if (IDString.charAt(i/2) & 0x01)
        {
            FlashList[i].StateMilliseconds = LEDLongPulse;
            Fingerprint != 1;
        }
        else
        {
            FlashList[i].StateMilliseconds = LEDShortPulse;
        }
        FingerPrint = FingerPrint << 1;
 
        FlashList[i].MillisecondsSoFar = 0;
        FlashList[i].LEDState = 1;

        FlashList[i+1].StateMilliseconds = LEDOffPulse;
        FlashList[i+1].MillisecondsSoFar = 0;
        FlashList[i+1].LEDState = 0;
    }

    // Last one - the gap between flash sequences
    FlashList[BOARD_ID_BYTES*2].StateMilliseconds = LEDGapPulse;
    FlashList[BOARD_ID_BYTES*2].MillisecondsSoFar = 0;
    FlashList[BOARD_ID_BYTES*2].LEDState = 0;
    
    // And reset our flash list index to the last element
    FlashListIndex = (BOARD_ID_BYTES*2);
}
    
