/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
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

/** @file
 *
 * Filesystem fronting ZXDB using high-level API
 *
 * ## Source code ##
 * \include zxdbfs.c
 */

#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>

#include <json-c/json.h>

#include <curl/curl.h>

#include <zxdbfs_byletter.h>
#include <zxdbfs_gameid.h>
#include <zxdbfs_http.h>
#include <zxdbfs_json.h>
#include <zxdbfs_paths.h>
#include <zxdbfs_search.h>

typedef unsigned int UINT;

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *zxdbrooturl;
    const char *cacherootdir;
    const char *cacherooturl;
    const char *useragent;
    int localroot;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--zxdbrooturl=%s", zxdbrooturl),
	OPTION("--cacherootdir=%s", cacherootdir),
	OPTION("--cacherooturl=%s", cacherooturl),
	OPTION("--useragent=%s", useragent),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

/** Various caches */
static json_object *urlcache = NULL;
static FSCache_t *fscache = NULL;
static FSCache_t *bylettercache = NULL;
int latestStatusSize = 0;

/**
 * Preload the by-letter cache
 */
void _preloadByLetterCache( char letter ) {

    char path[256];
    sprintf( path, "%s/by-letter-%c.json", options.cacherootdir, letter );

    struct stat st;
    if ( stat( path, &st ) == -1 ) {
        printf( "failed to stat preload file: %s\n", path );
        return;
    }

    printf( "preload file: %s (%ld bytes)\n", path, st.st_size );

    unsigned char *buf = (unsigned char *)malloc( st.st_size + 256 );

    FILE *f = fopen( path, "rb" );
    if ( f == NULL ) {
        printf( "failed to open preload file: %s\n", path );
        return;
    }

    fread( buf, 1, st.st_size, f );
    fclose( f );

    json_object *rv = json_tokener_parse( buf );
    if ( rv == NULL ) {
        printf( "failed to preload: %s\n", path );
    } else {
        printf( "preloaded: %s\n", path );
        char key[20] = { 0 };
        sprintf( key, "/by-letter/%c", letter );
        FSCacheEntry_t *byLetter =
            FSCacheEntry_createFromByLetter( key, rv );
        json_object_put( rv );
        if ( byLetter != NULL ) {
            printf( "adding all...\n" );
            FSCache_addAll( fscache, key, byLetter );
        }
    }
}

/**
 * unstub a dir_stub FSCacheEntry
 */
FSCacheEntry_t *_unstub( FSCacheEntry_t *fsCacheEntry, const char *path ) {

    int rv = 0;
    FSCacheEntry_t *fscrv = NULL;

    /** If the cache entry is a dirstub, page in the real one */
    FSCacheEntryType fsctype = FSCacheEntry_gettype( fsCacheEntry );
    if ( fsctype == FSCACHEENTRY_DIR_STUB ) {
        printf( "stub dir!\n" );
        FSCacheEntry_t *fsCacheEntryFull = FSCacheEntry_getAndCreateGame( urlcache, path, options.zxdbrooturl, NULL, 0 );
        if ( fsCacheEntryFull == NULL ) {
            printf( "failed to load unstubbed data for: %s\n", path );
        } else {
            printf( "fetched unstubbed data for: %s\n", path );
            dumpJSON( fsCacheEntry );
            rv = FSCache_delete( fscache, path );
            if ( rv == 1 ) {
                printf( "fscache removal failed during unstubbing\n" );
            } else {
                if ( rv == 2 ) {
                    printf( "fscacheentry not present in cache during unstubbing\n" );
                } else {
                    printf( "cache removal ok during unstubbing\n" );
                }
            }
                
            if ( FSCache_addAll( fscache, path, fsCacheEntryFull ) != 0 ) {
                printf( "Failed to overwrite stub data with unstubbed for: %s\n", path );
                return NULL;
            }
            fscrv = FSCache_get( fscache, path );
            fsctype = FSCacheEntry_gettype( fscrv );
            if ( fscrv == NULL || fsctype != FSCACHEENTRY_DIR ) {
                printf( "Failed to unstub from fscache for: %s\n", path );
                return NULL;
            }
            printf( "replaced stub with unstub for: %s\n", path );
        }
    } else {
        fscrv = fsCacheEntry;
    } 

    return fscrv;
}

/**
 * Handle the magic game directory structure
 */
