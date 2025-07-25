/** @file
 * Copyright (c) 2020,2021,2023-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/acs_pcie_enumeration.h"
#include "val/include/acs_pcie.h"
#include "val/include/acs_pe.h"
#include "val/include/acs_smmu.h"
#include "val/include/acs_memory.h"
#include "val/include/acs_exerciser.h"
#include "val/include/acs_pcie.h"
#include "val/include/val_interface.h"

#define TEST_NUM   (ACS_EXERCISER_TEST_NUM_BASE + 1)
#define TEST_RULE  "PCI_PP_04"
#define TEST_DESC  "Check P2P ACS Functionality           "

static
void
clear_error_status(uint32_t bdf)
{
  /* Clear Error Status Bits of the mentioned BDF */
  val_pcie_clear_device_status_error(bdf);
  val_pcie_clear_sig_target_abort(bdf);
  return;
}

static
uint32_t
get_target_exer_bdf(uint32_t req_rp_bdf, uint32_t *tgt_e_bdf,
                    uint32_t *tgt_rp_bdf, uint64_t *bar_base)
{

  uint32_t erp_bdf;
  uint32_t e_bdf;
  uint32_t instance;
  uint32_t cap_base;
  uint32_t req_rp_ecam_index;
  uint32_t erp_ecam_index;
  uint32_t status;

  instance = val_exerciser_get_info(EXERCISER_NUM_CARDS);

  while (instance-- != 0)
  {
      /* if init fail moves to next exerciser */
      if (val_exerciser_init(instance))
          continue;

      e_bdf = val_exerciser_get_bdf(instance);

      /* Read e_bdf BAR Register to get the Address to perform P2P */
      /* If No BAR Space, continue */
      val_pcie_get_mmio_bar(e_bdf, bar_base);
      if (*bar_base == 0)
          continue;

      /* Get RP of the exerciser */
      if (val_pcie_get_rootport(e_bdf, &erp_bdf))
          continue;

      /* It ACS Not Supported, continue */
      if (val_pcie_find_capability(erp_bdf, PCIE_ECAP, ECID_ACS, &cap_base) != PCIE_SUCCESS) {
          val_print(ACS_PRINT_DEBUG, "\n       ACS Not Supported for BDF : 0x%x", erp_bdf);
          continue;
      }

      if (req_rp_bdf != erp_bdf)
      {
          status = val_pcie_get_ecam_index(req_rp_bdf, &req_rp_ecam_index);
          if (status)
          {
             val_print(ACS_PRINT_ERR, "\n       Error Ecam index for req RP BDF: 0x%x", req_rp_bdf);
             goto clean_fail;
          }

          status = val_pcie_get_ecam_index(erp_bdf, &erp_ecam_index);
          if (status)
          {
             val_print(ACS_PRINT_ERR, "\n       Error Ecam index for tgt RP BDF: 0x%x", erp_bdf);
             goto clean_fail;
          }

          if (req_rp_ecam_index != erp_ecam_index)
              continue;

          *tgt_e_bdf = e_bdf;
          *tgt_rp_bdf = erp_bdf;

          /* Enable Bus Master Enable */
          val_pcie_enable_bme(e_bdf);
          /* Enable Memory Space Access */
          val_pcie_enable_msa(e_bdf);

          return ACS_STATUS_PASS;
      }
  }

clean_fail:
  /* Return failure if No Such Exerciser Found */
  *tgt_e_bdf = 0;
  *tgt_rp_bdf = 0;
  *bar_base = 0;
  return ACS_STATUS_FAIL;
}

