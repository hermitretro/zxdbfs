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

#include <curl/curl.h>

#include "zxdbfs_fscache.h"
#include "zxdbfs_http.h"
#include "zxdbfs_json.h"

int HTTP_TO_OSCODE( int res ) {
    switch ( res ) {
        case 200: {
            return 0;
        }
        case 400: {
            return -EINTR;
        }
        case 404: {
            return -ENOENT;
        }
        default: {
            return -ENOENT;
        }
    }

    return ENOENT;
}

/**
 * Return a JSON object from cURL
 */
static size_t
write_data(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        printf( "not enough memory (realloc returned NULL)\n" );
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

struct MemoryStruct *getURLViacURL( const char *host, const char *path, const char *useragent ) {

    json_object *rv = NULL;

    CURL *curl;
    CURLcode res;

    struct MemoryStruct *chunk = NULL;

    curl = curl_easy_init();
    if ( curl ) {

        chunk = (struct MemoryStruct *)malloc( sizeof( struct MemoryStruct ) );

        chunk->memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
        chunk->size = 0;    /* no data at this point */

        char fullurl[1024];
        sprintf( fullurl, "%s%s", host, path );
        printf( "fullurl: %s\n", fullurl );

        char luseragent[128];
        if ( useragent != NULL ) {
            snprintf( luseragent, sizeof( luseragent ), "User-Agent: %s", useragent );
        } else {
            sprintf( luseragent, "User-Agent: zxdbfs" );
        }

        struct curl_slist *headers = NULL; // init to NULL is important
        headers = curl_slist_append( headers, luseragent );
        headers = curl_slist_append( headers, "charsets: utf-8" );
        headers = curl_slist_append( headers, "Accept: text/html,application/xhtml+xml,application/xml,application/json,application/zip;q=0.9,image/webp,*/*;q=0.8" );

        curl_easy_setopt( curl, CURLOPT_URL, fullurl );
        curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt( curl, CURLOPT_VERBOSE, 1 );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write_data );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)chunk );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );
        curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L );

        /** This needs to be outside of the SSL test as we might be following a redirect.... */
        if ( 1 ) {
            curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 1L );
            curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 2L );
        } else {
            curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0L );
            curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 0L );
        }

        if ( 1 ) {
            curl_easy_setopt( curl, CURLOPT_USE_SSL, CURLUSESSL_ALL );
        }

        /* Perform the request, res will get the return code */
        res = curl_easy_perform( curl );
        /* Check for errors */
        if ( res != CURLE_OK ) {
            printf( "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
            goto curl_cleanup;
        }

        /** Content length */
        double ct;
        res = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &ct);
        if ( res != CURLE_OK ) {
            printf( "curl_easy_getinfo() failed: %s\n", curl_easy_strerror( res ) );
            goto curl_cleanup;
        } else {
            printf( "CURL: Content-Length: %lf\n", ct );
        }

curl_cleanup:
        curl_slist_free_all( headers );
        curl_easy_cleanup( curl );

        if ( chunk != NULL ) {
            printf( "chunk.size: %ld\n", chunk->size );
            if ( res != CURLE_OK ) {
                if ( chunk->memory != NULL ) {
                    free( chunk->memory );
                    chunk->memory = NULL;
                }
                free( chunk );
                chunk = NULL;
                return chunk;
            }
        }
    } else {
        printf( "CURL failed to initialise" );
        return NULL;
    }

    return chunk;
}

/**
 * Return a JSON object either from cache or via cURL. In either case,
 * the cache will be updated
 */
json_object *getURL( json_object *urlcache, const char *host, const char *path, const char *useragent ) {

    if ( host == NULL || path == NULL ) {
        return NULL;
    }

    json_object *jsonObject = NULL;

    /** Check the URL cache */
    char cachekey[256];
    sprintf( cachekey, "%s%s", host, path );
    if ( urlcache != NULL ) {
        jsonObject = json_object_object_get( urlcache, cachekey );
    }

    if ( jsonObject != NULL ) {
        printf( ">>> USING CACHE\n" );
        return json_object_get( jsonObject );
    } else {
        printf( ">>> NOT USING CACHE\n" );
        /** Make a call to ZXDB */

        struct MemoryStruct *chunk = getURLViacURL( host, path, useragent );
        if ( chunk == NULL || chunk->memory == NULL ) {
            return NULL;
        }

        jsonObject = json_tokener_parse( chunk->memory );
        if ( !jsonObject ) {
            free( chunk );
            return NULL;
        }

        free( chunk->memory );
        chunk->memory = NULL;
        free( chunk );
        chunk = NULL;

        /** Populate the cache */
        if ( urlcache != NULL ) {
            json_object_object_add( urlcache, cachekey, json_object_get( jsonObject ) );
        }
    }

    return jsonObject;
}

