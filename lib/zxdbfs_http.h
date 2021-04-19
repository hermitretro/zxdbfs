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

#ifndef _zxdbfs_http_h
#define _zxdbfs_http_h

struct MemoryStruct {
    char *memory;
    size_t size;
};

int HTTP_TO_OSCODE( int res );

static size_t write_data(void *contents, size_t size, size_t nmemb, void *userp);

struct MemoryStruct *getURLViacURL( const char *host, const char *path );
json_object *getURL( json_object *urlcache, const char *host, const char *path );

#endif /** !_zxdbfs_http_h */
