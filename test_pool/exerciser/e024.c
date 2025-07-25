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
#include "val/include/acs_pcie_enumeration.h"
#include "val/include/acs_iovirt.h"
#include "val/include/acs_pcie.h"
#include "val/include/acs_pe.h"
#include "val/include/acs_smmu.h"
#include "val/include/acs_memory.h"
#include "val/include/acs_exerciser.h"
#include "val/include/val_interface.h"

#define TEST_NUM   (ACS_EXERCISER_TEST_NUM_BASE + 24)
#define TEST_DESC  "RP's must support DPC                 "
#define TEST_RULE  "PCI_ER_05, PCI_ER_06"

#define ERR_FATAL 1
#define ERR_FATAL_NONFATAL 2
#define ERR_UNCORR   0x3
#define MAX_DEVICES  256

static uint32_t msg_type[] = {ERR_FATAL_NONFATAL, ERR_FATAL};
static uint32_t irq_pending;
static uint32_t lpi_int_id = 0x204C;

/* Allocating memory only for 256 devices */
static void     *cfg_space_buf[MAX_DEVICES];

static
void
intr_handler(void)
{
  /* Clear the interrupt pending state */
  irq_pending = 0;

  val_print(ACS_PRINT_INFO, "\n       Received MSI interrupt %x       ", lpi_int_id);
  val_gic_end_of_interrupt(lpi_int_id);
  return;
}

static uint32_t
restore_config_space(uint32_t rp_bdf)
{

  uint32_t idx;
  uint32_t bdf, dev_rp_bdf;
  uint32_t tbl_index = 0;
  addr_t   cfg_space_addr;

  pcie_device_bdf_table *bdf_tbl_ptr;
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();
  while (tbl_index < bdf_tbl_ptr->num_entries)
  {
      bdf = bdf_tbl_ptr->device[tbl_index++].bdf;

      if (val_pcie_get_rootport(bdf, &dev_rp_bdf))
              continue;

      /* Check if the RP of the device matches with the rp_bdf */
      if (rp_bdf != dev_rp_bdf)
              continue;

      /* Traverse through the devices under this RP and restore its config space */
      cfg_space_addr = val_pcie_get_bdf_config_addr(bdf);
      /* Restore the EP config space after Secondary Bus Reset */
      for (idx = 0; idx < PCIE_CFG_SIZE/4; idx++) {
          *((uint32_t *)cfg_space_addr + idx) = *(((uint32_t *)(cfg_space_buf[tbl_index])) + idx);
      }

      val_memory_free_aligned(cfg_space_buf[tbl_index]);
   }
  return 0;
}

static uint32_t
save_config_space(uint32_t rp_bdf)
{

  uint32_t idx;
  uint32_t bdf, dev_rp_bdf;
  uint32_t tbl_index = 0;
  addr_t   cfg_space_addr;
  uint32_t pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  pcie_device_bdf_table *bdf_tbl_ptr;
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();
  if (bdf_tbl_ptr->num_entries > MAX_DEVICES) {
      val_print(ACS_PRINT_WARN, "\n WARNING: Memory is allocated only for %d devices", MAX_DEVICES);
      val_print(ACS_PRINT_WARN, "\n The number of PCIe devices is %d", bdf_tbl_ptr->num_entries);
      val_print(ACS_PRINT_WARN, "\n for which the additional memory is not allocated", 0);
      val_print(ACS_PRINT_WARN, "\n and test may fail\n", 0);
  }

  while (tbl_index < bdf_tbl_ptr->num_entries)
  {
      bdf = bdf_tbl_ptr->device[tbl_index++].bdf;

      if (val_pcie_get_rootport(bdf, &dev_rp_bdf))
              continue;

      /* Check if the RP of the device matches with the rp_bdf */
      if (rp_bdf != dev_rp_bdf)
              continue;

      /* Traverse through the devices under this RP and store its config space. When SBR is
         performed, all the devices connected below the RP is reset. This needs to be restored
         after SBR*/
      cfg_space_buf[tbl_index] = val_aligned_alloc(MEM_ALIGN_4K, PCIE_CFG_SIZE);
      if (cfg_space_buf[tbl_index] == NULL)
      {
          val_print(ACS_PRINT_ERR, "\n       Memory allocation failed.", 0);
          val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
          return 1;
      }

      cfg_space_addr = val_pcie_get_bdf_config_addr(bdf);
      /* Save the EP config space to restore after Secondary Bus Reset */
      for (idx = 0; idx < PCIE_CFG_SIZE/4; idx++) {
          *(((uint32_t *)(cfg_space_buf[tbl_index])) + idx) = *((uint32_t *)cfg_space_addr + idx);
      }
   }
  return 0;
}


