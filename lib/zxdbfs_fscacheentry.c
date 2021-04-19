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

#include <stdio.h>
#include <string.h>

#include "zxdbfs_fscacheentry.h"
#include "zxdbfs_json.h"

/**
 * Return a suitably organised FSCacheEntry
 * In:
 *      fname - filename relating to the FSCacheEntry
 *      type - "dir" or "file"
 *      url - the URL the entry points at (only applicable to "file" entries)
 *      size - the size of the underlying data (only applicable to "file" entries)
 * Out:
 *      N/A
 * Returns:
 *      NULL - creation fails
 *      json_object * - creation OK
 */
FSCacheEntry_t *FSCacheEntry_create( const char *fname,
                                     FSCacheEntryType type,
                                     const char *url,
                                     int size ) {

    if ( fname == NULL || (FSCacheEntry_isValidType( type ) != 0) ) {
        return NULL;
    }

    FSCacheEntry_t *tmpobj = json_object_new_object();
    if ( tmpobj == NULL ) {
        return NULL;
    }

    FSCacheEntry_settype( tmpobj, type );
    FSCacheEntry_setfname( tmpobj, fname );
    FSCacheEntry_seturl( tmpobj, url );
    FSCacheEntry_setsize( tmpobj, size );
    if ( type == FSCACHEENTRY_DIR ) {
        FSCacheEntry_setfiles( tmpobj );
    }

    return tmpobj;
}

/**
 * Releases a cache entry object
 * In:
 *      obj - object to free
 * Out:
 *      N/A
 * Returns:
 *      N/A
 */
void FSCacheEntry_free( FSCacheEntry_t *obj ) {

    if ( obj == NULL ) {
        return;
    }

    json_object_put( obj );
}

/**
 * Copies a FSCacheEntry
 * In:
 *      obj - item to copy. Required

 * Out:
 *      N/A
 * Returns:
 *      newobj - copied item or NULL
 */
FSCacheEntry_t *FSCacheEntry_clone( FSCacheEntry_t *obj ) {

    if ( obj == NULL ) {
        return NULL;
    }

    json_object *tmpobj = NULL;
    int rv = json_object_deep_copy( obj, &tmpobj, NULL );
    if ( rv != 0 ) {
        return NULL;
    }

    return tmpobj;
}

/**
 * Adds a "file" (or "dir") FSCacheEntry to a "dir" FSCacheEntry
 * In:
 *     fsCacheEntry - a "dir" FSCacheEntry. Required
 *     fileFSCacheEntry - a "file" or "dir" FSCacheEntry. Required
 * Out:
 *      N/A
 * Returns:
 *      0 = success
 *      1 = failure
 */
int FSCacheEntry_addFile( FSCacheEntry_t *fsCacheEntry,
                          FSCacheEntry_t *fileFSCacheEntry ) {

    if ( fsCacheEntry == NULL || fileFSCacheEntry == NULL ) {
        return 1;
    }

    FSCacheEntryType fsctype = FSCacheEntry_gettype( fsCacheEntry );
    FSCacheEntryType filefsctype = FSCacheEntry_gettype( fileFSCacheEntry );
    if ( fsctype != FSCACHEENTRY_DIR &&
         filefsctype != FSCACHEENTRY_FILE &&
         filefsctype != FSCACHEENTRY_DIR &&
         filefsctype != FSCACHEENTRY_DIR_STUB ) {
        return 1;
    }

    json_object *files = json_object_object_get( fsCacheEntry, "files" );
    if ( files == NULL ) {
        files = json_object_new_array();
    }

    return json_object_array_add( files, fileFSCacheEntry );
}

/**
 * Returns whether the given type is valid
 * In:
 *      type - Type to check. Required.
 * Out:
 *      N/A
 * Returns:
 *      0 = valid type
 *      1 = invalid type
 */
int FSCacheEntry_isValidType( FSCacheEntryType type ) {
    
    if ( type == FSCACHEENTRY_FILE || type == FSCACHEENTRY_DIR ||
         type == FSCACHEENTRY_DIR_STUB ) {
        return 0;
    }

    return 1;
}

int FSCacheEntry_getnfiles( FSCacheEntry_t *fsCacheEntry ) {

    json_object *files = json_object_object_get( fsCacheEntry, "files" );
    if ( files == NULL ) {
        return 0;
    }

    return json_object_array_length( files );
}

