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

#include <gtest/gtest.h>

extern "C" {
#include <zxdbfs_json.h>
#include <zxdbfs_search.h>
}

#include "zxdbfs_tests_utils.h"

TEST(zxdbfs_search_tests, test_FSCacheEntry_createFromSearch_NULL) {
    
    /** Check degenerate case */
    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_createFromSearch( NULL, NULL, 0, NULL );
    ASSERT_TRUE( NULL == fsCacheEntry );
    FSCacheEntry_free( fsCacheEntry );
}

TEST(zxdbfs_search_tests, test_FSCacheEntry_createFromSearch_Hewson) {

#include <testdata/search-Hewson.h>
    json_object *searchData = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != searchData );

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_createFromSearch( "/search/Hewson", searchData, 0, NULL );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( searchData );

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/Hewson", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 256, FSCacheEntry_getnfiles( fsCacheEntry ) );

    for ( int i = 0 ; i < FSCacheEntry_getnfiles( fsCacheEntry ) ; i++ ) {
        FSCacheEntry_t *subentry = FSCacheEntry_getfile( fsCacheEntry, i );
        ASSERT_EQ( FSCACHEENTRY_DIR_STUB, FSCacheEntry_gettype( subentry ) );
    }

#ifdef PANTS
    /** Test the files... */
    const char *names[64] = {
        "/by-letter/X/Xevious_0005795/Xevious.tap.zip",
        "/by-letter/X/Xevious_0005795/Xevious.tzx.zip",
        "/by-letter/X/Xevious_0005795/Xevious(AmericanaSoftwareLtd).tzx.zip",
        "/by-letter/X/Xevious_0005795/Xevious(ErbeSoftwareS.A.).tzx.zip",
        "/by-letter/X/Xevious_0005795/Xevious(DroSoft).tzx.zip"
    };
    const char *urls[64] = {
        "/games/x/Xevious.tap.zip",
        "/games/x/Xevious.tzx.zip",
        "/games/x/Xevious(AmericanaSoftwareLtd).tzx.zip",
        "/games/x/Xevious(ErbeSoftwareS.A.).tzx.zip",
        "/games/x/Xevious(DroSoft).tzx.zip"
    };
    int sizes[] = {
        18492,
        41485,
        41277,
        42209,
        14000
    };

    for ( int i = 0 ; i < 5 ; i++ ) {

        FSCacheEntry_t *filex = FSCacheEntry_getfile( fsCacheEntry, i );
        ASSERT_TRUE( NULL != filex );

        ASSERT_EQ( FSCACHEENTRY_FILE, FSCacheEntry_gettype( filex ) );
        ASSERT_STREQ( names[i], FSCacheEntry_getfname( filex ) );
        ASSERT_EQ( sizes[i], FSCacheEntry_getsize( filex ) );
        ASSERT_EQ( 0, FSCacheEntry_getnfiles( filex ) );
    }

    /** Test the dirs */
    const char *dirnames[64] = {
        "/by-letter/X/Xevious_0005795/POKES",
        "/by-letter/X/Xevious_0005795/SCRSHOT"
    };
    const int dirsizes[] = {
        1,
        2
    };
    for ( int i = 5 ; i < FSCacheEntry_getnfiles( fsCacheEntry ) ; i++ ) {

        FSCacheEntry_t *filex = FSCacheEntry_getfile( fsCacheEntry, i );
        ASSERT_TRUE( NULL != filex );

        ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( filex ) );
        ASSERT_STREQ( dirnames[i - 5], FSCacheEntry_getfname( filex ) );
        ASSERT_EQ( 0, FSCacheEntry_getsize( filex ) );
        ASSERT_EQ( dirsizes[i - 5], FSCacheEntry_getnfiles( filex ) );

        switch ( i ) {
            case 5: {
                FSCacheEntry_t *filexfile0 = FSCacheEntry_getfile( filex, 0 );
                ASSERT_TRUE( NULL != filexfile0 );
                ASSERT_STREQ( "/by-letter/X/Xevious_0005795/POKES/Xevious (1987)(US Gold).pok", FSCacheEntry_getfname( filexfile0 ) );
                ASSERT_EQ( 116, FSCacheEntry_getsize( filexfile0 ) );
                break;
            }
            case 6: {
                FSCacheEntry_t *filexfile0 = FSCacheEntry_getfile( filex, 0 );
                ASSERT_TRUE( NULL != filexfile0 );
                ASSERT_STREQ( "/by-letter/X/Xevious_0005795/SCRSHOT/Xevious-load.png", FSCacheEntry_getfname( filexfile0 ) );
                ASSERT_EQ( 6186, FSCacheEntry_getsize( filexfile0 ) );

                FSCacheEntry_t *filexfile1 = FSCacheEntry_getfile( filex, 1 );
                ASSERT_TRUE( NULL != filexfile1 );
                ASSERT_STREQ( "/by-letter/X/Xevious_0005795/SCRSHOT/Xevious.gif", FSCacheEntry_getfname( filexfile1 ) );
                ASSERT_EQ( 3499, FSCacheEntry_getsize( filexfile1 ) );
                break;
            }
            default: {
                break;
            }
        }
    }
#endif

    FSCacheEntry_free( fsCacheEntry );
}

