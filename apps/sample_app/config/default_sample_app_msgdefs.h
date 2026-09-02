/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Specification for the SAMPLE_APP command and telemetry
 *   message payload and constant definitions.
 */
#ifndef DEFAULT_SAMPLE_APP_MSGDEFS_H
#define DEFAULT_SAMPLE_APP_MSGDEFS_H

#include "common_types.h"
#include "sample_app_fcncodes.h"

typedef struct SAMPLE_APP_DisplayParam_Payload
{
    uint32 ValU32;                                    /**< 32 bit unsigned integer value */
    int16  ValI16;                                    /**< 16 bit signed integer value */
    char   ValStr[SAMPLE_APP_MISSION_STRING_VAL_LEN]; /**< An example string */
} SAMPLE_APP_DisplayParam_Payload_t;

/*************************************************************************/
/*
** Type definition (Sample App housekeeping)
*/

typedef struct SAMPLE_APP_HkTlm_Payload
{
    uint8 CommandCounter;
    uint8 CommandErrorCounter;
} SAMPLE_APP_HkTlm_Payload_t;
typedef struct SAMPLE_APP_DataTlm_Payload
{
    uint32 PingCounter; /**< cuántos PINGs se han recibido y respondido */
} SAMPLE_APP_DataTlm_Payload_t;

/* --- NUEVO: telemetria de sensores leidos del F405 via MAVLink --- */
typedef struct SAMPLE_APP_SensorsTlm_Payload
{
    uint32 LastAttitudeMs;     /**< time_boot_ms del ultimo ATTITUDE recibido */

    float  Roll;               /**< rad, MAVLink ATTITUDE */
    float  Pitch;               /**< rad */
    float  Yaw;                 /**< rad */

    uint16 BatteryVoltageMv;    /**< mV, MAVLink SYS_STATUS */
    int16  BatteryCurrentCa;    /**< cA (centiamperios), -1 = desconocido */
    int8   BatteryRemaining;    /**< %, -1 = desconocido */

    int32  Lat;                 /**< degE7, MAVLink GPS_RAW_INT */
    int32  Lon;                 /**< degE7 */
    int32  AltMsl;               /**< mm */
    uint8  GpsFixType;
    uint8  SatellitesVisible;

    float  Groundspeed;          /**< m/s, MAVLink VFR_HUD */
    float  Airspeed;              /**< m/s */
    int16  Heading;               /**< deg */

    bool   HeartbeatSeen;          /**< true si se recibio al menos un HEARTBEAT */
} SAMPLE_APP_SensorsTlm_Payload_t;
#endif