int _readdirFSCache( const char *path,
                     void *buf, 
                     fuse_fill_dir_t filler,
			         off_t offset, struct fuse_file_info *fi,
			         enum fuse_readdir_flags flags, 
                     unsigned int pnfileinfo ) {

    int i;
    struct stat st;
    unsigned int nfileinfo = pnfileinfo;

    memset( &st, 0, sizeof( st ) );

    printf( ">>> _readdirFSCache: %s, nfileinfo = %d, offset = %ld\n", 
            path, nfileinfo, offset );

    /** Assume the fscache is populated.... */
    FSCacheEntry_t *fsCacheEntry = FSCache_get( fscache, path );
    if ( fsCacheEntry == NULL ) {
        printf( "fscacheroot is null.....\n" );
        /** Load the game data to the cache */
        return 0;
    }

    /** Potentially unstub */
    fsCacheEntry = _unstub( fsCacheEntry, path );
    if ( fsCacheEntry == NULL ) {
        return 0;
    }

    int nfiles = FSCacheEntry_getnfiles( fsCacheEntry );
    for ( i = offset ; i < nfiles ; i++ ) {


        FSCacheEntry_t *file = FSCacheEntry_getfile( fsCacheEntry, i );
        if ( file == NULL ) {
            printf( "FIXME\n" );
        }
        FSCacheEntryType file_type = FSCacheEntry_gettype( file );

        struct stat st;
        memset( &st, 0, sizeof( st ) );

        /** Strip the path from the filename to just the final path segment */
        const char *file_fname = FSCacheEntry_getfname( file );
        if ( file_fname == NULL ) {
            printf( "FIXME\n" );
        }
        char basename[256] = { 0 };
        getBasename( file_fname, basename );

        switch ( file_type ) {
            case FSCACHEENTRY_FILE: {
                //printf( "filling file: %s\n", file_fname );
                st.st_mode = S_IFREG | 0644;
                st.st_nlink = 1;
                st.st_size = FSCacheEntry_getsize( file );
                if ( filler( buf, basename, &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) != 0 ) {
                    printf( "failed to fill file: %s\n", file_fname );
                    return 0;
                }
                break;
            }
            case FSCACHEENTRY_DIR:
            case FSCACHEENTRY_DIR_STUB: {
                //printf( "filling dir: %s\n", file_fname );
                st.st_mode = S_IFDIR | 0755;
                st.st_nlink = 2;
                if ( filler( buf, basename, &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) != 0 ) {
                    printf( "failed to fill dir: %s\n", file_fname );
                    return 0;
                }
                break;
            }
            default: {
                printf( "Unknown fscacheentry\n" );
                break;
            }
        }
    }

    return 0;
}

/**
 * Handle listing the "by-letter" directory level, eg, /by-letter/A.
 * This will list a lot of game titles as directory entries only.
 * We append the underlying ID because there are clashes with titles
 * and we need to look each game's data up by ID...
 *
 * Use a preloaded cache for this since the REST endpoint offset pagination
 * isn't very compatible with libfuse offsetting
 */
int _readdirByLetterAZ( const char *path,
                        void *buf, 
                        fuse_fill_dir_t filler,
			            off_t offset, struct fuse_file_info *fi,
			            enum fuse_readdir_flags flags, 
                        unsigned int pnfileinfo ) {

    FSCacheEntry_t *byLetterRoot = NULL;
    unsigned int nfileinfo = pnfileinfo;

    printf( "_readdirByLetterAZ: in /by-letter/[A-Z]: offset: %ld, nfileinfo: %d\n", offset, nfileinfo );

    /** Extract the letter */
    if ( strlen( path ) < 12 ) {
        return -ENOENT;
    }

    int eindex = strlen( path ) - 1;
    if ( path[eindex] == '/' ) {
        eindex--;
    }
    
    char letter = path[eindex];
    letter = toupper( letter );
    printf( "Retrieving by-letter: %c\n", letter );

    char key[20] = { 0 };
    sprintf( key, "/by-letter/%c", letter );
    byLetterRoot = FSCache_get( fscache, key );
    if ( byLetterRoot == NULL ) {
        _preloadByLetterCache( letter );
        byLetterRoot = FSCache_get( fscache, key );
        if ( byLetterRoot == NULL ) {
            printf( ">>> FAILED TO RETRIEVE BY LETTER FROM CACHE\n" );

            /** Retrieve the full by-letter data from ZXDB */
            char url[256];
            sprintf( url, "/games/byletter/%c?contenttype=SOFTWARE&mode=tiny&size=5000&offset=0", letter );
            printf( "by-letter URL: %s\n", url );

            json_object *urlobj = getURL( urlcache, options.zxdbrooturl, url, options.useragent );
            if ( urlobj == NULL ) {
                printf( "Failed to retrieve by-letter data from ZXDB\n" );
                return -ENODEV;
            }

            /** Write back the data to the local cache */
            char cname[256];
            sprintf( cname, "%s/by-letter-%c.json", options.cacherootdir, letter );
            FILE *f = fopen( cname, "wb" );
            if ( f != NULL ) {
                const char *output = json_object_to_json_string_ext(urlobj, JSON_C_TO_STRING_PRETTY);
                fprintf( f, "%s\n", output );
                fclose( f );
                printf( "Writeback of by-letter cache OK\n" );
            } else {
                printf( "Failed to writeback the by-letter cache\n" );
            }

            /** Attempt final instantiation */
            byLetterRoot =
                FSCacheEntry_createFromByLetter( path, urlobj );
            if ( byLetterRoot != NULL ) {
                FSCache_addAll( fscache, key, byLetterRoot );
            }
        }
    }

    return _readdirFSCache( path, buf, filler, offset, fi,
                            flags, nfileinfo );
}

