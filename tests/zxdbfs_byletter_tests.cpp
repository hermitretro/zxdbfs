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
#include <zxdbfs_http.h>
}

TEST(zxdbfs_byletter_tests, test_FSCacheEntry_createFromByLetter) {

#include <testdata/by-letter-X.h>

    json_object *jsonObject = json_tokener_parse( jsonData );
    ASSERT_TRUE( NULL != jsonObject );

    FSCacheEntry_t *byLetter = 
        FSCacheEntry_createFromByLetter( "/by-letter/X", jsonObject );
    ASSERT_TRUE( NULL != byLetter );
    ASSERT_EQ( FSCACHEENTRY_DIR, FSCacheEntry_gettype( byLetter ) );
    ASSERT_STREQ( "/by-letter/X", FSCacheEntry_getfname( byLetter ) );
    ASSERT_EQ( 115, FSCacheEntry_getnfiles( byLetter ) );

    json_object_put( jsonObject );
    FSCacheEntry_free( byLetter );
}
