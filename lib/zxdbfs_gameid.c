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
#include "zxdbfs_http.h"
#include "zxdbfs_paths.h"

/**
 * Read game info to the cache. The path will either be:
 * /by-letter/[A-Z]/[Game Title + ID]
 * /by-letter/[A-Z]/[Game Title + ID]/POKES/*
 * /by-letter/[A-Z]/[Game Title + ID]/SCRSHOT/*
 * So, we want the bit between the third and fourth / (assuming 4 exists)
 */
FSCacheEntry_t *FSCacheEntry_createFromGame( const char *path, 
                                             json_object *gameData_o ) {

    if ( path == NULL || gameData_o == NULL ) {
        return NULL;
    }

    FSCacheEntry_t *dirEntry = FSCacheEntry_create( path, FSCACHEENTRY_DIR, NULL, 0 );
    if ( dirEntry == NULL ) {
        return NULL;
    }

    json_object *source_o = json_object_object_get( gameData_o, "_source" );
    if ( source_o == NULL ) {
        FSCacheEntry_free( dirEntry );
        return NULL;
    }

    /**
     * Flatten the releases into the directory. Assume there aren't any
     * name clashes....(there might be, but meh)
     */
    json_object *releases_o = json_object_object_get( source_o, "releases" );
    if ( releases_o == NULL ) {
        FSCacheEntry_free( dirEntry );
        return NULL;
    }

    for ( int i = 0 ; i < json_object_array_length( releases_o ) ; i++ ) {

        json_object *release_o = json_object_array_get_idx( releases_o, i );
        json_object *release_files_o = json_object_object_get( release_o, "files" );

        for ( int j = 0 ; j < json_object_array_length( release_files_o ) ; j++ ) {

            json_object *file_o = json_object_array_get_idx( release_files_o, j );

            json_object *file_path_o = json_object_object_get( file_o, "path" );
            const char *file_path = json_object_get_string( file_path_o );

            json_object *file_size_o = json_object_object_get( file_o, "size" );
            int file_size = json_object_get_int( file_size_o );

            /** Extract the root filename from the archive path */
            char filename[256] = { 0 };
            getBasename( file_path, filename );

            /**
             * Strip /pub/sinclair as the archive.org paths differ from the 
             * original WoS ones...
             */
            char fixedPath[256] = { 0 };
            fixupWoSPath( file_path, fixedPath );

            /** Synthesise the full local FS path to the file */
            char fsPath[256] = { 0 };
            sprintf( fsPath, "%s/%s", path, filename );

            /** Store the file data into the direntry */
            FSCacheEntry_t *file_fscache = FSCacheEntry_create( fsPath, FSCACHEENTRY_FILE, fixedPath, file_size );
            FSCacheEntry_addFile( dirEntry, file_fscache );
        }
    }

    /**
     * Handle the POK files
     */
    json_object *additionalDownloads_o = 
        json_object_object_get( source_o, "additionalDownloads" );
    if ( additionalDownloads_o == NULL ) {
        return NULL;
    }

    char pokesPath[256];
    sprintf( pokesPath, "%s/POKES", path );
    FSCacheEntry_t *pokesDirEntry = 
        FSCacheEntry_create( pokesPath, FSCACHEENTRY_DIR, NULL, 0 );
    if ( pokesDirEntry == NULL ) {
        return NULL;
    }

    for ( int i = 0 ; i < json_object_array_length( additionalDownloads_o ) ; i++ ) {
        json_object *download_o = json_object_array_get_idx( additionalDownloads_o, i );
        json_object *format_o = json_object_object_get( download_o, "format" );
        const char *format = json_object_get_string( format_o );

        if ( strcmp( format, "Pokes (POK)" ) == 0 ) {

            json_object *poke_path_o = 
                json_object_object_get( download_o, "path" );
            const char *poke_path = json_object_get_string( poke_path_o );

            json_object *poke_size_o = 
                json_object_object_get( download_o, "size" );
            int poke_size = json_object_get_int( poke_size_o );

            /** Extract the root filename from the archive path */
            char filename[256] = { 0 };
            getBasename( poke_path, filename );

            /**
             * Strip /pub/sinclair as the archive.org paths differ from the 
             * original WoS ones...
             */
            char fixedPath[256] = { 0 };
            fixupWoSPath( poke_path, fixedPath );

            /** Synthesise the full local FS path to the file */
            char fsPath[256] = { 0 };
            sprintf( fsPath, "%s/POKES/%s", path, filename );

            /** Store in the dir entry for the gameidcache */
            FSCacheEntry_t *file_fscache = FSCacheEntry_create( fsPath, FSCACHEENTRY_FILE, fixedPath, poke_size );
            FSCacheEntry_addFile( pokesDirEntry, file_fscache );
        }
    }

    int nfiles = FSCacheEntry_getnfiles( pokesDirEntry );
    if ( nfiles > 0 ) {
        FSCacheEntry_addFile( dirEntry, pokesDirEntry );
            //json_object_object_add( pokesdir, "files", pokesdirfiles );
            //json_object_object_add( fscache, filepath, pokesdir );
            //_dumpJSON( fscache );
    }

    /**
     * Handle the screenshot files
     */
    json_object *screens_o = 
        json_object_object_get( source_o, "screens" );
    if ( screens_o == NULL ) {
        return NULL;
    }

    char screensPath[256];
    sprintf( screensPath, "%s/SCRSHOT", path );
    FSCacheEntry_t *screensDirEntry = 
        FSCacheEntry_create( screensPath, FSCACHEENTRY_DIR, NULL, 0 );
    if ( screensDirEntry == NULL ) {
        return NULL;
    }

    for ( int i = 0 ; i < json_object_array_length( screens_o ) ; i++ ) {
        json_object *download_o = json_object_array_get_idx( screens_o, i );

        json_object *screen_path_o = 
            json_object_object_get( download_o, "url" );
        const char *screen_path = json_object_get_string( screen_path_o );

        json_object *screen_size_o = 
            json_object_object_get( download_o, "size" );
        int poke_size = json_object_get_int( screen_size_o );

        /** Extract the root filename from the archive path */
        char filename[256] = { 0 };
        getBasename( screen_path, filename );

        /**
         * Strip /pub/sinclair as the archive.org paths differ from the 
         * original WoS ones...
         */
        char fixedPath[256] = { 0 };
        fixupWoSPath( screen_path, fixedPath );

        /** Synthesise the full local FS path to the file */
        char fsPath[256] = { 0 };
        sprintf( fsPath, "%s/SCRSHOT/%s", path, filename );

        /** Store in the dir entry for the gameidcache */
        FSCacheEntry_t *file_fscache = FSCacheEntry_create( fsPath, FSCACHEENTRY_FILE, fixedPath, poke_size );
        FSCacheEntry_addFile( screensDirEntry, file_fscache );
    }

    nfiles = FSCacheEntry_getnfiles( screensDirEntry );
    if ( nfiles > 0 ) {
        FSCacheEntry_addFile( dirEntry, screensDirEntry );
            //json_object_object_add( pokesdir, "files", pokesdirfiles );
            //json_object_object_add( fscache, filepath, pokesdir );
            //_dumpJSON( fscache );
    }

    return dirEntry;
}