static
void
payload(void)
{

  uint32_t pe_index;
  uint32_t e_bdf;
  uint32_t erp_bdf;
  uint32_t reg_value;
  uint32_t instance;
  uint32_t fail_cnt;
  uint32_t test_skip = 1;
  uint32_t status;
  uint32_t rp_dpc_cap_base;
  uint32_t aer_offset;
  uint32_t rp_aer_offset;
  uint32_t error_source_id;
  uint32_t source_id;
  uint32_t dpc_trigger_reason;
  uint32_t timeout;
  uint32_t msi_check = 0;

  uint32_t device_id = 0;
  uint32_t stream_id = 0;
  uint32_t its_id = 0;
  uint32_t msi_index = 0;
  uint32_t msi_cap_offset = 0;

  fail_cnt = 0;
  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
  instance = val_exerciser_get_info(EXERCISER_NUM_CARDS);

  while (instance-- != 0)
  {
      if (val_exerciser_init(instance))
          continue;

      e_bdf = val_exerciser_get_bdf(instance);
      val_print(ACS_PRINT_DEBUG, "\n       Exerciser BDF - 0x%x", e_bdf);

      val_pcie_enable_eru(e_bdf);

      if (val_pcie_get_rootport(e_bdf, &erp_bdf))
          continue;
      val_pcie_enable_eru(erp_bdf);

      /* Check DPC capability */
      status = val_pcie_find_capability(erp_bdf, PCIE_ECAP, ECID_DPC, &rp_dpc_cap_base);
      if (status == PCIE_CAP_NOT_FOUND)
      {
          val_print(ACS_PRINT_ERR, "\n       ECID_DPC not found", 0);
          continue;
      }

      /* Check AER capability for both exerciser and RP */
      if (val_pcie_find_capability(e_bdf, PCIE_ECAP, ECID_AER, &aer_offset) != PCIE_SUCCESS) {
          val_print(ACS_PRINT_ERR, "\n       AER Capability not supported, Bdf : 0x%x", e_bdf);
          continue;
      }

      if (val_pcie_find_capability(erp_bdf, PCIE_ECAP, ECID_AER, &rp_aer_offset) != PCIE_SUCCESS) {
          val_print(ACS_PRINT_ERR, "\n       AER Capability not supported for RP : 0x%x", erp_bdf);
          fail_cnt++;
      }

      /* Search for MSI/MSI-X Capability */
      if ((val_pcie_find_capability(erp_bdf, PCIE_CAP, CID_MSIX, &msi_cap_offset)) &&
        (val_pcie_find_capability(erp_bdf, PCIE_CAP, CID_MSI, &msi_cap_offset))) {
        val_print(ACS_PRINT_ERR, "\n       No MSI/MSI-X Capability for Bdf 0x%x", erp_bdf);
        goto err_check;
      }

      msi_check = 1;
      /* Get DeviceID & ITS_ID for this device */
      status = val_iovirt_get_device_info(PCIE_CREATE_BDF_PACKED(erp_bdf),
                                        PCIE_EXTRACT_BDF_SEG(erp_bdf), &device_id,
                                        &stream_id, &its_id);

      if (status) {
          val_print(ACS_PRINT_ERR, "\n       iovirt_get_device failed for bdf 0x%x", e_bdf);
          val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
          return;
      }

       /* Get DeviceID & ITS_ID for this device */
      status = val_gic_request_msi(erp_bdf, device_id, its_id, lpi_int_id + instance, msi_index);
      if (status) {
          val_print(ACS_PRINT_ERR, "\n       MSI Assignment failed for bdf : 0x%x", erp_bdf);
          val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 2));
          return;
      }

      status = val_gic_install_isr(lpi_int_id + instance, intr_handler);

      if (status) {
          val_print(ACS_PRINT_ERR, "\n       Intr handler registration failed: 0x%x", lpi_int_id);
          val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
          return;
      }

