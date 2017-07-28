#ifndef _UTILS_H_
#define _UTILS_H_

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
void makeTimeDateStr(char* dest, const char* source, const SYSTEMTIME* in_sys_time=0);

#endif