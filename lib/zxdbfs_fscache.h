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

#ifndef _zxdbfs_fscache_h
#define _zxdbfs_fscache_h

#include <json-c/json.h>
#include <json-c/linkhash.h>

#include "zxdbfs_fscacheentry.h"

#define FSCACHE_DEFAULT_HASH_SIZE 16

typedef struct FSCache {
    json_object *cache;
} FSCache_t;

extern FSCache_t *FSCache_create();
extern int FSCache_free( FSCache_t *cache );
extern int FSCache_flush( FSCache_t *cache );

extern FSCacheEntry_t *FSCache_get( FSCache_t *cache, const char *key );
extern int FSCache_add( FSCache_t *cache, const char *key, FSCacheEntry_t *fsCacheEntry );
extern int FSCache_addAll( FSCache_t *cache, const char *key, FSCacheEntry_t *fsCacheEntry );
extern int FSCache_delete( FSCache_t *cache, const char *key );


#endif /** !_zxdbfs_fscache_h */
