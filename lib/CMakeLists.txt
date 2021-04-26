#
# Copyright (c)2021- Alligator Descartes <http://www.hermitretro.com>
#
# This file is part of zxdbfs.
#
#     zxdbfs is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     zxdbfs is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with zxdbfs.  If not, see <https://www.gnu.org/licenses/>.

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/json-c)

add_compile_options(-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -g)

list(APPEND ZXDBFSLIB_SOURCES
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_byletter.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_fscache.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_fscacheentry.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_gameid.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_http.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_json.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_paths.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_search.c"
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs_status.c"
)

add_library(zxdbfslib STATIC ${ZXDBFSLIB_SOURCES})
