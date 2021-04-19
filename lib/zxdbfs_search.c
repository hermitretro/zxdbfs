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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "zxdbfs_fscacheentry.h"
#include "zxdbfs_http.h"
#include "zxdbfs_json.h"
#include "zxdbfs_paths.h"
#include "zxdbfs_search.h"

FSCacheEntry_t *FSCacheEntry_createFromSearch( const char *path,
                                               json_object *searchData_o,
                                               float minscore,
                                               const char *searchTerm ) {

    if ( path == NULL || searchData_o == NULL ) {
        return NULL;
    }

    FSCacheEntry_t *dirEntry = FSCacheEntry_create( path, FSCACHEENTRY_DIR, NULL, 0 );
    if ( dirEntry == NULL ) {
        return NULL;
    }

    /** Extract the relevant JSON fields */
    json_object *hits = json_object_object_get( searchData_o, "hits" );
    if ( hits == NULL ) {
        printf( "malformed JSON\n" );
        return NULL;
    }
    json_object *hhits = json_object_object_get( hits, "hits" );
    if ( hhits == NULL ) {
        printf( "malformed JSON\n" );
        return NULL;
    }

    for ( int i = 0 ; i < json_object_array_length( hhits ) ; i++ ) {

        json_object *result_o = json_object_array_get_idx( hhits, i );

        json_object *source_o = json_object_object_get( result_o, "_source" );
        if ( source_o == NULL ) {
            return NULL;
        }

        json_object *stitle_o = json_object_object_get( source_o, "title" );
        json_object *id_o = json_object_object_get( result_o, "_id" );
    
        const char *fname = json_object_get_string( stitle_o );
        const char *id = json_object_get_string( id_o );

        /** Potentially filter out the result */
        json_object *score_o = json_object_object_get( result_o, "_score" );
        double score = 0;
        if ( score_o != NULL ) {
            score = json_object_get_double( score_o );
            if ( score <= minscore ) {
                continue;
            }
        }

        /** Title and publisher filter by original search term */
        int rejectRecord = 0;
        if ( searchTerm != NULL ) {
            rejectRecord = 1;
            if ( strcasestr( fname, searchTerm ) == NULL ) {
                /** Search publisher data */
                json_object *publishers_o = json_object_object_get( source_o, "publishers" );
                if ( publishers_o != NULL && json_object_is_type( publishers_o, json_type_array ) ) {
                    for ( int j = 0 ; j < json_object_array_length( publishers_o ) ; j++ ) {
                        json_object *publisher_o = json_object_array_get_idx( publishers_o, j );
                        if ( publisher_o != NULL ) {
                            json_object *publisher_name_o = json_object_object_get( publisher_o, "name" );
                            if ( publisher_name_o != NULL ) {
                                const char *publisher_name = json_object_get_string( publisher_name_o );
                                if ( strcasestr( publisher_name, searchTerm ) == NULL ) {
                                } else {
                                    rejectRecord = 0;
                                }
                            }
                        }
                    }
                }
            } else {
                rejectRecord = 0;
            }
        }

        if ( rejectRecord == 1 ) {
            continue;
        }

        char fname2[256];
        sprintf( fname2, "%s_%s", fname, id );

        char filepath[1024];
        sprintf( filepath, "%s/%s", path, fname2 );
        FSCacheEntry_t *centry = FSCacheEntry_create( filepath, FSCACHEENTRY_DIR_STUB, NULL, 0 );
        FSCacheEntry_addFile( dirEntry, centry );
    }

    return dirEntry;
}

FSCacheEntry_t *FSCacheEntry_filterSearchResults( FSCacheEntry_t *searchResults,
                                                  float minscore,
                                                  const char *searchTerm ) {

    if ( searchResults == NULL ) {
        return NULL;
    }

    int nfiles = FSCacheEntry_getnfiles( searchResults );
    printf( "nfiles: %d\n", nfiles );
    for ( int i = 0 ; i < nfiles ; i++ ) {
        FSCacheEntry_t *filex = FSCacheEntry_getfile( searchResults, i );
        dumpJSON( filex );
        if ( FSCacheEntry_gettype( filex ) != FSCACHEENTRY_FILE ) {
            continue;
        }

        printf( "-> processing %d\n", i );
    }

    return NULL;
}

FSCacheEntry_t *FSCacheEntry_getAndCreateSearch( json_object *urlcache,
                                                 const char *filepath,
                                                 const char *urlhost,
                                                 const char *urlpath,
                                                 int forceFetch ) {

    char searchTerm[128] = { 0 };
    char searchRootPath[256] = { 0 };
    int rv = getSearchTermFromPath( filepath, searchTerm, searchRootPath );
    if ( rv != 0 ) {
        return NULL;
    }

    /** We haven't retrieved this before (or we don't have a cache...) */
    /** Retrieve the data for the game ID */
    char *url = NULL;
    if ( strncmp( urlhost, "file://", 7 ) == 0 ) {
        url = strdup( urlpath );
    } else {
        url = (char *)malloc( 256 );
        memset( url, 0, 256 );
        sprintf( url, "/search?query=%s&mode=tiny&size=256&offset=0&sort=rel_desc&contenttype=SOFTWARE&availability=Available&machinetype=ZXSPECTRUM", searchTerm );
    }

    if ( url == NULL ) {
        return NULL;
    }

    printf( "URL: %s\n", url );

    /** Retrieve the full search data from ZXDB */
    json_object *urlobj = getURL( urlcache, urlhost, url );
    if ( urlobj == NULL ) {
        printf( "Failed to retrieve search data from ZXDB\n" );
        free( url );
        return NULL;
    } else {
        printf( "search data is ok\n" );
    }

    free( url );

    FSCacheEntry_t *fsCacheEntry =
        FSCacheEntry_createFromSearch( filepath, urlobj, 0, searchTerm );
    if ( fsCacheEntry == NULL ) {
        return NULL;
    }

    return fsCacheEntry;
}
