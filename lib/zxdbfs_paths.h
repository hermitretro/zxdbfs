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

#ifndef _zxdbfs_paths_h
#define _zxdbfs_paths_h

/** String mangling */
extern int countSeparators( char separator, const char *path, int *nseparators, int *separatorpos, int separatorpossz );
extern int countSlashes( const char *path, int *nseparators, int *separatorpos, int separatorpossz );
extern int countUnderscore( const char *path, int *nseparators, int *separatorpos, int separatorpossz );
extern int isAllDigits( const char *path );

/** ZXDB helpers */
extern int getTitleAndIDFromPath( const char *path, char *title, char *id,
                                  char *gamerootpath );
extern int getSearchTermFromPath( const char *path, char *searchTerm, char *searchrootpath );

/** Path mangling */
extern int getDirname( const char *path, char *dir );
extern int getBasename( const char *path, char *dir );

/** WoS path fixup */
int fixupWoSPath( const char *inpath, char *outpath );

char *getRootDownloadURL( const char *path );

char *getGameURLPath( const char *gameid );

#endif /** !_zxdbfs_paths_h */
