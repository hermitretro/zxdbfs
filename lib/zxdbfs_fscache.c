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

#include "zxdbfs_fscache.h"
#include "zxdbfs_json.h"

/**
 * Initialise a new FSCache
 * In:
 *      N/A
 * Out:
 *      N/A
 * Returns:
 *      New FSCache
 */
FSCache_t *FSCache_create() {

    FSCache_t *tmp = (FSCache_t *)malloc( sizeof( FSCache_t ) );
    if ( tmp == NULL ) {
        return NULL;
    }

    tmp->cache = json_object_new_object();
    if ( tmp->cache == NULL ) {
        free( tmp );
        return NULL;
    }

    return tmp;
}

/**
 * Frees the cache
 * In:
 *      N/A
 * Out:
 *      N/A
 * Returns:
 *      0 = success
 *      1 = failure
 */
int FSCache_free( FSCache_t *cache ) {

    if ( cache == NULL || cache->cache == NULL ) {
        return 1;
    }

    json_object_put( cache->cache );
    free( cache );

    return 0;
}

/**
 * Flushes the cache
 * In:
 *      N/A
 * Out:
 *      N/A
 * Returns:
 *      0 = success
 *      1 = failure
 */
int FSCache_flush( FSCache_t *cache ) {

    if ( cache == NULL || cache->cache == NULL ) {
        return 1;
    }

    json_object_put( cache->cache );
    cache->cache = json_object_new_object();
    if ( cache->cache == NULL ) {
        free( cache );
        return 1;
    }

    return 0;
}

/**
 * Adds an item to the cache. This will overwrite anything already at the
 * key
 * In:
 *      key - cache key. Required.
 *      fsCacheEntry - cache value. Required
 * Out:
 *      N/A
 * Returns:
 *      0 = success
 *      1 = failure
 */
int FSCache_add( FSCache_t *cache, const char *key, FSCacheEntry_t *fsCacheEntry ) {

    if ( cache == NULL || key == NULL || fsCacheEntry == NULL || cache->cache == NULL ) {
        return 1;
    }

    return json_object_object_add( cache->cache, key, fsCacheEntry );
}

/**
 * Adds an item to the cache and all it's available sub-entries.
 * This will overwrite anything already at the
 * key
 * In:
 *      key - cache key. Required.
 *      fsCacheEntry - cache value. Required
 * Out:
 *      N/A
 * Returns:
 *      0 = success
 *      1 = failure
 */
int FSCache_addAll( FSCache_t *cache, const char *key, FSCacheEntry_t *fsCacheEntry ) {

    if ( cache == NULL || key == NULL || fsCacheEntry == NULL || cache->cache == NULL ) {
        return 1;
    }

    /** Add top-level entry */
    int rv = json_object_object_add( cache->cache, key, fsCacheEntry );
    if ( rv != 0 ) {
        return 1;
    }

    /** Traverse 'files' and recursively add the entries */
    int nfiles = FSCacheEntry_getnfiles( fsCacheEntry );
    for ( int i = 0 ; i < nfiles ; i++ ) {
        FSCacheEntry_t *file = FSCacheEntry_getfile( fsCacheEntry, i );
        if ( file == NULL ) {
            return 1;
        }

        json_object *subclone = NULL;
        if ( json_object_deep_copy( file, &subclone, NULL ) == 1 ) {
            return 1;
        }
        const char *file_fname = FSCacheEntry_getfname( file );
        rv = FSCache_addAll( cache, FSCacheEntry_getfname( file ), subclone );
        if ( rv != 0 ) {
            return 1;
        }
    }

    return 0;
}

/**
 * Retrieves an item from the cache
 * In:
 *      cache - the cache. Required
 *      key - cache key. Required.
 * Out:
 *      N/A
 * Returns:
 *      Copy of the cached object or NULL
 */
FSCacheEntry_t *FSCache_get( FSCache_t *cache, const char *key ) {

    if ( cache == NULL || key == NULL || cache->cache == NULL ) {
        return NULL;
    }

    json_object *tmp = json_object_object_get( cache->cache, key );
    if ( tmp == NULL ) {
        return NULL;
    }
}

/**
 * Deletes an item from the cache
 * In:
 *      cache - the cache. Required
 *      key - cache key. Required.
 * Out:
 *      N/A
 * Returns:
 *      0 = success
 *      1 = failure
 *      2 = cache key not present
 */
int FSCache_delete( FSCache_t *cache, const char *key ) {

    if ( cache == NULL || key == NULL ) {
        return 1;
    }
    
    json_object *tmpobj = json_object_object_get( cache->cache, key );
    if ( tmpobj == NULL ) {
        /** Not present in the cache */
        return 2;
    }

    json_object_object_del( cache->cache, key );

    return 0;
}
