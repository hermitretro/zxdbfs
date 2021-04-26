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
    ASSERT_EQ( 0, status->dateok );
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
    ASSERT_EQ( 0, status->dateok );
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
    ASSERT_EQ( 1, status->dateok );
    ASSERT_EQ( 1, status->zxdbfsdok );
    ASSERT_EQ( 1, status->spidok );
    ASSERT_STREQ( "1.0.89", status->zxdbVersion );
}
