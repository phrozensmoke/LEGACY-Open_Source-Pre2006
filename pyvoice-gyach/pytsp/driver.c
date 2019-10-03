#include "config.h"

#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#ifdef __FreeBSD__
#include <sys/time.h>
#endif

#include "win32.h"
#include "wine/driver.h"
#include "wine/pe_image.h"
#include "wine/winreg.h"
#include "wine/vfw.h"
#include "registry.h"
#include "ldt_keeper.h"
#include "driver.h"
#include "ext.h"

extern char* def_path;

#if 1

/*
 * STORE_ALL/REST_ALL seems like an attempt to workaround problems due to
 * WINAPI/no-WINAPI bustage.
 *
 * There should be no need for the STORE_ALL/REST_ALL hack once all
 * function definitions agree with their prototypes (WINAPI-wise) and
 * we make sure, that we do not call these functions without a proper
 * prototype in scope.
 */

#define STORE_ALL
#define REST_ALL
#else
// this asm code is no longer needed
#define STORE_ALL \
    __asm__ __volatile__ ( \
    "push %%ebx\n\t" \
    "push %%ecx\n\t" \
    "push %%edx\n\t" \
    "push %%esi\n\t" \
    "push %%edi\n\t"::)

#define REST_ALL \
    __asm__ __volatile__ ( \
    "pop %%edi\n\t" \
    "pop %%esi\n\t" \
    "pop %%edx\n\t" \
    "pop %%ecx\n\t" \
    "pop %%ebx\n\t"::)
#endif

static int needs_free=0;

#ifdef MPLAYER    /* added: PhrozenSmoke, so the object can be freed after use */
	ldt_fs_t* MYLDT;
#endif


void SetCodecPath(const char* path)
{
    if(needs_free)free(def_path);
    if(path==0)
    {
	def_path=WIN32_PATH;
	needs_free=0;
	return;
    }
    def_path = (char*) malloc(strlen(path)+1);
    strcpy(def_path, path);
    needs_free=1;
}

static DWORD dwDrvID = 0;

LRESULT WINAPI SendDriverMessage(HDRVR hDriver, UINT message,
				 LPARAM lParam1, LPARAM lParam2)
{
    DRVR* module=(DRVR*)hDriver;
    int result;
#ifndef __svr4__
    char qw[300];
#endif
#ifdef DETAILED_OUT
    printf("SendDriverMessage: driver %X, message %X, arg1 %X, arg2 %X\n", hDriver, message, lParam1, lParam2);
#endif
    if (!module || !module->hDriverModule || !module->DriverProc) return -1;
#ifndef __svr4__
    __asm__ __volatile__ ("fsave (%0)\n\t": :"r"(&qw));
#endif

    Setup_FS_Segment();

    STORE_ALL;
    result=module->DriverProc(module->dwDriverID, hDriver, message, lParam1, lParam2);
    REST_ALL;

#ifndef __svr4__
    __asm__ __volatile__ ("frstor (%0)\n\t": :"r"(&qw));
#endif

#ifdef DETAILED_OUT
    printf("\t\tResult: %X\n", result);
#endif
    return result;
}

void DrvClose(HDRVR hDriver)
{
    if (hDriver)
    {
	DRVR* d = (DRVR*)hDriver;
	if (d->hDriverModule)
	{
	    Setup_FS_Segment();
	    if (d->DriverProc)
	    {
		SendDriverMessage(hDriver, DRV_CLOSE, 0, 0);
		d->dwDriverID = 0;
		SendDriverMessage(hDriver, DRV_FREE, 0, 0);
	    }
	    FreeLibrary(d->hDriverModule);
	}
	free(d);
    }
    CodecRelease();
	
    /* printf("DrvClose called\n"); fflush(stdout);  */

    /* added: below by PhrozenSmoke,  This MPlayer library was not 
	closing the file descriptor to '/dev/zero' in the object 'ldt_fs->fd' which 
	is created with a needed call to 'Setup_LDT_Keeper()' in 'DrvOpen' below.  
	As a result, after a half hour or so 
	of use of loading/unloading the truespeech codec, the app would eventually 
	segfault with a complaints about 
	'Cannot open /dev/zero for READ+WRITE. Check permissions!' (from 
	files ext.c and ldt_keeper.c.... would say 'too many files open' and 
	quit with a nasty segfault.  Hopefully, this fixes that problem, and it 
	should eliminate a memory leak caused by the 'ldt_fs' objects being created
	in 'DrvOpen' (call to 'Setup_LDT_Keeper()' ), 
	but then never freed when the Codec was unloaded.  Either way, it 
	shouldn't HURT to dealloc the 'ldt_fs' memory and close its 'ldt_fs->fd' 
	file handle to '/dev/zero' after it is no longer needed once the codec has 
	been unloaded and released

    */
#ifdef MPLAYER  /* added: PhrozenSmoke, resource cleanup */
   Restore_LDT_Keeper(MYLDT);
#endif
}

//DrvOpen(LPCSTR lpszDriverName, LPCSTR lpszSectionName, LPARAM lParam2)
HDRVR DrvOpen(LPARAM lParam2)
{
    NPDRVR hDriver;

    char unknown[0x124];
    const char* filename = (const char*) ((ICOPEN*) lParam2)->pV1Reserved;

#ifdef MPLAYER
    MYLDT=Setup_LDT_Keeper();   /*changed: PhrozenSmoke */
    // printf("Loading codec DLL: '%s'\n",filename); /* disabled: PhrozenSmoke */
#endif

    hDriver = (NPDRVR) malloc(sizeof(DRVR));
    if (!hDriver)
	return ((HDRVR) 0);
    memset((void*)hDriver, 0, sizeof(DRVR));

    CodecAlloc();
    Setup_FS_Segment();

    hDriver->hDriverModule = LoadLibraryA(filename);
    if (!hDriver->hDriverModule)
    {
	printf("Can't open library %s\n", filename);
	DrvClose((HDRVR)hDriver);
	return ((HDRVR) 0);
    }

    hDriver->DriverProc = (DRIVERPROC) GetProcAddress(hDriver->hDriverModule,
						      "DriverProc");
    if (!hDriver->DriverProc)
    {
	printf("Library %s is not a valid VfW/ACM codec\n", filename);
	DrvClose((HDRVR)hDriver);
	return ((HDRVR) 0);
    }

    TRACE("DriverProc == %X\n", hDriver->DriverProc);
    SendDriverMessage((HDRVR)hDriver, DRV_LOAD, 0, 0);
    TRACE("DRV_LOAD Ok!\n");
    SendDriverMessage((HDRVR)hDriver, DRV_ENABLE, 0, 0);
    TRACE("DRV_ENABLE Ok!\n");
    hDriver->dwDriverID = ++dwDrvID; // generate new id

    // open driver and remmeber proper DriverID
    hDriver->dwDriverID = SendDriverMessage((HDRVR)hDriver, DRV_OPEN, (LPARAM) unknown, lParam2);
    TRACE("DRV_OPEN Ok!(%X)\n", hDriver->dwDriverID);

    /* printf("Loaded DLL driver %s\n", filename); */ /* disabled: PhrozenSmoke */
    return (HDRVR)hDriver;
}
