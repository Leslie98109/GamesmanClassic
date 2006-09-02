#ifndef GMCORE_BPDB_MISC_H
#define GMCORE_BPDB_MISC_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include "gamesman.h"

//#define TRUE	1
//#define FALSE	0

//typedef char BOOLEAN;
typedef gzFile dbFILE;

typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef long long int INT64;
typedef unsigned long long int UINT64;
//typedef unsigned int UINT64;

typedef UINT32 GMSTATUS;

#define BITSINBYTE 8
#define BITSINPOS 64
//#define BITSINPOS 32

#define SAFE_FREE(ptr) \
		if(NULL != ptr) \
			free(ptr);

#define BPDB_TRACE(fnc, msg, err) \
		fprintf(stderr, "ERROR CODE 0x%04x : %s : %s\n", (err), (fnc), (msg));

#define STATUS_SUCCESS 0x0
#define STATUS_NOT_ENOUGH_MEMORY 0x1

#define GMSUCCESS(status) \
		(STATUS_SUCCESS == (status))



#define VALUESLOT 0
#define MEXSLOT 1
#define REMSLOT 2
#define VISITEDSLOT 3

/* List structure for schemes */
typedef struct Schemelist {
	// numeric identifier
	int			scheme;

	// pointers to load and save functions
	UINT64		(*read_varnum)	( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit );
    BOOLEAN		(*write_varnum)	( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips );

	BOOLEAN		indicator;

	// next scheme
	struct Schemelist *next;
} *Scheme_List;

Scheme_List scheme_list_new();
Scheme_List scheme_list_add(Scheme_List sl, int schemenum, UINT64 (*read_varnum)( dbFILE *inFile, BYTE *inputBuffer, UINT8 *offset, BOOLEAN alreadyReadFirstBit ), BOOLEAN (*write_varnum)( dbFILE *outFile, BYTE *outputBuffer, UINT8 *offset, UINT64 consecutiveSkips ), BOOLEAN indicator);

#endif /* GMCORE_BPDB_MISC_H */
