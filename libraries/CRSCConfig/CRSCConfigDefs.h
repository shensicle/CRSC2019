#ifndef _CRSCCONFIGDEFS_H
#define _CRSCCONFIGDEFS_H

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


// Lengths for fixed fields in the configuration structure
#define WIFI_SSID_LEN     25
#define WIFI_PASSWORD_LEN 25
#define IFTTT_KEY_LEN     30


// Number of scavenged board IDs we can store in our configuration
#define SCAVENGED_BOARD_LIST_LEN 5

// Number of bytes in a boardID. 4 bytes of data and two check bytes.
#define BOARD_ID_BYTES 4
#define BOARD_ID_CHECK_BYTES 2
#define BOARD_ID_LEN BOARD_ID_BYTES+BOARD_ID_CHECK_BYTES
#define BOARD_ID_BUF_LEN BOARD_ID_LEN+1     // Store null terminator - just makes everything easier

// This is what an uninitialized board ID looks like
const char UninitializedID[BOARD_ID_LEN] = {0,0,0,0,0,0};

// Structure to save the configuration for this sketch in EEPROM
typedef struct
{
      char WifiSSID[WIFI_SSID_LEN];     
      char WifiPassword[WIFI_PASSWORD_LEN];
      char IFTTTKey[IFTTT_KEY_LEN];  
      char MyBoardID[BOARD_ID_BUF_LEN];   
      unsigned char NumScavengedBoards;
      char ScavengedBoardList[SCAVENGED_BOARD_LIST_LEN][BOARD_ID_BUF_LEN];

}config_t;

// Default values for configuration items
#define DEFAULT_WIFI_SSID "SSID goes here"
#define DEFAULT_WIFI_PASSWORD "Password goes here"
#define DEFAULT_IFTTT_KEY "Key goes here"


#endif  