err_check:
      status = val_exerciser_set_param(ERROR_INJECT_TYPE, UNCORR_CMPT_TO, 1, instance);
      if (status != ERR_UNCORR) {
          val_print(ACS_PRINT_ERR, "\n       Error Injection failed, Bdf : 0x%x", e_bdf);
          continue;
      }

      test_skip = 0;

      /* check for both fatal and non-fatal error */
      for (int i = 0; i < 2; i++)
      {
          val_pcie_data_link_layer_status(erp_bdf);

          /* Save the config space of all the devices connected to the RP
           to restore after Secondary Bus Reset (SBR)*/
          save_config_space(erp_bdf);
          val_print(ACS_PRINT_INFO, "       EP BDF : 0x%x\n", e_bdf);

          irq_pending = 1;
          val_pcie_read_cfg(erp_bdf, rp_dpc_cap_base + DPC_CTRL_OFFSET, &reg_value);
          reg_value &= DPC_DISABLE_MASK;
          reg_value |= DPC_INTR_ENABLE;
          reg_value = reg_value | (msg_type[i] << DPC_CTRL_TRG_EN_SHIFT);
          val_pcie_write_cfg(erp_bdf, rp_dpc_cap_base + DPC_CTRL_OFFSET, reg_value);

          val_pcie_read_cfg(erp_bdf, rp_dpc_cap_base + DPC_CTRL_OFFSET, &reg_value);

          if (msg_type[i] == ERR_FATAL)
          {
              val_pcie_write_cfg(e_bdf, aer_offset + AER_UNCORR_SEVR_OFFSET, AER_UNCORR_SEVR_FATAL);
              val_pcie_write_cfg(e_bdf, aer_offset + AER_UNCORR_MASK_OFFSET, 0x0);
          }
          else
          {
              val_pcie_write_cfg(e_bdf, aer_offset + AER_UNCORR_SEVR_OFFSET, 0x0);
          }

          /*Inject error immediately*/
          val_exerciser_ops(INJECT_ERROR, CFG_READ, instance);

          val_pcie_read_cfg(e_bdf, CFG_READ, &reg_value);
          if (reg_value != PCIE_UNKNOWN_RESPONSE)
          {
              val_print(ACS_PRINT_ERR, "\n       EP not contained due to DPC", 0);
              fail_cnt++;
          }

          val_pcie_read_cfg(erp_bdf, rp_dpc_cap_base + DPC_STATUS_OFFSET, &reg_value);

          /* Check DPC Trigger status */
          if ((reg_value & 1) == 0)
          {
              val_print(ACS_PRINT_ERR, "\n       DPC Trigger status bit not set %x", reg_value);
              fail_cnt++;
          }

          dpc_trigger_reason = (reg_value & DPC_TRIGGER_MASK) >> 1;
          if (msg_type[i] == ERR_FATAL)
          {
              if (dpc_trigger_reason != 2)
              {
                  val_print(ACS_PRINT_ERR, "\n       DPC Trigger reason incorrect", 0);
                  fail_cnt++;
              }
          } else {
              if (dpc_trigger_reason != 1)
              {
                  val_print(ACS_PRINT_ERR, "\n       DPC Trigger reason incorrect", 0);
                  fail_cnt++;
              }
          }

          source_id = PCIE_CREATE_BDF_PACKED(e_bdf);
          error_source_id = (reg_value >> DPC_SOURCE_ID_SHIFT);
          if (source_id != error_source_id)
          {
              val_print(ACS_PRINT_ERR, "\n       DPC Error source Identification failed", 0);
              fail_cnt++;
          }

          if (msi_check == 1)
          {
              timeout = TIMEOUT_LARGE;
              while ((--timeout > 0) && irq_pending)
              {};

              if (timeout == 0) {
                  val_gic_free_irq(irq_pending, 0);
                  val_print(ACS_PRINT_ERR, "\n       Interrupt trigger failed for bdf 0x%x", e_bdf);
                  fail_cnt++;
                  continue;
              }
          }

          val_pcie_read_cfg(erp_bdf, rp_dpc_cap_base + DPC_STATUS_OFFSET, &reg_value);
          while (reg_value & 0x10)
          {};
          val_pcie_write_cfg(erp_bdf, rp_dpc_cap_base + DPC_STATUS_OFFSET, 1);

          val_pcie_read_cfg(erp_bdf, TYPE01_ILR, &reg_value);
          reg_value = reg_value | BRIDGE_CTRL_SBR_SET;
          val_pcie_write_cfg(erp_bdf, TYPE01_ILR, reg_value);

          /* Wait for Timeout */
          val_time_delay_ms(2 * ONE_MILLISECOND);

          val_pcie_read_cfg(erp_bdf, TYPE01_ILR, &reg_value);
          reg_value = reg_value & ~BRIDGE_CTRL_SBR_SET;
          val_pcie_write_cfg(erp_bdf, TYPE01_ILR, reg_value);

          timeout = TIMEOUT_LARGE;
          while (--timeout)
          {};

          status = val_pcie_data_link_layer_status(erp_bdf);
          if (status != PCIE_DLL_LINK_ACTIVE_NOT_SUPPORTED)
          {
              if (!status)
              {
                  /* Wait for for additional Timeout and check the status*/
                  uint32_t delay_status = val_time_delay_ms(100 * ONE_MILLISECOND);
                  if (!delay_status)
                  {
                      val_print(ACS_PRINT_ERR,
                               "\n       Failed to time delay for BDF 0x%x ", erp_bdf);
                      val_memory_free_aligned(cfg_space_buf);
                      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
                      return;
                  }

                  status = val_pcie_data_link_layer_status(erp_bdf);
              }
          }

          if (status == PCIE_DLL_LINK_STATUS_NOT_ACTIVE)
          {
              val_print(ACS_PRINT_ERR,
                       "\n       The link not active after reset for BDF 0x%x: ", erp_bdf);
              return ;
          }

          /*Disable the DPC*/

          val_pcie_read_cfg(erp_bdf, rp_dpc_cap_base + DPC_STATUS_OFFSET, &reg_value);
          val_pcie_write_cfg(erp_bdf, rp_dpc_cap_base + DPC_STATUS_OFFSET, reg_value | 0x1);

          val_pcie_read_cfg(erp_bdf, rp_dpc_cap_base + DPC_CTRL_OFFSET, &reg_value);
          val_pcie_write_cfg(erp_bdf, rp_dpc_cap_base + DPC_CTRL_OFFSET, reg_value & 0xFFFCFFFF);

          /* Restore the EP config space after Secondary Bus Reset */
          restore_config_space(erp_bdf);
          val_pcie_read_cfg(e_bdf, aer_offset + AER_UNCORR_STATUS_OFFSET, &reg_value);
          val_pcie_write_cfg(e_bdf, aer_offset + AER_UNCORR_STATUS_OFFSET, reg_value & 0xFFFFFFFF);

          val_pcie_read_cfg(e_bdf, CFG_READ, &reg_value);
          if (reg_value == PCIE_UNKNOWN_RESPONSE)
          {
              val_print(ACS_PRINT_ERR, "\n       EP not recovered from DPC %x", e_bdf);
              fail_cnt++;
          }

      }
  }

  if (test_skip)
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
  else if (fail_cnt)
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, fail_cnt));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));

  return;

}

uint32_t
e024_entry(void)
{
  uint32_t num_pe = 1;
  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* Get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), TEST_RULE);

  return status;
}
