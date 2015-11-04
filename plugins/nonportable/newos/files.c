/******************************************************************************
 * Copyright © 2014-2015 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include "../../includes/mutex.h"

#ifndef MAP_FILE
#define MAP_FILE        0
#define MS_ASYNC	1		/* Sync memory asynchronously.  */
#define MS_SYNC		4		/* Synchronous memory sync.  */
#define MS_INVALIDATE	2		/* Invalidate the caches.  */
extern int munmap (void *__addr, size_t __len) __THROW;
extern int mprotect (void *__addr, size_t __len, int __prot) __THROW;
extern int msync (void *__addr, size_t __len, int __flags);
#endif

void PostMessage(const char* format, ...);

int32_t os_supports_mappedfiles() { return(0); }
char *os_compatible_path(char *str)
{
    static char fname[1024];
    if ( strncmp(str,"/persistent/",strlen("/persistent/")) == 0 )
        return(str);
    if ( str[0] == '/' )
        str++;
    else if ( str[0] == '.' && str[1] == '/' )
        str += 2;
    sprintf(fname,"/persistent/%s",str);
    return(fname);
}

int32_t portable_truncate(char *fname,long filesize) { return(truncate(fname,filesize)); }

char *OS_rmstr() { return("rm"); }

void ensure_directory(char *dirname)
{
    FILE *fp; int32_t retval;
    PostMessage("ensure_directory.(%s)\n",dirname);
    if ( (fp= fopen(os_compatible_path(dirname),"rb")) == 0 )
    {
        retval = mkdir(dirname,511);
        PostMessage("ensure_directory.(%s) retval.%d\n",os_compatible_path(dirname),retval);
    }
    else
    {
        fclose(fp);
        PostMessage("%s exists\n",os_compatible_path(dirname));
    }
}

void *map_file(char *fname,uint64_t *filesizep,int32_t enablewrite)
{
	//void *mmap64(void *addr,size_t len,int32_t prot,int32_t flags,int32_t fildes,off_t off);
	int32_t fd,rwflags,flags = MAP_FILE|MAP_SHARED;
	uint64_t filesize;
    void *ptr = 0;
	*filesizep = 0;
	if ( enablewrite != 0 )
		fd = open(fname,O_RDWR);
	else fd = open(fname,O_RDONLY);
	if ( fd < 0 )
	{
		printf("map_file: error opening enablewrite.%d %s\n",enablewrite,fname);
        return(0);
	}
    if ( *filesizep == 0 )
        filesize = (uint64_t)lseek(fd,0,SEEK_END);
    else filesize = *filesizep;
	rwflags = PROT_READ;
	if ( enablewrite != 0 )
		rwflags |= PROT_WRITE;
//#if __i386__
	ptr = mmap(0,filesize,rwflags,flags,fd,0);
//#else
	//ptr = mmap64(0,filesize,rwflags,flags,fd,0);
//#endif
	close(fd);
    if ( ptr == 0 || ptr == MAP_FAILED )
	{
		printf("map_file.write%d: mapping %s failed? mp %p\n",enablewrite,fname,ptr);
		return(0);
	}
	*filesizep = filesize;
	return(ptr);
}
