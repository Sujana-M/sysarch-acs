/** @file
 * Copyright (c) 2016-2018, 2023-2025, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "include/acs_val.h"
#include "include/acs_timer_support.h"
#include "include/acs_common.h"

/**
  @brief   This API is used to read Timer related registers

  @param   Reg  Register to be read

  @return  Register value
**/
uint64_t
ArmArchTimerReadReg (
    ARM_ARCH_TIMER_REGS   Reg
  )
{

    switch (Reg) {

    case CntFrq:
      return ArmReadCntFrq();

    case CntPct:
      return ArmReadCntPct();

    case CntkCtl:
      return ArmReadCntkCtl();

    case CntpTval:
      return ArmReadCntpTval();

    case CntpCtl:
      return ArmReadCntpCtl();

    case CntvTval:
      return ArmReadCntvTval();

    case CntvCtl:
      return ArmReadCntvCtl();

    case CntvCt:
      return ArmReadCntvCt();

    case CntpCval:
      return ArmReadCntpCval();

    case CntvCval:
      return ArmReadCntvCval();

    case CntvOff:
      return ArmReadCntvOff();
    case CnthpCtl:
      return ArmReadCnthpCtl();
    case CnthpTval:
      return ArmReadCnthpTval();
    case CnthvCtl:
      return ArmReadCnthvCtl();
    case CnthvTval:
      return ArmReadCnthvTval();

    case CnthCtl:
    case CnthpCval:
      val_print(ACS_PRINT_TEST, "The register is related to Hypervisor Mode. \
      Can't perform requested operation\n ", 0);
      break;

    default:
      val_print(ACS_PRINT_TEST, "Unknown ARM Generic Timer register %x.\n ", Reg);
    }

    return 0xFFFFFFFF;
}

/**
  @brief   This API is used to write Timer related registers

  @param   Reg  Register to be read
  @param   data_buf Data to write in register

  @return  None
**/
void
ArmArchTimerWriteReg (
    ARM_ARCH_TIMER_REGS   Reg,
    uint64_t              *data_buf
  )
{

    switch(Reg) {

    case CntPct:
      val_print(ACS_PRINT_TEST, "Can't write to Read Only Register: CNTPCT\n", 0);
      break;

    case CntkCtl:
      ArmWriteCntkCtl(*data_buf);
      break;

    case CntpTval:
      ArmWriteCntpTval(*data_buf);
      break;

    case CntpCtl:
      ArmWriteCntpCtl(*data_buf);
      break;

    case CntvTval:
      ArmWriteCntvTval(*data_buf);
      break;

    case CntvCtl:
      ArmWriteCntvCtl(*data_buf);
      break;

    case CntvCt:
       val_print(ACS_PRINT_TEST, "Can't write to Read Only Register: CNTVCT\n", 0);
      break;

    case CntpCval:
      ArmWriteCntpCval(*data_buf);
      break;

    case CntvCval:
      ArmWriteCntvCval(*data_buf);
      break;

    case CntvOff:
      ArmWriteCntvOff(*data_buf);
      break;

    case CnthpTval:
      ArmWriteCnthpTval(*data_buf);
      break;
    case CnthpCtl:
      ArmWriteCnthpCtl(*data_buf);
      break;
    case CnthvTval:
      ArmWriteCnthvTval(*data_buf);
      break;
    case CnthvCtl:
      ArmWriteCnthvCtl(*data_buf);
      break;
    case CnthCtl:
    case CnthpCval:
      val_print(ACS_PRINT_TEST, "The register is related to Hypervisor Mode. \
      Can't perform requested operation\n", 0);
      break;

    default:
      val_print(ACS_PRINT_TEST, "Unknown ARM Generic Timer register %x.\n ", Reg);
    }
}