TEST(zxdbfs_search_tests, test_FSCacheEntry_createFromSearch_Hewson_filter) {

#include <testdata/search-Hewson.h>
    json_object *searchData = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != searchData );

    /** Filter out everything by score */
    json_object *cloneSearchData = NULL;
    FSCacheEntry_t *fsCacheEntry = NULL;

    json_object_deep_copy( searchData, &cloneSearchData, NULL );
    fsCacheEntry = FSCacheEntry_createFromSearch( "/search/Hewson", cloneSearchData, 20, NULL );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( cloneSearchData );
    cloneSearchData = NULL;

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/Hewson", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_getnfiles( fsCacheEntry ) );

    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    /** Filter out some rubbish by score */
    json_object_deep_copy( searchData, &cloneSearchData, NULL );
    fsCacheEntry = FSCacheEntry_createFromSearch( "/search/Hewson", cloneSearchData, 10, NULL );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( cloneSearchData );
    cloneSearchData = NULL;

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/Hewson", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 55, FSCacheEntry_getnfiles( fsCacheEntry ) );

    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    /** Filter out some rubbish by actual search term */
    json_object_deep_copy( searchData, &cloneSearchData, NULL );
    fsCacheEntry = FSCacheEntry_createFromSearch( "/search/Hewson", cloneSearchData, 0, "Hewson" );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( cloneSearchData );
    cloneSearchData = NULL;

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/Hewson", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 50, FSCacheEntry_getnfiles( fsCacheEntry ) );

    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    /** Final cleanup */
    json_object_put( searchData );
}

TEST(zxdbfs_search_tests, test_FSCacheEntry_createFromSearch_Zynaps_filter) {

#include <testdata/search-Zynaps.h>
    json_object *searchData = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != searchData );

    json_object *cloneSearchData = NULL;
    FSCacheEntry_t *fsCacheEntry = NULL;

    /** Filter out some rubbish by actual search term */
    json_object_deep_copy( searchData, &cloneSearchData, NULL );
    fsCacheEntry = FSCacheEntry_createFromSearch( "/search/Zynaps", cloneSearchData, 0, "Zynaps" );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( cloneSearchData );
    cloneSearchData = NULL;

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/Zynaps", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 1, FSCacheEntry_getnfiles( fsCacheEntry ) );

    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    /** Case-insensitive test... */
    json_object_deep_copy( searchData, &cloneSearchData, NULL );
    fsCacheEntry = FSCacheEntry_createFromSearch( "/search/zynaps", cloneSearchData, 0, "zynaps" );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( cloneSearchData );
    cloneSearchData = NULL;

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/zynaps", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 1, FSCacheEntry_getnfiles( fsCacheEntry ) );

    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    /** Final cleanup */
    json_object_put( searchData );
}

TEST(zxdbfs_search_tests, test_FSCacheEntry_createFromSearch_Uridium_filter) {

#include <testdata/search-Uridium.h>
    json_object *searchData = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != searchData );

    json_object *cloneSearchData = NULL;
    FSCacheEntry_t *fsCacheEntry = NULL;

    /** Filter out some rubbish by actual search term */
    json_object_deep_copy( searchData, &cloneSearchData, NULL );
    fsCacheEntry = FSCacheEntry_createFromSearch( "/search/Uridium", cloneSearchData, 0, "Uridium" );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( cloneSearchData );
    cloneSearchData = NULL;

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/Uridium", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 3, FSCacheEntry_getnfiles( fsCacheEntry ) );

    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    /** Case-insensitive test... */
    json_object_deep_copy( searchData, &cloneSearchData, NULL );
    fsCacheEntry = FSCacheEntry_createFromSearch( "/search/uridium", cloneSearchData, 0, "uridium" );
    ASSERT_TRUE( NULL != fsCacheEntry );
    json_object_put( cloneSearchData );
    cloneSearchData = NULL;

    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "/search/uridium", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_EQ( 3, FSCacheEntry_getnfiles( fsCacheEntry ) );

    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    /** Final cleanup */
    json_object_put( searchData );
}

#ifdef PANTS

TEST(zxdbfs_gameid_tests, test_getAndCreateGame) {

#include <testdata/zxdb-games-0005795.h>

    const char *path0 = "";
    FSCacheEntry_t *rv0 = FSCacheEntry_getAndCreateGame( NULL, "/path0", "file://", path0, 0 );
    ASSERT_TRUE( NULL == rv0 );

    /** Copy the JSON data to /tmp */
    int pid = getpid();
    char fname[128];
    sprintf( fname, "/tmp/%d.json", pid );

    int rv = createTestFile( fname, jsonData ); 
    ASSERT_EQ( 0, rv );

    /** Test with the direct path */
    const char *path1 = "/by-letter/X/Xevious_0005795";
    FSCacheEntry_t *rv1 = FSCacheEntry_getAndCreateGame( NULL, path1, "file://", fname, 0 );
    ASSERT_TRUE( NULL != rv1 );
    ASSERT_STREQ( path1, rv1->fname );
    FSCacheEntry_free( rv1 );

    /** Test with a subdir path -- should equal the direct path */
    const char *path2 = "/by-letter/X/Xevious_0005795/POKES/poke1.pok";
    FSCacheEntry_t *rv2 = FSCacheEntry_getAndCreateGame( NULL, path2, "file://", fname, 0 );
    ASSERT_TRUE( NULL != rv2 );
    ASSERT_STREQ( path1, rv2->fname );
    FSCacheEntry_free( rv2 );

    ASSERT_EQ( 0, unlinkTestFile( fname ) );
}

#endif