FSCacheEntryType FSCacheEntry_gettype( FSCacheEntry_t *fsCacheEntry ) {

    json_object *type = json_object_object_get( fsCacheEntry, "type" );
    if ( type == NULL ) {
        return FSCACHEENTRY_UNKNOWN;
    }

    const char *stype = json_object_get_string( type );
    if ( stype == NULL ) {
        return FSCACHEENTRY_UNKNOWN;
    }

    if ( strcmp( stype, "dir" ) == 0 ) {
        return FSCACHEENTRY_DIR;
    } else {
        if ( strcmp( stype, "dirstub" ) == 0 ) {
            return FSCACHEENTRY_DIR_STUB;
        } else {
            if ( strcmp( stype, "file" ) == 0 ) {
                return FSCACHEENTRY_FILE;
            }
        }
    }

    return FSCACHEENTRY_UNKNOWN;
}

int FSCacheEntry_settype( FSCacheEntry_t *fsCacheEntry, FSCacheEntryType ptype ) {

    int rv = 0;

    if ( fsCacheEntry == NULL ) {
        return 1;
    }

    switch ( ptype ) {
        case FSCACHEENTRY_DIR: {
            json_object *tmpobj = json_object_new_string( "dir" );
            if ( tmpobj == NULL ) {
                return 1;
            }
            return json_object_object_add( fsCacheEntry, "type", tmpobj );
        }
        case FSCACHEENTRY_DIR_STUB: {
            json_object *tmpobj = json_object_new_string( "dirstub" );
            if ( tmpobj == NULL ) {
                return 1;
            }
            return json_object_object_add( fsCacheEntry, "type", tmpobj );
        }
        case FSCACHEENTRY_FILE: {
            json_object *tmpobj = json_object_new_string( "file" );
            if ( tmpobj == NULL ) {
                return 1;
            }
            return json_object_object_add( fsCacheEntry, "type", tmpobj );
        }
    }

    json_object *tmpobj = json_object_new_string( "unknown" );
    if ( tmpobj == NULL ) {
        return 1;
    }
    return json_object_object_add( fsCacheEntry, "type", tmpobj );
}

const char *FSCacheEntry_getfname( FSCacheEntry_t *fsCacheEntry ) {

    json_object *fname = json_object_object_get( fsCacheEntry, "fname" );
    if ( fname == NULL ) {
        return NULL;
    }

    return json_object_get_string( fname );
}

int FSCacheEntry_setfname( FSCacheEntry_t *fsCacheEntry, const char *fname ) {

    if ( fsCacheEntry == NULL || fname == NULL ) {
        return 1;
    }

    json_object *tmpobj = json_object_new_string( fname );
    return json_object_object_add( fsCacheEntry, "fname", tmpobj );
}

const char *FSCacheEntry_geturl( FSCacheEntry_t *fsCacheEntry ) {

    json_object *url = json_object_object_get( fsCacheEntry, "url" );
    if ( url == NULL ) {
        return NULL;
    }

    return json_object_get_string( url );
}

int FSCacheEntry_seturl( FSCacheEntry_t *fsCacheEntry, const char *url ) {

    if ( fsCacheEntry == NULL || url == NULL ) {
        return 1;
    }
    
    json_object *tmpobj = json_object_new_string( url );
    return json_object_object_add( fsCacheEntry, "url", tmpobj );
}

int FSCacheEntry_getsize( FSCacheEntry_t *fsCacheEntry ) {

    json_object *size = json_object_object_get( fsCacheEntry, "size" );
    if ( size == NULL ) {
        return 0;
    }

    return json_object_get_int( size );
}

int FSCacheEntry_setsize( FSCacheEntry_t *fsCacheEntry, int size ) {

    if ( fsCacheEntry == NULL ) {
        return 1;
    }

    json_object *tmpobj = json_object_new_int( size );
    return json_object_object_add( fsCacheEntry, "size", tmpobj );
}

FSCacheEntry_t *FSCacheEntry_getfile( FSCacheEntry_t *fsCacheEntry, int findex ) {

    json_object *files = json_object_object_get( fsCacheEntry, "files" );
    if ( files == NULL ) {
        return NULL;
    }

    return json_object_array_get_idx( files, findex );
}

int FSCacheEntry_setfiles( FSCacheEntry_t *fsCacheEntry ) {

    if ( fsCacheEntry == NULL ) {
        return 1;
    }

    json_object *files = json_object_new_array();
    return json_object_object_add( fsCacheEntry, "files", files );
}

