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

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR}/testdata)
include_directories(/usr/src/googletest/googletest/include/ /usr/src/googletest/googletest)

add_compile_options(-D_FILE_OFFSET_BITS=64 -g)

list(APPEND TEST_SOURCES
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_byletter_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_fscache_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_fscacheentry_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_gameid_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_http_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_paths_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_search_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_status_tests.cpp
${CMAKE_CURRENT_LIST_DIR}/zxdbfs_tests_utils.cpp
/usr/src/googletest/googletest/src/gtest-all.cc
/usr/src/googletest/googletest/src/gtest_main.cc
)

link_directories(${PROJECT_SOURCE_DIR}/json-c)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_executable(zxdbfstests ${TEST_SOURCES})
target_link_libraries(zxdbfstests zxdbfslib json-c curl pthread)