/**
 * Initialise the filesystem
 */
static void *zxdb_fuse_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->auto_cache = 1;
    cfg->attr_timeout = 3600;

    urlcache = json_object_new_object();
    fscache = FSCache_create();
    bylettercache = FSCache_create();

	return NULL;
}

static void _getattrFromFSCache( FSCacheEntry_t *fscacheobj, struct stat *stbuf ) {

    if ( fscacheobj == NULL || stbuf == NULL ) {
        return;
    }

    FSCacheEntryType fsctype = FSCacheEntry_gettype( fscacheobj );
    switch ( fsctype ) {
        case FSCACHEENTRY_DIR:
        case FSCACHEENTRY_DIR_STUB: {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return;
        }
        case FSCACHEENTRY_FILE: {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = FSCacheEntry_getsize( fscacheobj );
            return;
        }
        default: {
            /** This is pretty bad... */
            printf( "_getattrfromcache: unknown fscacheentry type: %d\n", fsctype );
            return;
        }
    }
}

static FSCacheEntry_t *_getAndCreateGame( const char *path, struct stat *stbuf ) {

    /** Check the fscache first */
    FSCacheEntry_t *fsCacheEntry = FSCache_get( fscache, path );
    if ( fsCacheEntry != NULL ) {
        printf( "found fscacheentry for %s\n", path );
        _getattrFromFSCache( fsCacheEntry, stbuf );
    } else {
        printf( "failed to find fscacheentry for %s\n", path );
        /** Extract the gameroot path */
        char title[128] = { 0 };
        char id[16] = { 0 };
        char gamerootpath[128] = { 0 };
        getTitleAndIDFromPath( path, title, id, gamerootpath );
        /** */
        fsCacheEntry = FSCacheEntry_getAndCreateGame( urlcache, gamerootpath, options.zxdbrooturl, NULL, 0 );
        if ( fsCacheEntry == NULL ) {
            /** Failed to retrieve game data -- this is pretty bad */
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        } else {
            printf( "Got game data OK for: %s\n", gamerootpath );
            /** Fully populate the game data in the FS cache */
            FSCache_addAll( fscache, gamerootpath, fsCacheEntry );
            /** Refetch the current fscacheentry prior in case of unstubbing */
            fsCacheEntry = FSCache_get( fscache, path );
            _getattrFromFSCache( fsCacheEntry, stbuf );
            return 0;
        }
    }
}

/**
 * Refresh zxdbfsstatus info
 */
int _getStatusSize( int mode ) {

    const char *tempStatusFile = "/tmp/zxdbfsstatus.txt";

    unlink( tempStatusFile );

    char cmd[256];
    if ( mode == 0 ) {
        sprintf( cmd, "/home/pi/zxdbfs/bin/zxdbfsstatus > %s", tempStatusFile );
    } else {
        sprintf( cmd, "/home/pi/zxdbfs/bin/zxdbfsstatus -b > %s", tempStatusFile );
    }

    system( cmd );

    struct stat st;
    stat( tempStatusFile, &st );
    latestStatusSize = st.st_size;
    return st.st_size;
}