/**
 * Read game info to the cache. The path will either be:
 * /by-letter/[A-Z]/[Game Title + ID]
 * /by-letter/[A-Z]/[Game Title + ID]/POKES/*
 * /by-letter/[A-Z]/[Game Title + ID]/SCRSHOT/*
 * So, we want the bit between the third and fourth / (assuming 4 exists)
 */
FSCacheEntry_t *FSCacheEntry_getAndCreateGame( json_object *urlcache,
                                               const char *filepath,
                                               const char *urlhost,
                                               const char *urlpath,
                                               int forceFetch ) {

    //FSCacheEntry_t *gameRoot = NULL;

    char title[128] = { 0 };
    char id[16] = { 0 };
    char gamerootpath[128] = { 0 };

    int rv = 0;
    
    rv = getTitleAndIDFromPath( filepath, title, id, gamerootpath );
    if ( rv == 1 ) {
        return NULL; 
    }

    /** We haven't retrieved this before (or we don't have a cache...) */
    /** Retrieve the data for the game ID */
    char *url = NULL;
    if ( strncmp( urlhost, "file://", 7 ) == 0 ) {
        url = strdup( urlpath );
    } else {
        url = getGameURLPath( id );
    }

    if ( url == NULL ) {
        return NULL;
    }

    printf( "URL: %s\n", url );
    printf( "FS Game Root: %s\n", gamerootpath );

    json_object *gameData = getURL( urlcache, urlhost, url );
    if ( gameData == NULL ) {
        printf( ">>> FAILED TO RETRIEVE JSON OBJECT\n" );
        return NULL;
    } else {
        printf( ">>> gameData is OK\n" );
    }

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_createFromGame( gamerootpath, gameData );
    if ( fsCacheEntry == NULL ) {
        if ( urlcache != NULL ) {
//            json_object_put( gameData );
        }
        return NULL;
    }

    return fsCacheEntry;
}
