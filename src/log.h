//
// App:         UPS Monitoring Service
// Author:      Mirco Caramori
// Copyright:   (c) 2021 Mirco Caramori
// Repository:  https://github.com/padus/vacation
//
// Description: Messages provider
//
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: TYPE_SERVICE
//
// MessageText:
//
// Service
//
#define TYPE_SERVICE                     ((WORD)0x00000010L)

//
// MessageId: MSG_INFO
//
// MessageText:
//
// %1
//
#define MSG_INFO                         ((DWORD)0x40000001L)

//
// MessageId: MSG_WARNING
//
// MessageText:
//
// %1
//
#define MSG_WARNING                      ((DWORD)0x80000002L)

//
// MessageId: MSG_ERROR
//
// MessageText:
//
// %1
//
#define MSG_ERROR                        ((DWORD)0xC0000003L)

