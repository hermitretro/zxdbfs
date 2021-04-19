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
#include <zxdbfs_fscacheentry.h>
}

TEST(zxdbfs_fscacheentry_tests, test_FSCacheEntry_create) {

    FSCacheEntry_t *fsCacheEntry = FSCacheEntry_create( NULL, FSCACHEENTRY_UNKNOWN, NULL, 0 );
    ASSERT_EQ( NULL, fsCacheEntry );

    fsCacheEntry = FSCacheEntry_create( "fname", FSCACHEENTRY_UNKNOWN, NULL, 0 );
    ASSERT_EQ( NULL, fsCacheEntry ); 

    fsCacheEntry = FSCacheEntry_create( NULL, (FSCacheEntryType)123123, NULL, 0 );
    ASSERT_EQ( NULL, fsCacheEntry ); 

    fsCacheEntry = FSCacheEntry_create( "fname", (FSCacheEntryType)123123, NULL, 0 );
    ASSERT_EQ( NULL, fsCacheEntry ); 

    fsCacheEntry = FSCacheEntry_create( "dirname", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != fsCacheEntry ); 
    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "dirname", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_TRUE( NULL == FSCacheEntry_geturl( fsCacheEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_getsize( fsCacheEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_getnfiles( fsCacheEntry ) );
    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;

    fsCacheEntry = FSCacheEntry_create( "filename", FSCACHEENTRY_FILE, "https://testhost/testpath", 1234 );
    ASSERT_TRUE( NULL != fsCacheEntry ); 
    ASSERT_EQ( FSCACHEENTRY_FILE, FSCacheEntry_gettype( fsCacheEntry ) );
    ASSERT_STREQ( "filename", FSCacheEntry_getfname( fsCacheEntry ) );
    ASSERT_STREQ( "https://testhost/testpath", FSCacheEntry_geturl( fsCacheEntry ) );
    ASSERT_EQ( 1234, FSCacheEntry_getsize( fsCacheEntry ) );
    ASSERT_EQ( 0, FSCacheEntry_getnfiles( fsCacheEntry ) );
    FSCacheEntry_free( fsCacheEntry );
    fsCacheEntry = NULL;
}

TEST(zxdbfs_fscacheentry_tests, test_FSCacheEntry_addFile) {

    FSCacheEntry_t *dirEntry = FSCacheEntry_create( "dirname", FSCACHEENTRY_DIR, NULL, 0 );
    ASSERT_TRUE( NULL != dirEntry ); 

    /** Start off NULL */
    ASSERT_EQ( 0, FSCacheEntry_getnfiles( dirEntry ) );

    FSCacheEntry_t *fileEntry = FSCacheEntry_create( "filename", FSCACHEENTRY_FILE, "https://testhost/testpath", 1234 );
    ASSERT_TRUE( NULL != fileEntry );

    int rv = FSCacheEntry_addFile( dirEntry, fileEntry );
    ASSERT_EQ( 0, rv );

    ASSERT_EQ( 1, FSCacheEntry_getnfiles( dirEntry ) );

    FSCacheEntry_free( dirEntry );
}

TEST(zxdbfs_fscacheentry_tests, test_FSCacheEntry_isValidType) {

    ASSERT_EQ( 0, FSCacheEntry_isValidType( FSCACHEENTRY_FILE ) );
    ASSERT_EQ( 0, FSCacheEntry_isValidType( FSCACHEENTRY_DIR ) );
    ASSERT_EQ( 1, FSCacheEntry_isValidType( FSCACHEENTRY_UNKNOWN ) );
    ASSERT_EQ( 1, FSCacheEntry_isValidType( (FSCacheEntryType)10 ) );
}

