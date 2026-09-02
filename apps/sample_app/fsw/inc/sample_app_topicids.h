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
 *   SAMPLE_APP Application Topic IDs
 */

#ifndef SAMPLE_APP_TOPICIDS_H
#define SAMPLE_APP_TOPICIDS_H

#include "sample_app_topicid_values.h"

#define SAMPLE_APP_MISSION_CMD_TOPICID             SAMPLE_APP_MISSION_TIDVAL(CMD)
#define DEFAULT_SAMPLE_APP_MISSION_CMD_TOPICID     0x82
#define SAMPLE_APP_MISSION_SEND_HK_TOPICID         SAMPLE_APP_MISSION_TIDVAL(SEND_HK)
#define DEFAULT_SAMPLE_APP_MISSION_SEND_HK_TOPICID 0x83
#define SAMPLE_APP_MISSION_HK_TLM_TOPICID          SAMPLE_APP_MISSION_TIDVAL(HK_TLM)
#define DEFAULT_SAMPLE_APP_MISSION_HK_TLM_TOPICID  0x83

/* --- NUEVO: topic para nuestra telemetría de respuesta al PING --- */
#define SAMPLE_APP_MISSION_DATA_TLM_TOPICID         SAMPLE_APP_MISSION_TIDVAL(DATA_TLM)
#define DEFAULT_SAMPLE_APP_MISSION_DATA_TLM_TOPICID 0x8F

/* --- NUEVO: topic para la telemetría de sensores del F405 (MAVLink) ---
 * NOTA: 0x90 ya estaba en uso por MD_HK_TLM_TOPICID (apps/md), causaba una
 * colision de MID silenciosa (0x0890 en ambos). Los topic ID son un
 * espacio de nombres global de la mision, no por-app; 0x93 se verifico
 * libre contra todos los topicids.h del repo. */
#define SAMPLE_APP_MISSION_SENSORS_TLM_TOPICID         SAMPLE_APP_MISSION_TIDVAL(SENSORS_TLM)
#define DEFAULT_SAMPLE_APP_MISSION_SENSORS_TLM_TOPICID 0x93

#endif