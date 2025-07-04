/** @file
 * Copyright (c) 2023-2025, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/acs_val.h"
#include "val/include/acs_pe.h"
#include "val/include/val_interface.h"
#include "val/include/acs_pcie.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 86)
#define TEST_DESC  "Check RootPort P&NP Memory Access     "
#define TEST_RULE  "S_PCIe_02"

static void *branch_to_test;
static uint32_t bdf;
static uint32_t tbl_index;
static pcie_device_bdf_table *bdf_tbl_ptr;


static
void
esr(uint64_t interrupt_type, void *context)
{
  uint32_t pe_index;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Update the ELR to return to test specified address */
  val_pe_update_elr(context, (uint64_t)branch_to_test);

  val_print(ACS_PRINT_INFO, "\n       Received exception of type in test 861: %d", interrupt_type);
  val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
}

static uint32_t test_sequence_1B(uint8_t *addr)
{
  uint32_t write_val = 0xAB;
  uint32_t read_value, old_value;
  uint32_t pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_print(ACS_PRINT_INFO, "\n       testing 1B test sequence", 0);
  for (int idx = 0; idx < 8; idx++) {
      old_value = val_mmio_read8((addr_t)addr);
      val_mmio_write8((addr_t)addr, write_val);
      read_value = val_mmio_read8((addr_t)addr);

      if ((old_value != read_value && read_value == PCIE_UNKNOWN_RESPONSE)) {
        val_print(ACS_PRINT_ERR, "\n       Error in read and write 1B", 0);
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
        return 1;
      }

      val_mmio_write8((addr_t)addr, old_value);
      addr++;
  }

  return 0;
}

uint32_t test_sequence_2B(uint16_t *addr)
{
  uint32_t write_val = 0xABCD;
  uint32_t read_value, old_value;
  uint32_t pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
 
  val_print(ACS_PRINT_INFO, "\n       testing 2B test sequence", 0);
  for (int idx = 0; idx < 4; idx++) {
      old_value = val_mmio_read16((addr_t)addr);
      val_mmio_write16((addr_t)addr, write_val);
      read_value = val_mmio_read16((addr_t)addr);

      if ((old_value != read_value && read_value == PCIE_UNKNOWN_RESPONSE)) {
        val_print(ACS_PRINT_ERR, "\n       Error in read and write 2B", 0);
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 03));
        return 1;
      }

      val_mmio_write16((addr_t)addr, old_value);
      addr++;
  }

  return 0;
}

static
void
payload(void)
{
  uint32_t pe_index;
  uint32_t bar_data;
  uint32_t test_fails;
  uint32_t test_skip = 1;
  uint64_t bar_base = 0;
  uint32_t status;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  /* Install sync and async handlers to handle exceptions.*/
  status = val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
  status |= val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);
  if (status)
  {
      val_print(ACS_PRINT_ERR, "\n       Failed in installing the exception handler", 0);
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
      return;
  }

  branch_to_test = &&exception_return861;

  bar_data = 0;
  tbl_index = 0;
  test_fails = 0;

  while (tbl_index < bdf_tbl_ptr->num_entries)
  {
      val_print(ACS_PRINT_DEBUG, "\n       tbl_index - 0x%x", tbl_index);
      bdf = bdf_tbl_ptr->device[tbl_index++].bdf;

      val_print(ACS_PRINT_DEBUG, "\n       BDF - 0x%x", bdf);
      /*
       * For Function with Type 1 config space header, obtain
       * base address of the its own BAR address.
       */
      if (val_pcie_function_header_type(bdf) == TYPE0_HEADER) {
          val_print(ACS_PRINT_DEBUG, "\n  Skipping not a RP, BDF - 0x%x", bdf);
          continue;
      }
         
      val_pcie_get_mmio_bar(bdf, &bar_base);
      /* Skip this function if it doesn't have mmio BAR */
      if (!bar_base) {
         val_print(ACS_PRINT_DEBUG, "\n  Skipping as no MMIO BAR, RP BDF - 0x%x", bdf);
         continue;
      }

      /* If test runs for atleast an endpoint */
      test_skip = 0;

      bar_data = val_mmio_read(bar_base);

      if (test_sequence_1B((uint8_t *)bar_base)) {
          val_print(ACS_PRINT_ERR, "\n       Failed check for Bdf 0x%x", bdf);
          test_fails++;
      }

      if (test_sequence_2B((uint16_t *)bar_base)) {
          val_print(ACS_PRINT_ERR, "\n       Failed check for Bdf 0x%x", bdf);
          test_fails++;
      }
      val_print(ACS_PRINT_DEBUG, "\n       Doing write of bar base Bdf 0x%x", bdf);
      val_mmio_write(bar_base, bar_data);

exception_return861:
      if (IS_TEST_FAIL(val_get_status(pe_index))) {
        val_print(ACS_PRINT_ERR, "\n       Failed. Exception on Memory Access For Bdf 0x%x", bdf);
        val_pcie_clear_urd(bdf);
        test_fails++;
      }

       /* Reset the loop variables */
       bar_data = 0;
  }

  if (test_skip == 1) {
      val_print(ACS_PRINT_DEBUG, "\n       No MMIO BARs detected. Skipping test", 0);
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
  }
  else if (test_fails)
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, test_fails));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
}

uint32_t
p086_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), TEST_RULE);

  return status;
}
