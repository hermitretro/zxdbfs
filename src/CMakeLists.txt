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

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/lib)

add_compile_options(-D_FILE_OFFSET_BITS=64 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -g)

list(APPEND ZXDBFS_SOURCES
"${CMAKE_CURRENT_LIST_DIR}/zxdbfs.c"
)

link_directories(${PROJECT_SOURCE_DIR}/json-c ${PROJECT_SOURCE_DIR}/lib)
add_executable(zxdbfs ${ZXDBFS_SOURCES})
target_link_libraries(zxdbfs fuse3 json-c zxdbfslib curl)
