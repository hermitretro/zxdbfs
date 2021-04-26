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
#include <zxdbfs_status.h>
}

#include "zxdbfs_tests_utils.h"

TEST(zxdbfs_status_tests, test_ZXDBFSStatus_create) {

    ZXDBFSStatus_t *status = ZXDBFSStatus_create();
    ASSERT_TRUE( NULL != status );

    ASSERT_EQ( 0, strlen( status->ssid ) );
    ASSERT_EQ( 0, strlen( status->pairwiseCipher ) );
    ASSERT_EQ( 0, strlen( status->groupCipher ) );
    ASSERT_EQ( 0, strlen( status->keyManagement ) );
    ASSERT_EQ( 0, strlen( status->wpaState ) );
    ASSERT_EQ( 0, status->ntpdok );
    ASSERT_EQ( 0, status->zxdbfsdok );
    ASSERT_EQ( 0, status->spidok );
    ASSERT_EQ( 0, strlen( status->zxdbVersion ) );

    ZXDBFSStatus_free( status );
}

TEST(zxdbfs_status_tests, test_ZXDBFSStatus_NULL) {

    ZXDBFSStatus_t *status = ZXDBFSStatus_createFromJSON( NULL );
    ASSERT_TRUE( NULL == status );
}

TEST(zxdbfs_status_tests, test_ZXDBFSStatus_mixed) {

#include <testdata/status-mixed.h>
    json_object *status_o = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != status_o );

    ZXDBFSStatus_t *status = ZXDBFSStatus_createFromJSON( status_o );
    ASSERT_TRUE( NULL != status );

    ASSERT_STREQ( "12SSID34", status->ssid );
    ASSERT_STREQ( "CCMP", status->pairwiseCipher );
    ASSERT_STREQ( "CCMP", status->groupCipher );
    ASSERT_STREQ( "WPA2-PSK", status->keyManagement );
    ASSERT_STREQ( "COMPLETED", status->wpaState );
    ASSERT_EQ( 0, status->ntpdok );
    ASSERT_EQ( 0, status->zxdbfsdok );
    ASSERT_EQ( 0, status->spidok );
    ASSERT_STREQ( "unknown", status->zxdbVersion );
}

TEST(zxdbfs_status_tests, test_ZXDBFSStatus_good) {

#include <testdata/status-good.h>
    json_object *status_o = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != status_o );

    ZXDBFSStatus_t *status = ZXDBFSStatus_createFromJSON( status_o );
    ASSERT_TRUE( NULL != status );

    ASSERT_STREQ( "12SSID34", status->ssid );
    ASSERT_STREQ( "CCMP", status->pairwiseCipher );
    ASSERT_STREQ( "CCMP", status->groupCipher );
    ASSERT_STREQ( "WPA2-PSK", status->keyManagement );
    ASSERT_STREQ( "COMPLETED", status->wpaState );
    ASSERT_EQ( 1, status->ntpdok );
    ASSERT_EQ( 1, status->zxdbfsdok );
    ASSERT_EQ( 1, status->spidok );
    ASSERT_STREQ( "1.0.89", status->zxdbVersion );
}

#ifdef PANTS

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
#endif
