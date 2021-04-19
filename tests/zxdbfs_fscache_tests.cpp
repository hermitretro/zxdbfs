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
#include <zxdbfs_byletter.h>
#include <zxdbfs_fscache.h>
#include <zxdbfs_gameid.h>
#include <zxdbfs_json.h>
}

TEST(zxdbfs_fscache_tests, test_FSCache_create) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_free) {

    ASSERT_EQ( 1, FSCache_free( NULL ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_flush) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_create( "/path0/path1", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != fsCacheEntry );

    ASSERT_EQ( 0, FSCache_add( cache, "key", fsCacheEntry ) );

    ASSERT_EQ( 1, json_object_object_length( cache->cache ) );

    ASSERT_EQ( 0, FSCache_flush( cache ) );
    ASSERT_EQ( 0, json_object_object_length( cache->cache ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_add) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    /** Test broken cases */
    ASSERT_EQ( 1, FSCache_add( NULL, NULL, NULL ) );
    ASSERT_EQ( 1, FSCache_add( cache, NULL, NULL ) );
    ASSERT_EQ( 1, FSCache_add( NULL, "key", NULL ) );
    ASSERT_EQ( 1, FSCache_add( cache, "key", NULL ) );

    dumpJSON( cache->cache );

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_create( "/path0/path1", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != fsCacheEntry );

    ASSERT_EQ( 0, FSCache_add( cache, "key", fsCacheEntry ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_get_dir) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_create( "/path0/path1", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != fsCacheEntry );

    ASSERT_EQ( 0, FSCache_add( cache, "key", fsCacheEntry ) );

    FSCacheEntry_t *newFSCacheEntry = FSCache_get( cache, "key" );
    ASSERT_TRUE( newFSCacheEntry != NULL );
    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( newFSCacheEntry ) );
    ASSERT_STREQ( "/path0/path1", FSCacheEntry_getfname( newFSCacheEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_getsize( newFSCacheEntry ) );
    const char *url = FSCacheEntry_geturl( newFSCacheEntry );
    ASSERT_TRUE( NULL == url );
    ASSERT_EQ( 0, FSCacheEntry_getnfiles( newFSCacheEntry ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_addAll) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    /** Test broken cases */
    ASSERT_EQ( 1, FSCache_addAll( NULL, NULL, NULL ) );
    ASSERT_EQ( 1, FSCache_addAll( cache, NULL, NULL ) );
    ASSERT_EQ( 1, FSCache_addAll( NULL, "key", NULL ) );
    ASSERT_EQ( 1, FSCache_addAll( cache, "key", NULL ) );

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_create( "/path0/path1", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != fsCacheEntry );

    ASSERT_EQ( 0, FSCache_addAll( cache, "key", fsCacheEntry ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_get_file) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_create( "/path0/path1", FSCACHEENTRY_FILE, "https://testhost/testpath", 1234 );
    ASSERT_TRUE( NULL != fsCacheEntry );

    ASSERT_EQ( 0, FSCache_add( cache, "key", fsCacheEntry ) );

    FSCacheEntry_t *newFSCacheEntry = FSCache_get( cache, "key" );
    ASSERT_TRUE( newFSCacheEntry != NULL );
    ASSERT_EQ( FSCACHEENTRY_FILE, FSCacheEntry_gettype( newFSCacheEntry ) );
    ASSERT_STREQ( "/path0/path1", FSCacheEntry_getfname( newFSCacheEntry ) );
    ASSERT_EQ( 1234, FSCacheEntry_getsize( newFSCacheEntry ) );
    ASSERT_STREQ( "https://testhost/testpath", FSCacheEntry_geturl( newFSCacheEntry)  );
    ASSERT_EQ( 0, FSCacheEntry_getnfiles( newFSCacheEntry ) );
    //ASSERT_TRUE( NULL == newFSCacheEntry->files );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_get_dir_with_files_and_dirs) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    FSCacheEntry_t *dirEntry = FSCacheEntry_create( "/path0/path1", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != dirEntry );
    FSCacheEntry_t *fileEntry = FSCacheEntry_create( "/path0/path1/path2", FSCACHEENTRY_FILE, "https://testhost/testpath", 1234 );
    ASSERT_TRUE( NULL != fileEntry );
    FSCacheEntry_t *dirEntry2 = FSCacheEntry_create( "/path0/path1/path10", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != dirEntry2 );
    FSCacheEntry_t *fileEntry2 = FSCacheEntry_create( "/path0/path1/path10/path2", FSCACHEENTRY_FILE, "https://testhost2/testpath2", 5678 );
    ASSERT_TRUE( NULL != fileEntry2 );

    ASSERT_EQ( 0, FSCacheEntry_addFile( dirEntry, fileEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_addFile( dirEntry2, fileEntry2 ) );
    ASSERT_EQ( 0, FSCacheEntry_addFile( dirEntry, dirEntry2 ) );

    ASSERT_EQ( 0, FSCache_add( cache, "key", dirEntry ) );

    FSCacheEntry_t *newFSCacheEntry = FSCache_get( cache, "key" );
    ASSERT_TRUE( newFSCacheEntry != NULL );
    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( newFSCacheEntry ) );
    ASSERT_STREQ( "/path0/path1", FSCacheEntry_getfname( newFSCacheEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_getsize( newFSCacheEntry ) );
    ASSERT_EQ( 2, FSCacheEntry_getnfiles( newFSCacheEntry ) );
    //ASSERT_TRUE( NULL != newFSCacheEntry->files );

    //dumpJSON( cache->cache );

    /** Test the subfile */
    FSCacheEntry_t *file0 = FSCacheEntry_getfile( newFSCacheEntry, 0 );
    ASSERT_TRUE( NULL != file0 );
    ASSERT_EQ( FSCACHEENTRY_FILE, FSCacheEntry_gettype( file0 ) );
    ASSERT_STREQ( "/path0/path1/path2", FSCacheEntry_getfname( file0 ) );
    ASSERT_EQ( 1234, FSCacheEntry_getsize( file0 ) );
    ASSERT_STREQ( "https://testhost/testpath", FSCacheEntry_geturl( file0 ) );

    /** Test the subdir */
    FSCacheEntry_t *file1 = FSCacheEntry_getfile( newFSCacheEntry, 1 );
    ASSERT_TRUE( NULL != file1 );
    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( file1 ) );
    ASSERT_STREQ( "/path0/path1/path10", FSCacheEntry_getfname( file1 ) );
    ASSERT_EQ( 0, FSCacheEntry_getsize( file1 ) );
    ASSERT_EQ( 1, FSCacheEntry_getnfiles( file1 ) );

    /** Test the subdir subfile */
    FSCacheEntry_t *file1file0 = FSCacheEntry_getfile( file1, 0 );
    ASSERT_TRUE( NULL != file1file0 );
    ASSERT_STREQ( "/path0/path1/path10/path2", FSCacheEntry_getfname( file1file0 ) );
    ASSERT_EQ( FSCACHEENTRY_FILE, FSCacheEntry_gettype( file1file0 ) );
    ASSERT_EQ( 5678, FSCacheEntry_getsize( file1file0 ) );
    ASSERT_STREQ( "https://testhost2/testpath2", FSCacheEntry_geturl( file1file0 ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_get_dir_with_files) {

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    FSCacheEntry_t *dirEntry = FSCacheEntry_create( "/path0/path1", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != dirEntry );
    FSCacheEntry_t *fileEntry = FSCacheEntry_create( "/path0/path1/path2", FSCACHEENTRY_FILE, "https://testhost/testpath", 1234 );
    ASSERT_TRUE( NULL != fileEntry );

    ASSERT_EQ( 0, FSCacheEntry_addFile( dirEntry, fileEntry ) );

    ASSERT_EQ( 0, FSCache_add( cache, "key", dirEntry ) );

    FSCacheEntry_t *newFSCacheEntry = FSCache_get( cache, "key" );
    ASSERT_TRUE( newFSCacheEntry != NULL );
    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( newFSCacheEntry ) );
    ASSERT_STREQ( "/path0/path1", FSCacheEntry_getfname( newFSCacheEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_getsize( newFSCacheEntry ) );
    ASSERT_EQ( 1, FSCacheEntry_getnfiles( newFSCacheEntry ) );
    //ASSERT_TRUE( NULL != newFSCacheEntry->files );

    /** Test the subfile */
    FSCacheEntry_t *file0 = FSCacheEntry_getfile( newFSCacheEntry, 0 );
    ASSERT_TRUE( file0 != NULL );
    ASSERT_EQ( FSCACHEENTRY_FILE, FSCacheEntry_gettype( file0 ) );
    ASSERT_STREQ( "/path0/path1/path2", FSCacheEntry_getfname( file0 ) );
    ASSERT_EQ( 1234, FSCacheEntry_getsize( file0 ) );
    ASSERT_STREQ( "https://testhost/testpath", FSCacheEntry_geturl( file0 ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_addAll_AZ) {
    
#include <testdata/by-letter-X.h>

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    json_object *jsonObject = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != jsonObject );

    FSCacheEntry_t *byLetter =
        FSCacheEntry_createFromByLetter( "/by-letter/X", jsonObject );
    json_object_put( jsonObject );
    ASSERT_TRUE( NULL != byLetter );
    ASSERT_EQ( 115, FSCacheEntry_getnfiles( byLetter ) );

    ASSERT_EQ( 0, FSCache_addAll( cache, "root", byLetter ) );
    ASSERT_EQ( 116, json_object_object_length( cache->cache ) );
    FSCacheEntry_t *cachedAZ = FSCache_get( cache, "root" );
    ASSERT_TRUE( NULL != cachedAZ );
    ASSERT_EQ( 115, FSCacheEntry_getnfiles( cachedAZ ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}

TEST(zxdbfs_fscache_tests, test_FSCache_addAll_Xevious) {
    
#include <testdata/zxdb-games-0005795.h>

    FSCache_t *cache = FSCache_create();
    ASSERT_TRUE( NULL != cache );

    json_object *jsonObject = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != jsonObject );

    FSCacheEntry_t *gameRoot =
        FSCacheEntry_createFromGame( "/by-letter/X/Xevious_0005795", jsonObject );
    json_object_put( jsonObject );
    ASSERT_TRUE( NULL != gameRoot );
    ASSERT_EQ( 7, FSCacheEntry_getnfiles( gameRoot ) );

    ASSERT_EQ( 0, FSCache_addAll( cache, "/by-letter/X/Xevious_0005795", gameRoot ) );
    ASSERT_EQ( 11, json_object_object_length( cache->cache ) );

    ASSERT_EQ( 0, FSCache_free( cache ) );
}
