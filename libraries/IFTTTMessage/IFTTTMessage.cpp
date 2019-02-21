/*
Copyright 2019 Scott Henwood All Rights Reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Scott Henwood "AS IS" AND 
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

#include "IFTTTMessage.h"

#define IFTTT_URL "maker.ifttt.com"

// -----------------------------------------------------
IFTTTMessageClass::IFTTTMessageClass (int updateInterval)

{
    // Reserve buffers for our strings
    PostData.reserve(120);
    PostString.reserve(200);
    DeviceID.reserve(10);
    
    UpdateInterval = updateInterval;
    MillisecondsToRetry = updateInterval;
}

// -----------------------------------------------------
// Initialize - pass in API key for IFTTT and a tag to use in the JSON packet,
// which is typically a unique identifier for this host. This can't be done in
// constructor as we have to wait for personality to be read from EEPROM. Call
// this method once before calling Send()
void IFTTTMessageClass::Initialize (const char* theAPIKey, const char* deviceID, const char* deviceType)

{
    DeviceID = deviceID;
    PostData   = "";
    
    PostString =  "POST /trigger/crsc_message/with/key/";
    PostString += theAPIKey;
    PostString += " HTTP/1.1\nHost: ";
    PostString += IFTTT_URL;
    PostString += "\nUser-Agent: ";
    PostString += deviceType;
    PostString += "\nConnection: close\nContent-Type: application/json\nContent-Length: ";
}

// -----------------------------------------------------
// Connect to the ifttt service.
bool IFTTTMessageClass::Connect (void)
{
   // Value which, when set, indicates connection to IFTTT server was successful
   bool returnValue = true;
   
   if(TheClient.connect(IFTTT_URL,80))  // Test the connection to the server
   {
     Serial.print("Connected to "); Serial.println(IFTTT_URL);
   }
   else
   {
     Serial.println("Failed to connect to "); Serial.println(IFTTT_URL);
     returnValue = false;
   }
   
   return (returnValue);
}

// -----------------------------------------------------
// Send a message. Return value indicates whether or not message was successfully sent
bool IFTTTMessageClass::Send (String theMessage)
{
 
    if (Connect())
    {
    	// Note that ifttt only supports labels value1, value2, value3
  	    PostData = "{\"value1\":\"";
   	    PostData.concat (DeviceID);
  	    PostData.concat ("\",\"value2\":\"");
  	    PostData.concat(theMessage);
  	    PostData.concat("\"}");

  	    TheClient.print (PostString);          // Connection details
	    TheClient.println(PostData.length());  // length of JSON payload
        TheClient.println();
        TheClient.println(PostData);           // JSON payload
    }
}

// -----------------------------------------------------
// Attempt to send a message to IFTTT and return a flag which, when set, indicates success.
// When called repeatedly, this method implements retries with a 10 second delay until
// the message has been sent successfully.
bool IFTTTMessageClass::SendMessage (char* theMessage)
{
  bool returnValue = false;

  MillisecondsToRetry -= UpdateInterval;

  if (MillisecondsToRetry <= 0)
  {
    returnValue = Send (theMessage);

    // If this was not successful ...
    if (returnValue == false)
    {
      MillisecondsToRetry = RetryInterval;
      Serial.println (F("\nConnection to ifttt.com failed. Will try again in 10 seconds."));
      Serial.println (F("In the mean time, please notify one of the CANARIE staff that you have completed the scavenger hunt\n\n"));
    }
    else
    {
        // Get ready for the next time we are called (ideally with a new message)
        MillisecondsToRetry = UpdateInterval;
    }
  }  
  return (returnValue);

}
   
