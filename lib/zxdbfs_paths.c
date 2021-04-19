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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "zxdbfs_paths.h"

/**
 * Count the number of separators in the given path
 * In:
 *      separator - the separator character
 *      path - the path to examine
 *      separatorpossz - the size of the separatorpos array
 * Out:
 *      nseparators - stores the number of separators found
 *      separatorpos - an optional int[] which will be populated with the positions of the separators
 * Returns:
 *      0 - success
 *      1 - failure
 */
int countSeparators( char separator, const char *path, int *nseparators, int *separatorpos, int separatorpossz ) {

    if ( path == NULL || nseparators == NULL ) {
        return -1;
    }

    *nseparators = 0;

    if ( separatorpos != NULL && separatorpossz > 0 ) {
        for ( int i = 0 ; i < separatorpossz ; i++ ) {  
            separatorpos[i] = -1;
        }
    }

    for ( int i = 0 ; i < strlen( path ) ; i++ ) {
        if ( path[i] == separator ) {
            if ( separatorpos != NULL ) {
                separatorpos[*nseparators] = i;
            }
            (*nseparators)++;
        }
    }

    return 0;
}

int countSlashes( const char *path, int *nseparators, int *separatorpos, int separatorpossz ) {
    return countSeparators( '/', path, nseparators, separatorpos, separatorpossz );
}

int countUnderscores( const char *path, int *nseparators, int *separatorpos, int separatorpossz ) {
    return countSeparators( '_', path, nseparators, separatorpos, separatorpossz );
}

/**
 * Is the given string composed only of digits?
 * In:
 *      str - Input string
 * Returns:
 *      0 = All digits
 *      1 = Not all digits
 */
int isAllDigits( const char *str ) {

    if ( str == NULL || strlen( str ) == 0 ) {
        return 1;
    }

    int rv = 0;
    for ( int i = 0 ; i < strlen( str ) ; i++ ) {
        if ( !isdigit( str[i] ) ) {
            return 1;
        }
    }

    return 0;
}

/**
 * Extract the title and ID from the given path
 * In:
 *      path - the path to examine
 * Out:
 *      title - stores the title. Required. Contents should be set to NULL
 *      id - stores the ID. Required. Contents should be set to NULL
 *      gamerootpath - stores the path up to, and including, the title_id segment
 * Returns:
 *      0 - success
 *      1 - failure
 */
int getTitleAndIDFromPath( const char *path, char *title, char *id, char *gamerootpath ) {

    int rv = 0;

    if ( path == NULL || title == NULL || id == NULL || gamerootpath == NULL ) {
        return -1;
    }

    int nslashes = 0;
    int slashpos[32];
    rv = countSlashes( path, &nslashes, slashpos, 32 );
    if ( rv != 0 ) {
        return rv;
    }

    int sindex = 0, eindex = 0;
    int poffset = 0;

    if ( nslashes == 0 ) {
        slashpos[0] = -1;
        slashpos[1] = strlen( path );
        poffset = 0;
    } else {
        /** Try and find the appropriate-looking path segment */
        /** Work back from the end of the path */
        poffset = nslashes;
        /** Is the final slash trailing? */
        if ( slashpos[nslashes] == strlen( path ) ) {
            poffset--;
        }
    }
    
    /** Scan backwards through path segments */
    while ( poffset >= 0 ) {
        int sindex = slashpos[poffset] + 1;
        int eindex = slashpos[poffset + 1] + 1;
        /** No trailing slash */
        if ( eindex < sindex  ) {
            eindex = strlen( path );
        }
        if ( path[eindex - 1] == '/' ) {
            eindex--;
        }

        char pathSegment[256] = { 0 };
        strncpy( pathSegment, &path[sindex], (eindex - sindex) );

        /** Does the last seven characters look ID-ish? */
        if ( strlen( pathSegment ) >= 7 ) {
            int nuscores = 0;
            int uscorepos[8];
            rv = countUnderscores( pathSegment, &nuscores, uscorepos, 8 );
            if ( nuscores > 0 ) {
                int lastuscorepos = uscorepos[nuscores - 1] + 1;
                /** Is the last underscore section ID-ish? */
                if ( strlen( &pathSegment[lastuscorepos] ) == 7 ) {
                    if ( isAllDigits( &pathSegment[lastuscorepos] ) == 0 ) {
                        strncpy( title, pathSegment, lastuscorepos - 1 );
                        strcpy( id, &pathSegment[lastuscorepos] );
                        strncpy( gamerootpath, path, eindex );
                        return 0;
                    }
                }
            }
        }

        poffset--;
    }

    return 1;
}

/**
 * Returns the dirname component of the given path. This assumes the final
 * path segment is a filename
 * In:
 *      path - Path to extract the dirname from. Required
 *      dir - Path with final segment chopped off. Required and should have NULL contents. The caller is responsible for being sized appropriately
 * Out:
 * Returns:
 *      0 = success
 *      1 = failure
 */
