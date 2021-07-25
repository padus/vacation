//
// App:         UPS Monitoring Service
// Author:      Mirco Caramori
// Copyright:   (c) 2021 Mirco Caramori
// Repository:  https://github.com/padus/vacation
//
// Description: Precompiled system and std headers
//

#pragma once

// System Includes -------------------------------------------------------------------------------------------------------------

#define _UNICODE                                // C/C++ headers
#define UNICODE                                 // Windows headers

#define WIN32_LEAN_AND_MEAN                     // No extra Windows stuff

#include <sdkddkver.h>
#include <windows.h>

#include <shellapi.h>
#include <winhttp.h>
#include <strsafe.h>

#include <string>

#include "version.h"
#include "log.h"

// EOF -------------------------------------------------------------------------------------------------------------------------
