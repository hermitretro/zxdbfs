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
#include <zxdbfs_paths.h>
}

TEST(zxdbfs_paths_tests, test_countSlashes) {

    int nslashes = 0;

    ASSERT_EQ( -1, countSlashes( NULL, NULL, NULL, 0 ) );

    const char *path0 = "";
    ASSERT_EQ( -1, countSlashes( path0, NULL, NULL, 0 ) );
    ASSERT_EQ( 0, countSlashes( path0, &nslashes, NULL, 0 ) );
    ASSERT_EQ( 0, nslashes );

    const char *path1 = "/test";
    ASSERT_EQ( 0, countSlashes( path1, &nslashes, NULL, 0 ) );
    ASSERT_EQ( 1, nslashes );

    const char *path2 = "/test/";
    ASSERT_EQ( 0, countSlashes( path2, &nslashes, NULL, 0 ) );
    ASSERT_EQ( 2, nslashes );

    const char *path3 = "/test0/test1";
    ASSERT_EQ( 0, countSlashes( path3, &nslashes, NULL, 0 ) );
    ASSERT_EQ( 2, nslashes );

    int slashpos[10];
    const char *path4 = "/test0/test1/test2/test3/test4/test5/";
    ASSERT_EQ( 0, countSlashes( path4, &nslashes, slashpos, 10 ) );
    ASSERT_EQ( 7, nslashes );
    ASSERT_EQ( 0, slashpos[0] );
    ASSERT_EQ( 6, slashpos[1] );
    ASSERT_EQ( 12, slashpos[2] );
    ASSERT_EQ( 18, slashpos[3] );
    ASSERT_EQ( 24, slashpos[4] );
    ASSERT_EQ( 30, slashpos[5] );
    ASSERT_EQ( 36, slashpos[6] );
}

TEST(zxdbfs_paths_tests, test_isAllDigits) {

    ASSERT_EQ( 1, isAllDigits( NULL ) );
    
    const char *path0 = "";
    ASSERT_EQ( 1, isAllDigits( path0 ) );

    const char *path1 = "0123123A";
    ASSERT_EQ( 1, isAllDigits( path1 ) );

    const char *path2 = "_0123456";
    ASSERT_EQ( 1, isAllDigits( path2 ) );

    const char *path3 = "0123456";
    ASSERT_EQ( 0, isAllDigits( path3 ) );
}

void _assertQuazatronTitleAndID( const char *path, const char *rootpath ) {

    char title[128] = { 0 };
    char id[16] = { 0 };
    char gamerootpath[128] = { 0 };

    ASSERT_EQ( 0, getTitleAndIDFromPath( path, title, id, gamerootpath ) );
    ASSERT_STREQ( "Quazatron", title );
    ASSERT_STREQ( "0003972", id );
    ASSERT_STREQ( rootpath, gamerootpath );
}

