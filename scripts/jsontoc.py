
#  Copyright (C) 2021  Alligator Descartes <alligator.descartes@hermitretro.com>
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

import json
import os
import sys

def convert(input):
    if isinstance(input, dict):
        return dict(map(convert, input.items()))
    elif isinstance(input, list):
        return [convert(element) for element in input]
    else:
        return input

if __name__ == "__main__":
    infile = open( sys.argv[1], 'r' )
    outfile = open( sys.argv[2], 'w' )

    data = json.load( infile )
    data = convert( data )
    data = json.dumps( data )
    data = data.replace( ', "', ',"' )
    data = data.replace( ', {', ',{' )
    data = data.replace( '"', '\\"' )
    data = data.replace( ': ', ':' )
    data = data.replace( '\\n', '\\\\n' )
    outfile.write( 'const char *jsonData = "')
    outfile.write( data )
    outfile.write( '";\n')

    infile.close()
    outfile.close()
