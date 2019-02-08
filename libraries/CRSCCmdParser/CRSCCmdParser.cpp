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

#include <CRSCCmdParser.h>


// --------------------------------------------------------------
//Skip whitespace in the string
void CRSCCmdParser::SkipWhitespace (void)
{
    bool done = false;
    while ((CurrPos < StringPtr->length()) && (done == false))
    {
        // You would think from the Arduino docs that isWhitespace()
        // would be the correct call here, but it isn't. It only detects
        // spaces and tabs.
        if (isSpace(StringPtr->charAt (CurrPos)))
        {
            CurrPos ++;
        }
        else
        {
            done = true;
        }
    }
}
	
	
// --------------------------------------------------------------
// Constructor - pass in string
CRSCCmdParser::CRSCCmdParser (String* theString)
{
    StringPtr = theString;
    Reset();
}

// --------------------------------------------------------------
// Function to terminate current parsing activities and restart
void CRSCCmdParser::Reset (void)
{
    CurrPos = 0;
}
	
// --------------------------------------------------------------
// Function returning a flag which, when set, indicates that there
// is more data remaining to parse.
bool CRSCCmdParser::MoreDataAvailable (void)
{
    if (CurrPos < StringPtr->length())
        return (true);
    else
        return (false);
}

// --------------------------------------------------------------
// Method to return an unsigned long value. Any whitespace prior to the
// value is skipped. If no suitable value is encountered, a zero is
// returned.
unsigned long CRSCCmdParser::GetUnsignedLong (void)
{
    unsigned long returnValue = 0;
	
    bool haveDigit = true;
	
    SkipWhitespace ();
	
    while ((CurrPos < StringPtr->length()) && (haveDigit == true))
    {
        // If we have a number
        char nextChar = StringPtr->charAt(CurrPos++);
        if ((nextChar >= '0') && (nextChar <= '9'))
        {
            returnValue = returnValue * 10 + (nextChar - '0');
        }
        else
        {
            haveDigit = false;
        }
    }
    return (returnValue);
}

// --------------------------------------------------------------
// Get a single non-whitespace character. If there are no such
// characters, return 0x00
char CRSCCmdParser::GetChar (void)
{
    char returnValue = 0x00;
	
    SkipWhitespace();

    if (CurrPos < StringPtr->length())
        returnValue = StringPtr->charAt(CurrPos++);

    return (returnValue);
}

// --------------------------------------------------------------
// Return a string of length up to maxLen after skipping over whitespace. Return 0x00 if there is
// no string on the command line.
void CRSCCmdParser::GetString (char* theResult, unsigned maxLen)
{
    SkipWhitespace ();

    if (StringPtr->length() - CurrPos > maxLen)
    {
        theResult[0] = 0x00;
    }
    else
    {
        // Use c_str instead of substring() to reduce heap fragmentation caused by 
        // creating temporary string objects all over the place. MaxLen - 1 because
        // strings we receive from the serial interface always have a line feed at the end.
        strncpy (theResult, StringPtr->c_str()+CurrPos, maxLen-1);    
    }
}

// --------------------------------------------------------------
// Return a string of length up to maxLen after skipping over leading whitespace and stopping at
// trailing whitespace. Return 0x00 if there is
// no string on the command line.
void CRSCCmdParser::GetStringToWhitespace (char* theResult, unsigned maxLen)
{
    SkipWhitespace ();
    
    bool done = false;
    int i = 0;
        
    while ((CurrPos < StringPtr->length()) && (i < maxLen) && (done == false))
    {
        if (isSpace(StringPtr->charAt(CurrPos)))
        {
             // We're done, terminate the result string and leave
             theResult[i] = 0x00;
             done = true;
        }
        else
        {
             // Copy this character into our result string and move to next
             theResult[i++] = StringPtr->charAt(CurrPos++);
         }
     }      
}