TEST(zxdbfs_paths_tests, test_getTitleAndIDFromPath) {

    char title[128] = { 0 };
    char id[16] = { 0 };
    char gamerootpath[128] = { 0 };

    const char *path0 = "";
    ASSERT_EQ( -1, getTitleAndIDFromPath( path0, NULL, NULL, NULL ) );

    /** Standalone file */
    const char *path1 = "Quazatron_0003972";
    _assertQuazatronTitleAndID( path1, path1 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path2 = "/Quazatron_0003972";
    _assertQuazatronTitleAndID( path2, path2 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    /** Absolute paths */
    const char *path3 = "/path0/Quazatron_0003972";
    _assertQuazatronTitleAndID( path3, path3 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path4 = "/path0/path1/Quazatron_0003972";
    _assertQuazatronTitleAndID( path4, path4 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path5 = "/path0/path1/path2/Quazatron_0003972";
    _assertQuazatronTitleAndID( path5, path5 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path6 = "/path0/path1/path2/path4/Quazatron_0003972";
    _assertQuazatronTitleAndID( path6, path6 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path7 = "/path0/path1/path2/path4/Quazatron_0003972/path5";
    _assertQuazatronTitleAndID( path7, "/path0/path1/path2/path4/Quazatron_0003972" );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    /** Relative paths */
    const char *path8 = "path0/path1/path2/path4/Quazatron_0003972/path5";
    _assertQuazatronTitleAndID( path8, "path0/path1/path2/path4/Quazatron_0003972" );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path9 = "path0/path1/Quazatron_0003972";
    _assertQuazatronTitleAndID( path9, path9 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path10 = "path0/path1/path2/Quazatron_0003972";
    _assertQuazatronTitleAndID( path10, path10 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path11 = "path0/path1/path2/path4/Quazatron_0003972";
    _assertQuazatronTitleAndID( path11, path11 );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path12 = "path0/path1/path2/path4/Quazatron_0003972/path5";
    _assertQuazatronTitleAndID( path12, "path0/path1/path2/path4/Quazatron_0003972" );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path13 = "path0/path1/Qua_BLAH_PLOP_zatron_0003972";
    ASSERT_EQ( 0, getTitleAndIDFromPath( path13, title, id, gamerootpath ) );
    ASSERT_STREQ( "Qua_BLAH_PLOP_zatron", title );
    ASSERT_STREQ( "0003972", id );
    ASSERT_STREQ( path13, gamerootpath );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path14 = "/path0/path1/Qua_BLAH_PLOP_zatron_0003972/path3";
    ASSERT_EQ( 0, getTitleAndIDFromPath( path14, title, id, gamerootpath ) );
    ASSERT_STREQ( "Qua_BLAH_PLOP_zatron", title );
    ASSERT_STREQ( "0003972", id );
    ASSERT_STREQ( "/path0/path1/Qua_BLAH_PLOP_zatron_0003972", gamerootpath );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    /** Fail tests -- ID is too short */
    const char *path15 = "path0/path1/Qua_BLAH_PLOP_zatron_003972";
    ASSERT_EQ( 1, getTitleAndIDFromPath( path15, title, id, gamerootpath ) );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path16 = "/path0/path1/Qua_BLAH_PLOP_zatron_003972";
    ASSERT_EQ( 1, getTitleAndIDFromPath( path16, title, id, gamerootpath ) );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    const char *path17 = "/path0/path1/Quazatron_003972";
    ASSERT_EQ( 1, getTitleAndIDFromPath( path17, title, id, gamerootpath ) );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );

    /** This failed in a previous test */
    /** It works here, but failed there. Turned into a bad initialisation issue */
    const char *path18 = "/by-letter/X/Xevious_0005795/POKES/poke1.pok";
    ASSERT_EQ( 0, getTitleAndIDFromPath( path18, title, id, gamerootpath ) );
    ASSERT_STREQ( "Xevious", title );
    ASSERT_STREQ( "0005795", id );
    ASSERT_STREQ( "/by-letter/X/Xevious_0005795", gamerootpath );
    memset( title, 0, sizeof( title ) );
    memset( id, 0, sizeof( id ) );
    memset( gamerootpath, 0, sizeof( gamerootpath ) );
}

TEST(zxdbfs_paths_tests, test_getDirname) {

    char dir[256] = { 0 };

    ASSERT_EQ( 1, getDirname( NULL, NULL ) );
    ASSERT_EQ( 1, getDirname( "/", NULL ) );
    ASSERT_EQ( 1, getDirname( NULL, dir ) );

    const char *path0 = "";
    ASSERT_EQ( 1, getDirname( path0, dir ) );
    memset( dir, 0, sizeof( dir ) );

    const char *path1 = "/test";
    ASSERT_EQ( 0, getDirname( path1, dir ) );
    ASSERT_STREQ( "/", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path2 = "/path0/path1";
    ASSERT_EQ( 0, getDirname( path2, dir ) );
    ASSERT_STREQ( "/path0", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path3 = "/";
    ASSERT_EQ( 0, getDirname( path3, dir ) );
    ASSERT_STREQ( "/", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path4 = "/path0/path1/path2/path3";
    ASSERT_EQ( 0, getDirname( path4, dir ) );
    ASSERT_STREQ( "/path0/path1/path2", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path5 = "relative/path";
    ASSERT_EQ( 0, getDirname( path5, dir ) );
    ASSERT_STREQ( "relative", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path6 = "path";
    ASSERT_EQ( 0, getDirname( path6, dir ) );
    ASSERT_STREQ( ".", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path7 = "/path0/path1/";
    ASSERT_EQ( 0, getDirname( path7, dir ) );
    ASSERT_STREQ( "/path0", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path8 = "/path0/";
    ASSERT_EQ( 0, getDirname( path8, dir ) );
    ASSERT_STREQ( "/", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path9 = "/path0//path1";
    ASSERT_EQ( 0, getDirname( path9, dir ) );
    ASSERT_STREQ( "/path0", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path10 = "/path0///////path1";
    ASSERT_EQ( 0, getDirname( path10, dir ) );
    ASSERT_STREQ( "/path0", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path11 = "/path0///path1///test";
    ASSERT_EQ( 0, getDirname( path11, dir ) );
    ASSERT_STREQ( "/path0///path1", dir );
    memset( dir, 0, sizeof( dir ) );
}

TEST(zxdbfs_paths_tests, test_getBasename) {

    char dir[256] = { 0 };

    ASSERT_EQ( 1, getBasename( NULL, NULL ) );
    ASSERT_EQ( 1, getBasename( "/", NULL ) );
    ASSERT_EQ( 1, getBasename( NULL, dir ) );

    const char *path0 = "";
    ASSERT_EQ( 1, getBasename( path0, dir ) );
    memset( dir, 0, sizeof( dir ) );

    const char *path1 = "/test";
    ASSERT_EQ( 0, getBasename( path1, dir ) );
    ASSERT_STREQ( "test", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path2 = "/path0/path1";
    ASSERT_EQ( 0, getBasename( path2, dir ) );
    ASSERT_STREQ( "path1", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path3 = "/";
    ASSERT_EQ( 0, getBasename( path3, dir ) );
    ASSERT_STREQ( "/", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path4 = "/path0/path1/path2/path3";
    ASSERT_EQ( 0, getBasename( path4, dir ) );
    ASSERT_STREQ( "path3", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path5 = "relative/path";
    ASSERT_EQ( 0, getBasename( path5, dir ) );
    ASSERT_STREQ( "path", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path6 = "path";
    ASSERT_EQ( 0, getBasename( path6, dir ) );
    ASSERT_STREQ( "path", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path7 = "/path0/path1/";
    ASSERT_EQ( 0, getBasename( path7, dir ) );
    ASSERT_STREQ( "path1", dir );
    memset( dir, 0, sizeof( dir ) );

    const char *path8 = "/path0////path1//";
    ASSERT_EQ( 0, getBasename( path8, dir ) );
    ASSERT_STREQ( "path1", dir );
    memset( dir, 0, sizeof( dir ) );
}

TEST(zxdbfs_paths_tests, test_fixupWoSPath) {

    char outpath[256] = { 0 };

    ASSERT_EQ( 1, fixupWoSPath( NULL, NULL ) );
    ASSERT_EQ( 1, fixupWoSPath( "path0", NULL ) );
    ASSERT_EQ( 1, fixupWoSPath( NULL, outpath ) );

    ASSERT_EQ( 0, fixupWoSPath( "/path0/path1", outpath ) );
    ASSERT_STREQ( "/path0/path1", outpath );
    memset( outpath, 0, sizeof( outpath ) );

    ASSERT_EQ( 0, fixupWoSPath( "/pub/sinclair/path0/path1", outpath ) );
    ASSERT_STREQ( "/path0/path1", outpath );
    memset( outpath, 0, sizeof( outpath ) );
}

TEST(zxdbfs_paths_tests, test_getRootDownloadURL) {

    const char *path0 = "";
    ASSERT_TRUE( NULL == getRootDownloadURL( path0 ) );

    const char *path1 = "/zxdb/sinclair/testfile";
    char *url1 = getRootDownloadURL( path1 );
    ASSERT_TRUE( NULL != url1 );
    ASSERT_STREQ( "https://spectrumcomputing.co.uk", url1 );
    free( url1 );

    const char *path2 = "/games/testfile";
    char *url2 = getRootDownloadURL( path2 );
    ASSERT_TRUE( NULL != url2 );
    ASSERT_STREQ( "https://archive.org/download/World_of_Spectrum_June_2017_Mirror/World%20of%20Spectrum%20June%202017%20Mirror.zip/World%20of%20Spectrum%20June%202017%20Mirror/sinclair", url2 );
    free( url2 );
}

TEST(zxdbfs_paths_tests, test_getGameURLPath) {

    const char *id0 = "";
    ASSERT_TRUE( NULL == getGameURLPath( id0 ) );

    const char *id1 = "123456";
    ASSERT_TRUE( NULL == getGameURLPath( id1 ) );

    const char *id2 = "123456a";
    ASSERT_TRUE( NULL == getGameURLPath( id2 ) );

    const char *id3 = "1234567";
    char *rv3 = getGameURLPath( id3 );
    ASSERT_TRUE( NULL != rv3 );
    free( rv3 );
}

TEST(zxdbfs_paths_tests, test_getSearchTermFromPath) {

    char searchTerm[128] = { 0 };
    char searchRootPath[256] = { 0 };

    const char *path0 = "";
    ASSERT_EQ( 1, getSearchTermFromPath( path0, NULL, NULL ) );

    const char *path1 = "/search";
    ASSERT_EQ( 1, getSearchTermFromPath( path1, searchTerm, searchRootPath ) );
    ASSERT_EQ( 0, strlen( searchRootPath ) );
    memset( searchTerm, 0, sizeof( searchTerm ) );
    memset( searchRootPath, 0, sizeof( searchRootPath ) );

    const char *path2 = "/search/searchTerm";
    ASSERT_EQ( 0, getSearchTermFromPath( path2, searchTerm, searchRootPath  ) );
    ASSERT_STREQ( "searchTerm", searchTerm );
    ASSERT_EQ( 0, strlen( searchRootPath ) );
    memset( searchTerm, 0, sizeof( searchTerm ) );
    memset( searchRootPath, 0, sizeof( searchRootPath ) );

    const char *path3 = "/search/searchTerm/after_search_term";
    ASSERT_EQ( 0, getSearchTermFromPath( path3, searchTerm, searchRootPath  ) );
    ASSERT_STREQ( "searchTerm", searchTerm );
    ASSERT_STREQ( "/after_search_term", searchRootPath );
    memset( searchTerm, 0, sizeof( searchTerm ) );
    memset( searchRootPath, 0, sizeof( searchRootPath ) );

    const char *path4 = "/search/searchTerm/after_search_term/more_search_term";
    ASSERT_EQ( 0, getSearchTermFromPath( path4, searchTerm, searchRootPath  ) );
    ASSERT_STREQ( "searchTerm", searchTerm );
    ASSERT_STREQ( "/after_search_term/more_search_term", searchRootPath );
    memset( searchTerm, 0, sizeof( searchTerm ) );
    memset( searchRootPath, 0, sizeof( searchRootPath ) );
}
