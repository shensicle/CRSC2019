#ifndef _IFTTT_MESSAGE_H
#define _IFTTT_MESSAGE_H

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


#include <Arduino.h>

#include <WiFiServerSecure.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiType.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFiAP.h>
#include <WiFiClient.h>
#include <BearSSLHelpers.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#include <WiFiServerSecureBearSSL.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiServerSecureAxTLS.h>


class IFTTTMessageClass
{
  private:
    char* APIKey;
    WiFiClient TheClient;
    
    // String used for static parts of message sent to IFTTT
    String PostString;    
    
    // String used to hold the first label of the JSON packet, typically a
    // unique identifier for the sensor host
    String SensorID;
    
    // Connect to the ifttt service. Returns true if connection was successful
    virtual bool Connect (void);



  public:
   // Constructor - doens't do much because we have to wait until configuration
   // is loaded from the personality file before initializing this object
   IFTTTMessageClass (void);

   // Initialize - pass in API key for IFTTT and a tag to use in the JSON packet,
   // which is typically a unique identifier for this host. This can't be done in
   // constructor as we have to wait for personality to be read from EEPROM. Call
   // this method once before calling Send()
   void Initialize (const char* theAPIKey, const char* sensorID);

    // Send a message. Return value indicates whether or not message was successfully sent
    virtual bool Send (String theMessage);
    
};

#endif
