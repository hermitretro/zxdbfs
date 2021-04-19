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

#ifndef _zxdbfs_search_h
#define _zxdbfs_search_h

extern FSCacheEntry_t *FSCacheEntry_createFromSearch( const char *path,
                                               json_object *searchData_o,
                                               float minscore,
                                               const char *searchTerm );
extern FSCacheEntry_t *FSCacheEntry_getAndCreateSearch( json_object *urlcache,
                                                 const char *filepath,
                                                 const char *urlhost,
                                                 const char *urlpath,
                                                 int forceFetch );

#endif /** !_zxdbfs_search_h */