static int zxdb_fuse_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
    int res;
    //FILINFO finfo;

	memset(stbuf, 0, sizeof(struct stat));

    printf( "getattr: %s\n", path );

    if ( strcmp( path, "/" ) == 0 ) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
     }

    /**
     * Handle the magic /cache directories
     */
    if ( strncmp( path, "/cache", 6 ) == 0 ) {
        if ( strncmp( path, "/cache/fscache", 14 ) == 0 ) {
            if ( strcmp( path, "/cache/fscache/flush" ) == 0 ) {
                printf( "flushing fscache\n" );
                FSCache_flush( fscache );
            } else {
                if ( fscache != NULL && fscache->cache != NULL ) {
                    if ( strcmp( path, "/cache/fscache" ) == 0 ) {
                        dumpJSON( fscache->cache );
                    }
                }
            }
        }
        if ( strncmp( path, "/cache/urlcache", 14 ) == 0 ) {
            if ( strcmp( path, "/cache/urlcache/flush" ) == 0 ) {
                printf( "flushing urlcache\n" );
                json_object_put( urlcache );
                urlcache = json_object_new_object();
            }
        }

        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    /** Handle the magic /status directories */
    if ( strncmp( path, "/status", 7 ) == 0 ) {
        if ( strncmp( path, "/status/json", 12 ) == 0 ) {

            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = _getStatusSize( 0 );
            return 0;
        }
        if ( strncmp( path, "/status/binary", 14 ) == 0 ) {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = 0;
            return 0;
        }

        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    /** Handle /by-letter magic directory */
    if ( (strncmp( path, "/by-letter", 10 ) == 0) ) {

        /** /by-letter */
        if ( strlen( path ) == 10 ) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        }

        /** /by-letter/[A-Z] */
        if ( strlen( path ) == 12 ) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        }
            
        _getAndCreateGame( path, stbuf );
    }

    /** Handle /search magic directory */
    if ( strncmp( path, "/search", 7 ) == 0 ) {

        if ( strlen( path ) == 7 ) {
            /** /search */
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        }

        printf( "getattr: /search\n" );

        /** Process the query or pre-existing result */
        /** Check the fscache first */
        FSCacheEntry_t *fsCacheEntry = FSCache_get( fscache, path );
        if ( fsCacheEntry != NULL ) {
            printf( "found fscacheentry for %s\n", path );
            _getattrFromFSCache( fsCacheEntry, stbuf );
        } else {
            printf( "failed to find fscacheentry for %s\n", path );
            /** This is a naked search term or a search result? */
            char searchTerm[128] = { 0 };
            char searchRootPath[256] = { 0 };

            /** Do we have a cached entry for the naked search term? */
            if ( getSearchTermFromPath( path, searchTerm, searchRootPath ) != 0 ) {
                printf( "failed to find search term\n" );
                return 0;
            } 
            
            char searchkey[128] = { 0 };
            sprintf( searchkey, "/search/%s", searchTerm );
            fsCacheEntry = FSCache_get( fscache, searchkey );
            if ( fsCacheEntry == NULL ) {
                fsCacheEntry = FSCacheEntry_getAndCreateSearch( urlcache, path, options.zxdbrooturl, NULL, 0 );
                if ( fsCacheEntry == NULL ) {
                    /** Failed to retrieve search data -- this is pretty bad */
                    stbuf->st_mode = S_IFDIR | 0755;
                    stbuf->st_nlink = 2;
                    return 0;
                } else {
                    printf( "Got search data OK for: %s\n", path );
                    /** Fully populate the game data in the FS cache */
                    FSCache_addAll( fscache, path, fsCacheEntry );
                    /** Add the search term into /search */
                    int needsAdd = 0;
                    FSCacheEntry_t *search = FSCache_get( fscache, "/search" );
                    if ( search == NULL ) {
                        printf( "creating new /search fscache entry\n" );
                        search = FSCacheEntry_create( "/search", FSCACHEENTRY_DIR, NULL, 0 );
                        needsAdd = 1;
                    } else {
                        printf( "reusing /search fscache entry\n" );
                    }
                    FSCacheEntry_t *search_searchTerm = FSCacheEntry_create( searchkey, FSCACHEENTRY_DIR, NULL, 0 );
                    FSCacheEntry_addFile( search, search_searchTerm );
                    if ( needsAdd ) {
                        FSCache_add( fscache, "/search", search );
                    }

                    /** Refetch the current fscacheentry prior in case of unstubbing */
                    fsCacheEntry = FSCache_get( fscache, path );
                }
            }

            /** At this stage, we should definitely have the naked search data in */
            /** If we've got qualification after the search term, treat that as game data */
            if ( strlen( searchRootPath ) > 0 ) {
                printf( "have search root path. load game data: %s\n", path );
                _getAndCreateGame( path, stbuf );
            } else {
                _getattrFromFSCache( fsCacheEntry, stbuf );
            }

            return 0;
        }
    }

	return 0;
}

