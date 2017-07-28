#ifndef DLPORTIO_H
  #define DLPORTIO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IN
  #define IN
#endif

#define DLPORT_API _stdcall

unsigned char DLPORT_API
DlPortReadPortUchar(
    IN unsigned long Port
    );

unsigned short DLPORT_API
DlPortReadPortUshort(
    IN unsigned long Port
    );

unsigned long DLPORT_API
DlPortReadPortUlong(
    IN unsigned long Port
    );

void DLPORT_API
DlPortReadPortBufferUchar(
    IN unsigned long Port,
    IN unsigned char* Buffer,
    IN unsigned long  Count
    );

void DLPORT_API
DlPortReadPortBufferUshort(
    IN unsigned long Port,
    IN unsigned short* Buffer,
    IN unsigned long Count
    );

void DLPORT_API
DlPortReadPortBufferUlong(
    IN unsigned long Port,
    IN unsigned long* Buffer,
    IN unsigned long Count
    );

void DLPORT_API
DlPortWritePortUchar(
    IN unsigned long Port,
    IN unsigned char Value
    );

void DLPORT_API
DlPortWritePortUshort(
    IN unsigned long Port,
    IN unsigned short Value
    );

void DLPORT_API
DlPortWritePortUlong(
    IN unsigned long Port,
    IN unsigned long Value
    );

void DLPORT_API
DlPortWritePortBufferUchar(
    IN unsigned long Port,
    IN unsigned char* Buffer,
    IN unsigned long  Count
    );

void DLPORT_API
DlPortWritePortBufferUshort(
    IN unsigned long Port,
    IN unsigned short* Buffer,
    IN unsigned long Count
    );

void DLPORT_API
DlPortWritePortBufferUlong(
    IN unsigned long Port,
    IN unsigned long* Buffer,
    IN unsigned long Count
    );

#ifdef __cplusplus
}
#endif

#endif // DLPORTIO_H
