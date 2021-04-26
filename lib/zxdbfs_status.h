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

#ifndef _zxdbfs_status_h
#define _zxdbfs_status_h

#include <json-c/json.h>

typedef struct ZXDBFSStatus {
    char ssid[64];
    char pairwiseCipher[16];
    char groupCipher[16];
    char keyManagement[16];
    char wpaState[16];
    char ipAddress[32];
    int ntpdok;
    int dateok;
    int zxdbfsdok;
    int spidok;
    char zxdbVersion[16];
} ZXDBFSStatus_t;

extern ZXDBFSStatus_t *ZXDBFSStatus_create();
extern void ZXDBFSStatus_free( ZXDBFSStatus_t *status );
extern ZXDBFSStatus_t *ZXDBFSStatus_createFromJSON( json_object *statusRoot_o );

#endif /** !_zxdbfs_status_h */
