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
#include <unistd.h>

int createTestFile( const char *path, const char *jsonData ) {

    FILE *f = fopen( path, "wb" );
    if ( !f ) {
        return 1;
    }
    fprintf( f, "%s", jsonData );
    fclose( f );

    return 0;
}

int unlinkTestFile( char *path ) {
    
    return unlink( path );
}
