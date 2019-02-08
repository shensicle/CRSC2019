#ifndef _CMD_PARSER_H
#define _CMD_PARSER_H

/*
Copyright � 2011 Scott Henwood/shensicle photographic. All Rights Reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Scott Henwood/shensicle photographic "AS IS" AND 
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

#include <arduino.h>

class CRSCCmdParser
{
protected:

    //Pointer to the string we're parsing
    String* StringPtr;
	
    //Storage for the current position in the string
    int CurrPos;
	
    //Skip whitespace in the string
    void SkipWhitespace (void);
    
public:

    // Constructor - pass in string
    CRSCCmdParser (String* theString);
	
    // Function to terminate current parsing activities and restart
    void Reset (void);
	
    // Function returning a flag which, when set, indicates that there
    // is more data remaining to parse.
    bool MoreDataAvailable (void);
	
    // Method to return an unsigned long value. Any whitespace prior to the
    // value is skipped. If no suitable value is encountered, a zero is
    // returned.
    unsigned long GetUnsignedLong (void);

    // Get a single non-whitespace character. If there are no such
    // characters, return 0x00
    char GetChar (void);
	
    // Return a string of length up to maxLen after skipping over whitespace. Return 0x00 if there is
    // no string on the command line.
    void GetString (char* theResult, unsigned maxLen);
    
    // Return a string of length up to maxLen after skipping over leading whitespace and stopping at
    // trailing whitespace. Return 0x00 if there is no string on the command line.
    void GetStringToWhitespace (char* theResult, unsigned maxLen);

};


#endif
