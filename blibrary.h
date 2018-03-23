#ifndef BLIBRARY_H
#define BLIBRARY_H

/*
 * BLibrary
 * Used to communicate to each library
 * Implement for each library
 */

#include "bloader.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Information about module
typedef struct BLinfo
{
	char name[256];
	int version;
	char description[4096];
} blinfo;

BLOADER_IMPORT int blibrary_initialize(blmodule * module);

BLOADER_IMPORT void blibrary_info(blinfo * info);

BLOADER_IMPORT void blibrary_deinit();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // BLOADER_H