static int zxdb_fuse_readdir( const char *path, void *buf, 
                                 fuse_fill_dir_t filler,
			                     off_t offset, struct fuse_file_info *fi,
			                     enum fuse_readdir_flags flags )
{
    int res;
    int rv = 0;
    unsigned int nfileinfo = offset;

    struct stat st;
    memset( &st, 0, sizeof( st ) );

    nfileinfo++;    /** readdirplus offset needs to start at 1.. */

    printf( "zxdb_fuse_readdir: nfileinfo: %d\toffset: %ld\n", nfileinfo, offset );

    /**
     * Inject "this" and parent directories only at the start of the dir read
     */
    if ( offset == 0 ) {
        printf( "inject default directories\n" );
        st.st_mode = S_IFDIR | 0755;
        st.st_nlink = 2;
        st.st_ino = 0xffffffff; /** Needs to be set otherwise these directories won't add in plus mode. This value corresponds to FUSE_UNKNOWN_INO in fuse.c */
        if ( filler( buf, ".", &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
            printf( "failed to inject .\n" );
        }
        if ( filler( buf, "..", &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
            printf( "failed to inject .\n" );
        }
    }

    /**
     * Are we in any of the magic root directories top-level to 
     * avoid ZXDB calls?
     */
    if ( strcmp( path, "/" ) == 0 ) {
        printf( "readdir: in /\n" );

        if ( offset == 0 ) {
            if ( filler( buf, "by-letter", &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
                printf( "failed to inject by-letter\n" );
            }
            if ( filler( buf, "search", &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
                printf( "failed to inject search\n" );
            }
            if ( filler( buf, "status", &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
                printf( "failed to inject status\n" );
            }
        }

        return 0;
    }

    /** Handle by-letter */
    if ( strcmp( path, "/by-letter" ) == 0 ) {
        printf( "readdir: in /by-letter\n" );
        
        st.st_mode = S_IFDIR | 0755;
        st.st_nlink = 2;
        st.st_ino = 0xffffffff; /** Needs to be set otherwise these directories won't add in plus mode. This value corresponds to FUSE_UNKNOWN_INO in fuse.c */

        int loffset = offset;
        if ( loffset > 0 ) {
            loffset -= 2;   /** Subtract . and .. */
        }

        for ( int i = loffset ; i < 26 ; i++ ) {
            char b[256];
            sprintf( b, "%c", 'A' + i );
            printf( "injecting %s at %i, %i\n", b, i, nfileinfo );
            if ( filler( buf, b, &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
                break;
            }
        }

        return 0;
    }

    /** Handle /search */
    if ( strcmp( path, "/search" ) == 0 ) {
        printf( "readdir: in /search\n" );
        
        st.st_mode = S_IFDIR | 0755;
        st.st_nlink = 2;
        st.st_ino = 0xffffffff; /** Needs to be set otherwise these directories won't add in plus mode. This value corresponds to FUSE_UNKNOWN_INO in fuse.c */

        /** Return any search results */
        return _readdirFSCache( path, buf, filler, offset, fi, flags, nfileinfo );
    }

    /** Handle /status -- contains two magic files */
    if ( strcmp( path, "/status" ) == 0 ) {
        printf( "readdir: in /status\n" );

        if ( offset > 0 ) {
            return 0;
        }

        st.st_mode = S_IFREG | 0644;
        st.st_nlink = 1;
        st.st_size = latestStatusSize;

        if ( filler( buf, "binary", &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
            printf( "failed to inject /status/binary\n" );
        }
        if ( filler( buf, "json", &st, nfileinfo++, FUSE_FILL_DIR_PLUS ) ) {
            printf( "failed to inject /status/json\n" );
        }

        return 0;
    }

    /**
     * Which magic directory type are we in that calls into ZXDB?
     *
    /** Top-level /by-letter/[A-Z]? */
    printf( "readdir: after top-level dir processing: nfileinfo = %d, offset = %ld\n", nfileinfo, offset );
    if ( strncmp( path, "/by-letter", 10 ) == 0 ) {
        if ( strlen( path ) == 12 ) {
            return _readdirByLetterAZ( path, buf, filler, offset, fi, flags, nfileinfo );
        } else {
            /** Probably a game subdirectory */
            if ( strlen( path ) > 12 ) {
                return _readdirFSCache( path, buf, filler, offset, fi, flags, nfileinfo );
            }
        }
    }

    if ( strncmp( path, "/search", 7 ) == 0 ) {
        /** Search results subdir */
        printf( "search results subdir\n" );
        return _readdirFSCache( path, buf, filler, offset, fi, flags, nfileinfo );
    }

readdir_cleanup:
	return rv;
}

static int zxdb_fuse_open(const char *path, struct fuse_file_info *fi)
{
    int res;
    int i;

    char *rooturl = NULL;
    char *fscurl = NULL;
    int fscsize = 0;

    printf( "fuse_open: %s (mode %d)\n", path, fi->flags );

    if ( strncmp( path, "/status", 7 ) != 0 ) {
        /** Fetch the file info */
        FSCacheEntry_t *fsCacheEntry = FSCache_get( fscache, path );
        if ( fsCacheEntry == NULL ) {
            printf( "fuse_open: fscache not populated!\n" );
            return 0;
        }

        FSCacheEntryType fsctype = FSCacheEntry_gettype( fsCacheEntry );
        fscurl = FSCacheEntry_geturl( fsCacheEntry );
        if ( fsctype != FSCACHEENTRY_FILE || fscurl == NULL ) {
            printf( "Malformed fscache object: %d, %s\n", fsctype, fscurl );
            dumpJSON( fsCacheEntry );
            return 0;
        }

        printf( "URL: %s\n", fscurl );

        /** Figure out where the actual file is... */
        rooturl = getRootDownloadURL( fscurl );
        if ( rooturl != NULL ) {
            printf( "actual URL: %s%s\n", rooturl, fscurl );
        } else {
            printf( "cannot determine root url\n" );
            return -ENOENT;
        }

        fscsize = FSCacheEntry_getsize( fsCacheEntry );
    } else {
        /** Magic status directory */
        rooturl = strdup( "file://" );
        fscurl = strdup( "/tmp/zxdbfsstatus.txt" );
    }

    /** Retrieve the URL via cURL */
    struct MemoryStruct *chunk = getURLViacURL( rooturl, fscurl, options.useragent );
    if ( chunk != NULL ) {
        if ( fscsize != 0 ) {
            if ( fscsize != chunk->size ) {
                printf( "chunk/metadata mismatch: %ld != %d\n", chunk->size, fscsize );
            }
        }
        fi->fh = (unsigned long)chunk;
    } else {
        fi->fh = 0;
    }
    
    return 0;
}

static int zxdb_fuse_release( const char *path, struct fuse_file_info *fi)
{

    /** Free data in fi->fh */

    struct MemoryStruct *fp = (struct MemoryStruct *)fi->fh;
    if ( fp == NULL ) {
        printf( "release: fp is NULL\n" );
        return -ENOENT;
    }

    free( fp );
    fi->fh = 0;

    return 0;
}

static int zxdb_fuse_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	(void) fi;
    int res;

    printf( "fuse_read: %s -> %ld bytes (%ld offset)\n", path, size, offset );

    struct MemoryStruct *fp = (struct MemoryStruct *)fi->fh;
    if ( fp == NULL ) {
        printf( "read: fp is NULL\n" );
        return -ENOENT;
    }

    /** 
     * Compute how much data to copy
     */
    int ntocopy = size;
    if ( (offset + size) > fp->size ) {
        printf( "offset + size > fpsize\n" );
        ntocopy = fp->size - offset;
    } else {
        ntocopy = size;
    }

    memcpy( buf, &fp->memory[offset], ntocopy );

	return ntocopy;
}

static const struct fuse_operations zxdb_fuse_oper = {
	.init       = zxdb_fuse_init,
	.getattr	= zxdb_fuse_getattr,
	.readdir	= zxdb_fuse_readdir,
	.open		= zxdb_fuse_open,
	.read		= zxdb_fuse_read,
    .release    = zxdb_fuse_release
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.zxdbrooturl = strdup("https://api.zxinfo.dk/v3");
	options.cacherootdir = strdup("/tmp/zxdbfscache");
    char lcacherooturl[256];
    sprintf( lcacherooturl, "file://%s", options.cacherootdir );
    options.cacherooturl = strdup( lcacherooturl );
    options.localroot = 0;  /** Set to 1 to disable .. at top-level */
    options.useragent = strdup("zxdbfs");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &zxdb_fuse_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