static
uint32_t
check_source_validation (uint32_t req_instance, uint32_t req_rp_bdf, uint64_t bar_base)
{
  /* Check 1 : ACS Source Validation */

  uint32_t  reg_value;
  uint32_t  sub_bus;
  uint32_t  new_bdf;

  /* Clear error status bits of the Req RP*/
  clear_error_status(req_rp_bdf);

  /* Pass Sequence */
  val_exerciser_set_param(DMA_ATTRIBUTES, (uint64_t)bar_base, 1, req_instance);
  val_exerciser_ops(START_DMA, EDMA_FROM_DEVICE, req_instance);

  /* DMA should be successful and not trigger Corr/Uncorr error, UR response, Sig Target Abort. */
  if ((val_pcie_is_device_status_error(req_rp_bdf) != 0) ||
     (val_pcie_is_sig_target_abort(req_rp_bdf) != 0)) {
      val_print(ACS_PRINT_ERR,
                     "\n       Src Validation unexpected Error on RootPort : 0x%x", req_rp_bdf);
      return ACS_STATUS_FAIL;
  }

  /* Change Requester ID for DMA such that it does not fall under req_rp_bdf
   * Secondary & Subordinate bdf
  */
  val_pcie_read_cfg(req_rp_bdf, TYPE1_PBN, &reg_value);
  sub_bus = (reg_value >> SUBBN_SHIFT) & SUBBN_MASK;
  new_bdf = PCIE_CREATE_BDF(PCIE_EXTRACT_BDF_SEG(req_rp_bdf),
                            (sub_bus+1), 0, 0);
  new_bdf = PCIE_CREATE_BDF_PACKED(new_bdf);

  /* Clear error status bits of the Req RP*/
  clear_error_status(req_rp_bdf);

  val_exerciser_set_param(CFG_TXN_ATTRIBUTES, TXN_REQ_ID, new_bdf, req_instance);
  val_exerciser_set_param(DMA_ATTRIBUTES, (uint64_t)bar_base, 1, req_instance);

  val_exerciser_ops(START_DMA, EDMA_FROM_DEVICE, req_instance);

  /* Check For Error in Device Status Register / Status Register
   * Secondary Status Register
   */
  if ((val_pcie_is_device_status_error(req_rp_bdf) == 0) &&
     (val_pcie_is_sig_target_abort(req_rp_bdf) == 0)) {
      /* Fail the part */
      val_print(ACS_PRINT_ERR,
                     "\n       Src Validation Expected Error RootPort : 0x%x", req_rp_bdf);
      return ACS_STATUS_FAIL;
  }

  return ACS_STATUS_PASS;
}

static
uint32_t
check_transaction_blocking (uint32_t req_instance, uint32_t req_rp_bdf, uint64_t bar_base)
{
  /* Check 2 : ACS Transaction Blocking */

  /* Clear error status bits of the Req RP*/
  clear_error_status(req_rp_bdf);

  /* Transaction with Address Type other than default(0x0)
   * must result into Transaction blocking.
   */
  val_exerciser_set_param(CFG_TXN_ATTRIBUTES, TXN_ADDR_TYPE, AT_RESERVED, req_instance);
  val_exerciser_set_param(DMA_ATTRIBUTES, (uint64_t)bar_base, 1, req_instance);

  val_exerciser_ops(START_DMA, EDMA_FROM_DEVICE, req_instance);

  /* Check For Error in Device Status Register / Status Register
   * Secondary Status Register
   */
  if ((val_pcie_is_device_status_error(req_rp_bdf) == 0) &&
     (val_pcie_is_sig_target_abort(req_rp_bdf) == 0)) {
      /* Fail the part */
      val_print(ACS_PRINT_ERR,
                      "\n       Traxn Blocking Expected Error RootPort : 0x%x", req_rp_bdf);
      return ACS_STATUS_FAIL;
  }

  return ACS_STATUS_PASS;
}