int getDirname( const char *path, char *dir ) {

    int rv = 0;

    if ( path == NULL || dir == NULL || strlen( path ) == 0 ) {
        return 1;
    }

    int nslashes = 0;
    int slashpos[32];
    rv = countSlashes( path, &nslashes, slashpos, 32 );
    if ( rv != 0 ) {
        return rv;
    }

    if ( nslashes == 0 ) {
        strcpy( dir, "." );
        return 0;
    }

    if ( nslashes == 1 && slashpos[0] == 0 ) {
        /** File in / */
        strcpy( dir, "/" );
        return 0;
    }

    /** Handle cases where we have a terminating / */
    int offset = nslashes - 1;
    if ( path[strlen( path ) - 1] == '/' ) {
        offset--;
    }

    if ( offset == 0 ) {
        strcpy( dir, "/" );
    }

    /** Remove trailing slashes */
    int index = slashpos[offset];
    while ( path[index] == '/' ) {
        index--;
    }

    strncpy( dir, path, index + 1 );
    
    return 0;
}

/**
 * Returns the basename component of the given path
 * In:
 *      path - Path to extract the basename of. Required
 *      dir - Final path segment. Required and should have NULL contents. The caller is responsible for being sized appropriately
 * Out:
 * Returns:
 *      0 = success
 *      1 = failure
 */
int getBasename( const char *path, char *dir ) {

    int rv = 0;

    if ( path == NULL || dir == NULL || strlen( path ) == 0 ) {
        return 1;
    }

    int nslashes = 0;
    int slashpos[32];
    rv = countSlashes( path, &nslashes, slashpos, 32 );
    if ( rv != 0 ) {
        return rv;
    }

    /** Just a filename... */
    if ( nslashes == 0 ) {
        strcpy( dir, path );
        return 0;
    }

    /** Just / */
    if ( strlen( path ) == 1 && path[0] == '/' ) {
        strcpy( dir, path );
        return 0;
    }

    /** Strip trailing slashes */
    int eindex = strlen( path ) - 1;
    while ( path[eindex] == '/' ) {
        eindex--;
    }

    /** Scan back to previous slash */
    int sindex = eindex;
    while ( path[sindex] != '/' && sindex >= 0 ) {
        sindex--;
    }
    sindex++;

    strncpy( dir, &path[sindex], eindex - sindex + 1 );
    
    return 0;
}

/**
 * Fix path from original WoS path to archive.org path
 * In:
 *      inpath - Original WoS path. Required.
 * Out:
 *      outpath - Fixed path. Required. Should be preallocated and zeroed
 * Returns:
 *      0 = success
 *      1 = failure
 */
int fixupWoSPath( const char *inpath, char *outpath ) {

    if ( inpath == NULL || outpath == NULL ) {
        return 1;
    }

    /**
     * Strip /pub/sinclair as the archive.org paths differ from the 
     * original WoS ones...
     */
    if ( strncmp( inpath, "/pub/sinclair", 13 ) == 0 ) {
        strcpy( outpath, &inpath[13] );
    } else {
        strcpy( outpath, inpath );
    }

    return 0;
}

/**
 * Returns root URL for downloads from the path. This should be URL
 * escaped. Escaping happens later only on the path
 *
 * In:
 *      path - Path to analyse. Required
 * Out:
 *      N/A
 * Returns:
 *      Root download URL for the given path
 */
char *getRootDownloadURL( const char *path ) {

    char *rv = NULL;

    if ( path == NULL ) {
        return NULL;
    }

    if ( strncmp( path, "/zxdb/sinclair", 14 ) == 0 ) {
        rv = strdup( "https://spectrumcomputing.co.uk" );
    } else {
        if ( (strncmp( path, "/games", 5 ) == 0) ||
             (strncmp( path, "/screens", 8 ) == 0) ) {
            rv = strdup( "https://archive.org/download/World_of_Spectrum_June_2017_Mirror/World%20of%20Spectrum%20June%202017%20Mirror.zip/World%20of%20Spectrum%20June%202017%20Mirror/sinclair" );
        } else {
            return NULL;
        }
    }

    return rv;
}

/**
 * Returns the ZXDB URL to fetch game data from
 * In:
 *      gameid - the ZXDB game id (7 digits)
 * Out:
 *      N/A
 * Returns:
 *      the URL excluding host or NULL on failure
 */
char *getGameURLPath( const char *gameid ) {

    if ( gameid == NULL ) {
        return NULL;
    }

    if ( strlen( gameid ) != 7 ) {
        return NULL;
    }

    if ( isAllDigits( gameid ) != 0 ) {
        return NULL;
    }

    char path[256];
    sprintf( path, "/games/%s?mode=compact", gameid );

    return strdup( path );
}

int getSearchTermFromPath( const char *path, char *searchTerm,
                           char *searchrootpath ) {

    int rv = 0;

    if ( path == NULL || searchTerm == NULL || searchrootpath == NULL ) {
        return 1;
    }

    if ( strncmp( path, "/search", 7 ) != 0 ) {
        return 1;
    }

    int nslashes = 0;
    int slashpos[32] = { -1 };
    rv = countSlashes( path, &nslashes, slashpos, 32 );
    if ( rv != 0 ) {
        return rv;
    }

    if ( nslashes >= 2 ) {
        /** The search term is always after /search/<term> */
        /** The search term is always after /search/<term>/... */
        if ( slashpos[1] != -1 ) {
            if ( slashpos[2] != -1 ) {
                strncpy( searchTerm, &path[slashpos[1] + 1], (slashpos[2] - slashpos[1] - 1) );
                strcpy( searchrootpath, &path[slashpos[2]] );
                return 0;
            } else {
                strcpy( searchTerm, &path[slashpos[1] + 1] );
                return 0;
            }
        }
    }

    return 1;
}
