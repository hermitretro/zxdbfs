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

cmake_minimum_required (VERSION 2.8)

project (zxdbfs)

add_subdirectory(${PROJECT_SOURCE_DIR}/json-c/)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/)
add_subdirectory(${PROJECT_SOURCE_DIR}/tests/)

