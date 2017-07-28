#include "StdAfx.h"
#include "utils.h"
#include "cImageLoader.h"

/*
Appends the current date and time to the string "source",
placing it between the filename and the extension if an
extension exists.  Places the resulting string in "dest".

In other words, it turns :

bob.txt

...into...

bob.02.12.28.14.55.44.txt

If a station ID environment variable was found, it is
placed before the date string, separated from the date string
by an underscore, e.g. :

bob.patient5_02.12.28.14.55.44.txt

*/
void makeTimeDateStr(char* dest, const char* source, const SYSTEMTIME* in_sys_time) {

	SYSTEMTIME sys_time;

	if (in_sys_time) {
		sys_time = *in_sys_time;
	}
	else {
		GetSystemTime(&sys_time);
	}

	// Make the string for the time and date alone
	char timeDateStr[1000];
	sprintf(timeDateStr, "%04d.%02d.%02d.%02d.%02d.%02d",
		sys_time.wYear, sys_time.wMonth, sys_time.wDay,
		sys_time.wHour, sys_time.wMinute, sys_time.wSecond);

	char tmp[1000];

	strcpy(tmp, source);

	// Find the beginning of the extension in the source string.  The
	// start of the extension is defined to be the character following the
	// last period in the source string.
	char* extension = find_extension(tmp);

	// If there was no extension, just concatenate the source and time/date strings
	if (extension == 0) {
		sprintf(dest, "%s.%s", tmp, timeDateStr);
	}

	// Otherwise stick the time/date string between the main source string and its extension
	else {
		sprintf(dest, "%s.%s.%s", tmp, timeDateStr, extension);
	}

} // makeTimeDateStr
