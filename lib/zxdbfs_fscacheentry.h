/*
  Copyright (C) 2021  Alligator Descartes <alligator.descartes@hermitretro.com>

 This file is part of zxdbfs.

     zxdbfs is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     zxdbfs is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with zxdbfs.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef _zxdbfs_fscacheentry_h
#define _zxdbfs_fscacheentry_h

#include <json-c/json.h>

typedef enum { 
    FSCACHEENTRY_UNKNOWN, 
    FSCACHEENTRY_DIR, 
    FSCACHEENTRY_FILE,
    FSCACHEENTRY_DIR_STUB
} FSCacheEntryType;

/*
struct FSCacheEntry;
typedef struct FSCacheEntry {
    FSCacheEntryType type;
    char fname[256];
    char url[256];
    size_t size;
    struct FSCacheEntry **files;
    int nfiles;
} FSCacheEntry_t;
*/
#define FSCacheEntry_t json_object

extern FSCacheEntry_t *FSCacheEntry_create( const char *fname,
                                            FSCacheEntryType type,
                                            const char *url,
                                            int size );
extern FSCacheEntry_t *FSCacheEntry_clone( FSCacheEntry_t *obj );
extern void FSCacheEntry_free( FSCacheEntry_t *fsCacheEntry );
extern int FSCacheEntry_addFile( FSCacheEntry_t *fsCacheEntry,
                                 FSCacheEntry_t *fileFSCacheEntry );
extern int FSCacheEntry_isValidType( FSCacheEntryType type );

extern int FSCacheEntry_getnfiles( FSCacheEntry_t *fsCacheEntry );
extern FSCacheEntryType FSCacheEntry_gettype( FSCacheEntry_t *fsCacheEntry );
extern int FSCacheEntry_settype( FSCacheEntry_t *fsCacheEntry, FSCacheEntryType type );
extern const char *FSCacheEntry_getfname( FSCacheEntry_t *fsCacheEntry );
extern int FSCacheEntry_setfname( FSCacheEntry_t *fsCacheEntry, const char *fname );
extern const char *FSCacheEntry_geturl( FSCacheEntry_t *fsCacheEntry );
extern int FSCacheEntry_seturl( FSCacheEntry_t *fsCacheEntry, const char *url );
extern int FSCacheEntry_getsize( FSCacheEntry_t *fsCacheEntry );
extern int FSCacheEntry_setsize( FSCacheEntry_t *fsCacheEntry, int url );
extern FSCacheEntry_t *FSCacheEntry_getfile( FSCacheEntry_t *fsCacheEntry, int findex );
extern int FSCacheEntry_setfiles( FSCacheEntry_t *fsCacheEntry );

#endif /** !_zxdbfs_fscacheentry_h */