static
void
payload(void)
{

  uint32_t status;
  uint32_t pe_index;
  uint32_t req_e_bdf;
  uint32_t req_rp_bdf;
  uint32_t tgt_e_bdf;
  uint32_t tgt_rp_bdf;
  uint32_t instance;
  uint32_t fail_cnt;
  uint32_t cap_base;
  uint32_t reg_value;
  uint32_t test_skip;
  uint64_t bar_base;
  uint32_t curr_bdf_failed = 0;
  pcie_device_bdf_table *bdf_tbl_ptr;
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  fail_cnt = 0;
  test_skip = 1;
  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
  instance = val_exerciser_get_info(EXERCISER_NUM_CARDS);

  uint32_t acsctrl_default[bdf_tbl_ptr->num_entries][1];

  /* Check If PCIe Hierarchy supports P2P. */
  if (val_pcie_p2p_support() == NOT_IMPLEMENTED) {
    val_print(ACS_PRINT_DEBUG, "\n       The test is applicable only if the system supports", 0);
    val_print(ACS_PRINT_DEBUG, "\n       P2P traffic. If the system supports P2P, pass the", 0);
    val_print(ACS_PRINT_DEBUG, "\n       command line option '-p2p' while running the binary", 0);
    val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  /* Store ACS Control reg bits in an array for every Exerciser and reset them
     to default at the end. */
  val_pcie_read_acsctrl(acsctrl_default);

  while (instance-- != 0)
  {

      /* if init fail moves to next exerciser */
      if (val_exerciser_init(instance))
          continue;

      req_e_bdf = val_exerciser_get_bdf(instance);
      val_print(ACS_PRINT_DEBUG, "\n       Requester exerciser BDF - 0x%x", req_e_bdf);

      /* Get RP of the exerciser */
      if (val_pcie_get_rootport(req_e_bdf, &req_rp_bdf))
          continue;

      /* It ACS Not Supported, Fail.*/
      if (val_pcie_find_capability(req_rp_bdf, PCIE_ECAP, ECID_ACS, &cap_base) != PCIE_SUCCESS) {
          val_print(ACS_PRINT_ERR, "\n       ACS Not Supported for BDF : 0x%x", req_rp_bdf);
          fail_cnt++;
          continue;
      }

      /* Enable Source Validation & Transaction Blocking */
      val_pcie_read_cfg(req_rp_bdf, cap_base + ACSCR_OFFSET, &reg_value);
      reg_value = reg_value | (1 << ACS_CTRL_SVE_SHIFT) | (1 << ACS_CTRL_TBE_SHIFT);
      val_pcie_write_cfg(req_rp_bdf, cap_base + ACSCR_OFFSET, reg_value);

      /* Find another exerciser on other rootport,
         Break from the test if no such exerciser if found */
      if (get_target_exer_bdf(req_rp_bdf, &tgt_e_bdf, &tgt_rp_bdf, &bar_base))
          continue;

      val_print(ACS_PRINT_DEBUG, "\n       Target exerciser BDF - 0x%x", tgt_e_bdf);

      /* Enable Source Validation & Transaction Blocking */
      val_pcie_read_cfg(tgt_rp_bdf, cap_base + ACSCR_OFFSET, &reg_value);
      reg_value = reg_value | (1 << ACS_CTRL_SVE_SHIFT) | (1 << ACS_CTRL_TBE_SHIFT);
      val_pcie_write_cfg(tgt_rp_bdf, cap_base + ACSCR_OFFSET, reg_value);
      test_skip = 0;

      /* Check For ACS Functionality */
      status = check_source_validation(instance, req_rp_bdf, bar_base);
      if (status == ACS_STATUS_SKIP)
          val_print(ACS_PRINT_DEBUG, "\n       ACS Source Validation Skipped for 0x%x", req_rp_bdf);
      else if (status)
          curr_bdf_failed++;

      val_exerciser_set_param(CFG_TXN_ATTRIBUTES, TXN_REQ_ID, RID_NOT_VALID, instance);

      status = check_transaction_blocking(instance, req_rp_bdf, bar_base);
      if (status == ACS_STATUS_SKIP)
          val_print(ACS_PRINT_DEBUG,
                    "\n       ACS Transaction Blocking Skipped for 0x%x", req_rp_bdf);
      else if (status)
          curr_bdf_failed++;

      if (curr_bdf_failed > 0) {
          val_print(ACS_PRINT_ERR,
                    "\n       ACS Functional Check Failed, RP Bdf : 0x%x",
                    req_rp_bdf);
          curr_bdf_failed = 0;
          fail_cnt++;
      }

      /* Clear error status bits of the Req RP*/
      clear_error_status(req_rp_bdf);
  }

  /* Write back default values of ACS Control reg. */
  val_pcie_write_acsctrl(acsctrl_default);

  if (test_skip == 1)
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 2));
  else if (fail_cnt)
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, fail_cnt));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 1));

  return;

}

uint32_t
e001_entry(void)
{
  uint32_t num_pe = 1;
  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* Get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
