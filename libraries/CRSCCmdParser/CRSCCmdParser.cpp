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
        if ((StringPtr->charAt (CurrPos) == ' ') ||
            (StringPtr->charAt (CurrPos) == '\t'))
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
// Method to return a command. Commands are in the form of a 
// hyphen followed by a single letter or number. Whitespace before
// the hyphen is skipped and the single letter or number is returned.
// If a command cannot be found, the null character (0x00) is returned.
char CRSCCmdParser::GetCommand (void)
{       
    char returnValue = 0x00;
    bool done = false;
	
    SkipWhitespace ();
	
    while ((CurrPos < StringPtr->length()) && (done == false))
    {
        // If we have a hyphen ...
        if (StringPtr->charAt(CurrPos) == '-')
        {
            CurrPos ++;
            if (CurrPos < StringPtr->length())
            {
                returnValue = StringPtr->charAt (CurrPos++);
            }
            done = true;
        }
        else
        {
            done = true;  // didn't see a '-' when we expected one
        }
    }
	
    return (returnValue);
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
        StringPtr->substring(CurrPos, StringPtr->length()).toCharArray(theResult, StringPtr->length()-CurrPos);	
    }
}

