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

#include <stdio.h>
#include <string.h>

#include "zxdbfs_json.h"

/**
 * Dump a JSON object in pretty format to stdout
 * In:
 *      obj - The JSON structure to dump 
 * Out:
 *      N/A
 * Returns:
 *      N/A
 */
void dumpJSON( json_object *obj ) {
    const char *output = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY);
    printf( "%s\n", output );
}
