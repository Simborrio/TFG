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
 *
 * Main header file for the Sample application
 */

#ifndef SAMPLE_APP_H
#define SAMPLE_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "sample_app_mission_cfg.h"
#include "sample_app_platform_cfg.h"

#include "sample_app_perfids.h"
#include "sample_app_msgids.h"
#include "sample_app_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Valores en crudo leidos por la tarea MAVLink, protegidos por
** SensorCacheMutex. El comando GET_SENSORS_CC copia esto al payload
** de telemetria bajo el mismo mutex.
*/
typedef struct
{
    uint32 LastAttitudeMs;

    float Roll;
    float Pitch;
    float Yaw;

    uint16 BatteryVoltageMv;
    int16  BatteryCurrentCa;
    int8   BatteryRemaining;

    int32 Lat;
    int32 Lon;
    int32 AltMsl;
    uint8 GpsFixType;
    uint8 SatellitesVisible;

    float Groundspeed;
    float Airspeed;
    int16 Heading;

    bool HeartbeatSeen;
} SAMPLE_APP_SensorCache_t;

/*
** Global Data
*/
typedef struct
{
    uint8 CommandCounter;
    uint8 CommandErrorCounter;

    SAMPLE_APP_HkTlm_t HkTlm;

    /* --- NUEVO --- */
    SAMPLE_APP_DataTlm_t DataTlm;
    uint32               PingCounter;   /* contador propio de pings */

    /* --- NUEVO: sensores del F405 via MAVLink --- */
    SAMPLE_APP_SensorsTlm_t  SensorsTlm;
    SAMPLE_APP_SensorCache_t SensorCache;
    osal_id_t                SensorCacheMutex;
    CFE_ES_TaskId_t          MavlinkTaskId;

    uint32 RunStatus;

    CFE_SB_PipeId_t CommandPipe;

    CFE_TBL_Handle_t TblHandles[SAMPLE_APP_PLATFORM_NUMBER_OF_TABLES];
} SAMPLE_APP_Data_t;
/*
** Global data structure
*/
extern SAMPLE_APP_Data_t SAMPLE_APP_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_APP_Main), these
**       functions are not called from any other source module.
*/
void         SAMPLE_APP_Main(void);
CFE_Status_t SAMPLE_APP_Init(void);
void         SAMPLE_APP_MavlinkTask(void);

#endif /* SAMPLE_APP_H */
