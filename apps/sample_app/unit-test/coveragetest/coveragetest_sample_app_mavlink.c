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

/*
** File: coveragetest_sample_app_mavlink.c
**
** Purpose:
** Coverage build placeholder for sample_app_mavlink.c
**
** Notes:
** SAMPLE_APP_MavlinkTask() talks directly to a real serial device via
** POSIX open/termios/read calls (not OSAL/CFE stubs), and runs an
** infinite blocking read loop by design - it is not a meaningful
** target for the stub-based coverage harness used elsewhere in this
** app. This file only exists to satisfy the per-source-file coverage
** test convention in unit-test/CMakeLists.txt.
*/

#include "sample_app_coveragetest_common.h"

void UtTest_Setup(void) {}
