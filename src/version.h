//
// App:         UPS Monitoring Service
// Author:      Mirco Caramori
// Copyright:   (c) 2021 Mirco Caramori
// Repository:  https://github.com/padus/vacation
//
// Description: Version info
//

#ifdef NDEBUG
#define VER_DEBUG                   0
#else
#define VER_DEBUG                   VS_FF_DEBUG
#endif

#define VER_VERSION                 1,12,29,0
#define VER_VERSION_STR             L"1.12.29\0"

#define VER_AUTHOR_STR              L"Mirco Caramori"
#define VER_COPYRIGHT_STR           L"(c) 2021 Mirco Caramori"

#define VER_PRODUCTNAME_STR         L"UPS Monitoring Service"
#define VER_FILENAME_STR            L"vacation.exe"
#define VER_INTERNALNAME_STR        L"vacation"

#define VER_DESCRIPTION_STR         L"Hubitat UPS AC power presence and battery percentage"
