#ifndef _CRSCSERIALINTERFACE_H
#define _CRSCSERIALINTERFACE_H

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


#include <String.h>
#include "CRSCCmdParser.h"
#include "CRSCConfig.h"

class CRSCSerialInterface
{
protected:
    // String to store received characters
    String InputString;
	
    // Parser object
    CRSCCmdParser Parser;
	
    // A flag which, when set, indicates that we have received a complete
    // command - ie. last character was a line feed
    bool CommandComplete;
	
    // Pointer to the configuration object
    CRSCConfigClass* TheConfiguration;
    
    // Handlers for some of the longer commands - to keep Update() readable
    void ProcessACommand(void);
    void ProcessDCommand(void);
    void ProcessICommand(void);
    void ProcessRCommand(void);
    
public:
    // Constructor
    CRSCSerialInterface (CRSCConfigClass* theConfiguration);
	
    // Add a character to the command currently being built up
    void Add (char inChar);
	
    // If we have a complete command, parse and act on it
    void Update (void);
    
    // Display our help text. Public so it can be called directly at startup
    // or in response the the terminal interface 'H' command.
    void DisplayHelp (void); 
};

#endif