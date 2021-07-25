;//
;// App:         UPS Monitoring Service
;// Author:      Mirco Caramori
;// Copyright:   (c) 2021 Mirco Caramori
;// Repository:  https://github.com/padus/vacation
;//
;// Description: Messages provider
;//

LanguageNames=(English=0x409:MSG00409)

MessageIdTypedef=WORD

MessageId=0x10
SymbolicName=TYPE_SERVICE
Language=English
Service
.

MessageIdTypedef=DWORD

MessageId=0x01
Severity=Informational
Facility=Application
SymbolicName=MSG_INFO
Language=English
%1
.

MessageId=0x02
Severity=Warning
Facility=Application
SymbolicName=MSG_WARNING
Language=English
%1
.

MessageId=0x03
Severity=Error
Facility=Application
SymbolicName=MSG_ERROR
Language=English
%1
.
