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
#include "zxdbfs_gameid.h"
#include "zxdbfs_paths.h"

/**
 */
FSCacheEntry_t *FSCacheEntry_createFromByLetter( const char *path, 
                                                 json_object *byLetterRoot_o ) {

    if ( path == NULL || byLetterRoot_o == NULL ) {
        return NULL;
    }

    FSCacheEntry_t *dirEntry = 
        FSCacheEntry_create( path, FSCACHEENTRY_DIR, NULL, 0 );
    if ( dirEntry == NULL ) {
        return NULL;
    }

    /** Extract the relevant JSON fields */
    json_object *hits = json_object_object_get( byLetterRoot_o, "hits" );
    if ( hits == NULL ) {
        FSCacheEntry_free( dirEntry );
        return NULL;
    }
    json_object *hhits = json_object_object_get( hits, "hits" );
    if ( hhits == NULL ) {
        FSCacheEntry_free( dirEntry );
        return NULL;
    }

    /** Process each game as a new subdirectory */
    for ( int i = 0 ; i < json_object_array_length( hhits ) ; i++ ) {
        json_object *temp = json_object_array_get_idx( hhits, i );
        if ( temp != NULL ) {
            json_object *tempsource = json_object_object_get( temp, "_source" );
            if ( tempsource != NULL ) {
                json_object *title = json_object_object_get( tempsource, "title" );
                json_object *lid = json_object_object_get( temp, "_id" );

                /** We need to synthesize a unique filename due to duplicate titles */
                char fname[256];
                sprintf( fname, "%s_%s", json_object_get_string( title ), json_object_get_string( lid )  );

                /** Sanitise the filename in case there are illegal characters */
                for ( int j = 0 ; j < strlen( fname ) ; j++ ) {
                    if ( fname[j] == '/' || fname[j] == ':' ) {
                        fname[j] = '_';
                    }
                }

                char fullpath[256];
                sprintf( fullpath, "%s/%s", path, fname );

                FSCacheEntry_t *fsCacheEntry =
                    FSCacheEntry_create( fullpath, FSCACHEENTRY_DIR_STUB, NULL, 0 );
                FSCacheEntry_addFile( dirEntry, fsCacheEntry ); 
            }
        } else {
            printf( "failed to get JSON source for %d\n", i );
        }
    }

    return dirEntry;
}
