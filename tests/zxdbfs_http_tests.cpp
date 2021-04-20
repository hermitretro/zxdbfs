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
#include <zxdbfs_fscache.h>
#include <zxdbfs_http.h>
#include <zxdbfs_json.h>
}

#include "zxdbfs_tests_utils.h"

TEST(zxdbfs_http_tests, test_getURL_error) {

    /** Bad parameters */
    json_object *rv0 = getURL( NULL, NULL, NULL, NULL );
    ASSERT_TRUE( NULL == rv0 );

    json_object *rv1 = getURL( NULL, "file://", NULL, NULL );
    ASSERT_TRUE( NULL == rv1 );

    json_object *rv2 = getURL( NULL, NULL, "/path", NULL );
    ASSERT_TRUE( NULL == rv2 );

    /** File not found */
    json_object *rv3 = getURL( NULL, "file://", "/tmp/doesnotexist.json", NULL );
    ASSERT_TRUE( NULL == rv3 );

}

TEST(zxdbfs_http_tests, test_getURL_uncached) {

#include <testdata/zxdb-games-0005795.h>

    int pid = getpid();
    char fname[128];
    sprintf( fname, "/tmp/%d.json", pid );

    int rv = createTestFile( fname, jsonData ); 
    ASSERT_EQ( 0, rv );

    json_object *rv0 = getURL( NULL, "file://", fname, NULL );
    ASSERT_TRUE( NULL != rv0 );

    ASSERT_EQ( 0, unlinkTestFile( fname ) );

    json_object_put( rv0 );
}

TEST(zxdbfs_http_tests, test_getURL_cached) {

#include <testdata/zxdb-games-0005795.h>

    json_object *urlcache = json_object_new_object();
    ASSERT_EQ( 0, json_object_object_length( urlcache ) );

    int pid = getpid();
    char fname[128];
    sprintf( fname, "/tmp/%d.json", pid );

    int rv = createTestFile( fname, jsonData ); 
    ASSERT_EQ( 0, rv );

    json_object *rv0 = getURL( urlcache, "file://", fname, NULL );
    ASSERT_TRUE( NULL != rv0 );
    json_object_put( rv0 );

    ASSERT_EQ( 0, unlinkTestFile( fname ) );

    /** Should have one entry in the URL cache... */
    ASSERT_EQ( 1, json_object_object_length( urlcache ) );

    /** Refetch from the cache */
    json_object *cachedata = getURL( urlcache, "file://", fname, NULL );
    ASSERT_TRUE( NULL != cachedata );
    json_object_put( cachedata );

    json_object_put( urlcache );
}
