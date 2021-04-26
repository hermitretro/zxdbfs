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

#include "zxdbfs_status.h"
#include "zxdbfs_json.h"

ZXDBFSStatus_t *ZXDBFSStatus_create() {

    ZXDBFSStatus_t *status = (ZXDBFSStatus_t *)malloc( sizeof( ZXDBFSStatus_t ) );
    if ( status == NULL ) {
        return NULL;
    }

    memset( status, 0, sizeof( ZXDBFSStatus_t ) );
    return status;
}

/**
 * Releases a status object
 * In:
 *      obj - object to free
 * Out:
 *      N/A
 * Returns:
 *      N/A
 */
void ZXDBFSStatus_free( ZXDBFSStatus_t *obj ) {

    if ( obj == NULL ) {
        return;
    }

    free( obj );
}

/**
 * Return a suitably populated status object
 * In:
 *      statusRoot_o - the JSON object previously obtained from somewhere
 * Out:
 *      N/A
 * Returns:
 *      NULL - creation fails
 *      ZXDBFSStatus_t * - creation OK
 */
ZXDBFSStatus_t *ZXDBFSStatus_createFromJSON( json_object *statusRoot_o ) {

    if ( statusRoot_o == NULL ) {
        return NULL;
    }

    ZXDBFSStatus_t *status = ZXDBFSStatus_create();
    if ( status == NULL ) {
        return NULL;
    }

    /** Check the type */
    json_object *type = json_object_object_get( statusRoot_o, "type" );
    if ( type == NULL ) {
        ZXDBFSStatus_free( status );
        return NULL;
    } else {
        if ( strcmp( "zxdbfsstatus", json_object_get_string( type ) ) != 0 ) {
            ZXDBFSStatus_free( status );
            return NULL;
        }
    }

    /** Extract the relevant JSON fields for wpa_cli */
    json_object *wpa_cli_o = json_object_object_get( statusRoot_o, "wpa_cli" );
    if ( wpa_cli_o == NULL ) {
        ZXDBFSStatus_free( status );
        return NULL;
    } else {
        /** SSID */
        json_object *wpa_cli_ssid_o = 
            json_object_object_get( wpa_cli_o, "ssid" );
        if ( wpa_cli_ssid_o == NULL ) {
            ZXDBFSStatus_free( status );
            return NULL;
        }
        strncpy( status->ssid, json_object_get_string( wpa_cli_ssid_o ), sizeof( status->ssid ) );

        /** Pairwise cipher */
        json_object *wpa_cli_pairwiseCipher_o = 
            json_object_object_get( wpa_cli_o, "pairwise_cipher" );
        if ( wpa_cli_pairwiseCipher_o == NULL ) {
            ZXDBFSStatus_free( status );
            return NULL;
        }
        strncpy( status->pairwiseCipher, json_object_get_string( wpa_cli_pairwiseCipher_o ), sizeof( status->pairwiseCipher ) );

        /** Group cipher */
        json_object *wpa_cli_groupCipher_o = 
            json_object_object_get( wpa_cli_o, "group_cipher" );
        if ( wpa_cli_groupCipher_o == NULL ) {
            ZXDBFSStatus_free( status );
            return NULL;
        }
        strncpy( status->groupCipher, json_object_get_string( wpa_cli_groupCipher_o ), sizeof( status->groupCipher ) );

        /** Key management */
        json_object *wpa_cli_keyManagement_o = 
            json_object_object_get( wpa_cli_o, "key_mgmt" );
        if ( wpa_cli_keyManagement_o == NULL ) {
            ZXDBFSStatus_free( status );
            return NULL;
        }
        strncpy( status->keyManagement, json_object_get_string( wpa_cli_keyManagement_o ), sizeof( status->keyManagement ) );

        /** State */
        json_object *wpa_cli_wpaState_o = 
            json_object_object_get( wpa_cli_o, "wpa_state" );
        if ( wpa_cli_wpaState_o == NULL ) {
            ZXDBFSStatus_free( status );
            return NULL;
        }
        strncpy( status->wpaState, json_object_get_string( wpa_cli_wpaState_o ), sizeof( status->wpaState ) );
    }

    /** Extract the process fields */
    json_object *ntpdok_o = json_object_object_get( statusRoot_o, "ntpdok" );
    if ( ntpdok_o == NULL ) {
        ZXDBFSStatus_free( status );
        return NULL;
    } else {
        status->ntpdok = json_object_get_int( ntpdok_o );
    }

    json_object *dateok_o = json_object_object_get( statusRoot_o, "dateok" );
    if ( dateok_o == NULL ) {
        ZXDBFSStatus_free( status );
        return NULL;
    } else {
        status->dateok = json_object_get_int( dateok_o );
    }

    json_object *zxdbfsdok_o = json_object_object_get( statusRoot_o, "zxdbfsdok" );
    if ( zxdbfsdok_o == NULL ) {
        ZXDBFSStatus_free( status );
        return NULL;
    } else {
        status->zxdbfsdok = json_object_get_int( zxdbfsdok_o );
    }
    
    json_object *spidok_o = json_object_object_get( statusRoot_o, "spidok" );
    if ( spidok_o == NULL ) {
        ZXDBFSStatus_free( status );
        return NULL;
    } else {
        status->spidok = json_object_get_int( spidok_o );
    }
    
    /** ZXDB version */
    json_object *zxdbVersion_o = 
        json_object_object_get( statusRoot_o, "zxdbversion" );
    if ( zxdbVersion_o == NULL ) {
        ZXDBFSStatus_free( status );
        return NULL;
    } else {
        strncpy( status->zxdbVersion, json_object_get_string( zxdbVersion_o ), sizeof( status->zxdbVersion ) );
    }
    
    return status;
}

