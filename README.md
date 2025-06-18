#  Unified Compliance Test Suites for Arm System Architecture Standards

## Table of Contents

- [Introduction to Arm sysarch-acs](#introduction)
- [Unified Architecture Compliance Suite](#unified-architecture-compliance-suite)
  - [Build Steps](#build-steps1)
- [BSA Architecture Compliance Suite](#bsa-architecture-compliance-suite)
  - [Build Steps](#build-steps1)
  - [Release details](#release-details)
  - [Limitations](#limitations)
- [SBSA Architecture Compliance Suite](#sbsa-architecture-compliance-suite)
  - [Build Steps](#build-steps1)
  - [Release details](#release-details)
  - [Limitations](#limitations)
- [DRTM ACS](#drtm-system-mpam-and-mem_test)
- [System MPAM](#drtm-system-mpam-and-mem_test)
- [mem_test](#drtm-system-mpam-and-mem_test)

## Introduction to Arm sysarch-acs

This repository is a centralized collection of **compliance test suites** designed to validate conformance to a range of **Arm System Architecture Specifications**. It brings together reference implementations for verifying platform behavior against the following standards:

* **BSA** (Base System Architecture)
* **SBSA** (Server Base System Architecture)
* **SYS-MPAM** (System-level Memory Partitioning and Monitoring)
* **DRTM** (Dynamic Root of Trust for Measurement)

The goal of this repository is to provide a consistent and reusable framework for **system integrators**, **silicon vendors**, **firmware developers**, and **platform validation teams** to ensure their implementations meet architectural compliance requirements. These suites are platform-agnostic and can be integrated into both development and manufacturing validation pipelines.

## Unified Architecture Compliance Suite
Unified **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/d/?lang=en), [SBSA](https://developer.arm.com/documentation/den0029/i/?lang=en) specification, so that implementers can verify if these behaviours have been interpreted correctly.

Most of the tests are executed from UEFI (Unified Extensible Firmware Interface) Shell by executing the ACS UEFI shell application.
A few tests are executed by running the BSA and SBSA ACS Linux application which in turn depends on the BSA and SBSA ACS Linux kernel module.
The tests can also be executed in a Bare-metal environment. The initialization of the Bare-metal environment is specific to the environment and is out of scope of this document.

### Build steps
#### UEFI Shell application
##### 1. Building from source
    Before you start the ACS build, ensure that the following requirements are met.

- Any mainstream Linux-based OS distribution running on a x86 or AArch64 machine.
- git clone the [EDK2 tree](https://github.com/tianocore/edk2). Recommended edk2 commit is 836942fbadb629050b866a8052e6af755bcdf623
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) to local <edk2_path>.
- Install GCC-ARM 13.2 [toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
- Install the build prerequisite packages to build EDK2.<br />
Note:<br />
- The details of the packages are beyond the scope of this document.

##### 1.1 Target Platform
###### To start the ACS build for platform using ACPI table, perform the following steps:

1.  cd local_edk2_path
2.  git submodule update --init --recursive
3.  git clone https://github.com/ARM-software/sysarch-acs.git ShellPkg/Application/sysarch-acs

##### 1.2 Build environment
###### If the build environment is Linux, perform the following steps:
1.  export GCC49_AARCH64_PREFIX= GCC13.2 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine.<br /> For an AArch64 build it should point to /usr/bin/
2.  export PACKAGES_PATH= path pointing to edk2-libc
3.  source edksetup.sh
4.  make -C BaseTools/Source/C
5.  source ShellPkg/Application/sysarch-acs/tools/scripts/acsbuild.sh unified 

##### 1.3 Build output

The EFI executable file is generated at <edk2_path>/Build/Shell/DEBUG_GCC49/AARCH64/UnifiedAcs.efi

## BSA Architecture Compliance Suite

BSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/d/?lang=en) specification, so that implementers can verify if these behaviours have been interpreted correctly.

Most of the tests are executed from UEFI (Unified Extensible Firmware Interface) Shell by executing the ACS UEFI shell application.
A few tests are executed by running the BSA ACS Linux application which in turn depends on the BSA ACS Linux kernel module.
The tests can also be executed in a Bare-metal environment. The initialization of the Bare-metal environment is specific to the environment and is out of scope of this document.

### Build steps
#### UEFI Shell application

##### 1. Building from source
    Before you start the ACS build, ensure that the following requirements are met.

- Any mainstream Linux-based OS distribution running on a x86 or AArch64 machine.
- git clone the [EDK2 tree](https://github.com/tianocore/edk2). Recommended edk2 commit is 836942fbadb629050b866a8052e6af755bcdf623
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) to local <edk2_path>.
- Install GCC-ARM 13.2 [toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
- Install the build prerequisite packages to build EDK2.<br />
Note:<br />
- The details of the packages are beyond the scope of this document.

##### 1.1 Target Platform
###### To start the ACS build for platform using ACPI table, perform the following steps:

1.  cd local_edk2_path
2.  git submodule update --init --recursive
3.  git clone https://github.com/ARM-software/sysarch-acs.git ShellPkg/Application/sysarch-acs

###### To start the ACS build for platform using Device tree, perform the following steps:

1.  cd local_edk2_path
2.  git submodule update --init --recursive
3.  git clone https://github.com/ARM-software/sysarch-acs.git ShellPkg/Application/sysarch-acs

#####    1.2 Build environment
###### If the build environment is Linux, perform the following steps:
1.  export GCC49_AARCH64_PREFIX= GCC13.2 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine.<br /> For an AArch64 build it should point to /usr/bin/
2.  export PACKAGES_PATH= path pointing to edk2-libc
3.  source edksetup.sh
4.  make -C BaseTools/Source/C
- For ACPI
5.  source ShellPkg/Application/sysarch-acs/tools/scripts/acsbuild.sh bsa
- For DT
5.  source ShellPkg/Application/sysarch-acs/tools/scripts/acsbuild.sh bsa_dt

#### 1.3 Build output

The EFI executable file is generated at <edk2_path>/Build/Shell/DEBUG_GCC49/AARCH64/Bsa.efi

### 2. Test suite execution

#### Prerequisites
#####Firmware dependencies
- If the system supports LPIs (Interrupt ID > 8192) then Firmware should support installation of handler for LPI interrupts.
    - If you are using edk2, change the ArmGic driver in the ArmPkg to support installation of handler for LPIs.
    - Add the following in edk2/ArmPkg/Drivers/ArmGic/GicV3/ArmGicV3Dxe.c
>        - After [#define ARM_GIC_DEFAULT_PRIORITY  0x80]
>          +#define ARM_GIC_MAX_NUM_INTERRUPT 16384
>        - Change this in GicV3DxeInitialize function.
>          -mGicNumInterrupts      = ArmGicGetMaxNumInterrupts (mGicDistributorBase);
>          +mGicNumInterrupts      = ARM_GIC_MAX_NUM_INTERRUPT;

The execution of the compliance suite varies depending on the test environment. The following steps assume that the test suite is invoked through the ACS UEFI shell application.

#### 2.1 Silicon

On a system where a USB port is available and functional, perform the following steps:

#### For IR Systems:
1. Copy 'Bsa.efi' and 'Shell.efi' to a USB Flash drive.
2. Boot the system to U-Boot shell.
3. Plug in the USB flash drive to one of the functional USB ports on the system.
4. To determine the file system number of the plugged-in USB drive, execute command <br />
   `usb start`
5. Copy the 'Shell.efi' to memory location using the command <br />
   `Syntax: fatload usb <dev_num> ${kernel_addr_r} Shell.efi` <br />
   `Eg: fatload usb 0 ${kernel_addr_r} Shell.efi` <br />
   This boots the system to UEFI Shell.
6. To determine the file system number of the plugged-in USB drive, execute 'map -r' command.
7. Type 'fs<x>' where '<x>' is replaced by the number determined in step 5.
8. To start the compliance tests, run the executable Bsa.efi with the appropriate parameters.
9. Copy the UART console output to a log file.
Note: 'Shell.efi' is available in the [pebuilt_images/IR](prebuilt_images)

#### 2.2 Emulation environment with secondary storage
On an emulation environment with secondary storage, perform the following steps:

1. Create an image file which contains the '.efi' file. For example:
  - mkfs.vfat -C -n HD0 hda.img 2097152
  - sudo mount -o rw,loop=/dev/loop0,uid=\`whoami\`,gid=\`whoami\` hda.img /mnt/acs
    In case loop0 is busy, please specify the one that is free.
  - sudo cp  "<path to application>/<efi binary>" /mnt/acs/
  - sudo umount /mnt/acs
2. Load the image file to the secondary storage using a backdoor. The steps to load the image file are emulation environment-specific and beyond the scope of this document.
3. Boot the system to UEFI shell.
4. To determine the file system number of the secondary storage, execute 'map -r' command.
5. Type 'fs<x>' where '<x>' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Bsa.efi with the appropriate parameters.
7. Copy the UART console output to a log file for analysis and certification.

  - For information on the uefi shell application parameters, see the [BSA User Guide](docs/bsa/arm_bsa_architecture_compliance_user_guide.pdf)

#### 2.3 Emulation environment without secondary storage

On an emulation platform where secondary storage is not available, perform the following steps:

1. Add the path to '<acs>.efi' file in the UEFI FD file.
2. Build UEFI image including the UEFI Shell.
3. Boot the system to UEFI shell.
4. Run the executable '<acs>.efi' to start the compliance tests. For details about the parameters,see the [BSA User Guide](docs/arm_bsa_architecture_compliance_user_guide.pdf) and [SBSA USer Guide](docs/sbsa/arm_sbsa_architecture_compliance_user_guide.pdf)
5. Copy the UART console output to a log file for analysis and certification.


#### Linux application

##### Build steps
1. cd <sysarch-acs 1path>/linux_app/bsa-acs-app
2. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
3. make

Successful completion of above steps will generate executable file bsa

##### Steps for Running BSA Tests in Linux

##### Loading the kernel module
Before the BSA ACS Linux application can be run, load the BSA ACS kernel module respectively using the insmod command.

```sh
shell> insmod bsa_acs.ko
```

##### Running BSA ACS

```sh
shell> ./bsa_app or ./bsa
```

  - For information on the BSA Linux application parameters, see the [BSA User Guide](docs/bsa/arm_bsa_architecture_compliance_user_guide.pdf)

##### BSA Linux Test Log View

```sh
shell> sudo dmesg | tail -500 # print last 500 kernel logs
```

After the run is complete, you can remove the BSA module from the system if it is no longer needed.

```sh
shell> sudo rmmod bsa_acs
```

##### Build Script aguments
The following arguments can be used when running the build.sh script:

- __-v or --version__ \- Specifies the Linux kernel version to be used for cross-compilation.
                    If not provided, the default version is 6.8.

- __--GCC_TOOLS__     \-  Specifies the GCC toolchain version for cross-compilation.
                    The default version is 13.2.rel1.

- __--help__          \-  Displays information about the ACS build environment, including default values,
                    usage instructions, and additional notes.

- __--clean__         \-  Removes the output folder build, which contains the resulting modules
                    and applications from the build.

- __--clean_all__     \-  Removes all downloaded repositories and build-related files,
                    including the output directory.

#### Bare-metal abstraction

The Bare-metal build environment is platform specific.

To provide a baseline, the build steps to integrate and run the Bare-metal tests from UEFI shell are provided in [README.md](pal/baremetal/target/RDN2/README.md).

For details on generating the binaries to run on Bare-metal environment, refer [README.md](pal/baremetal/README.md)

#### Running Exerciser tests for complete coverage

Exerciser is a client device wrapped up by PCIe Endpoint. This device is created to meet BSA and SBSA requirements for various PCIe capability validation tests. Running the Exerciser tests provides additional test coverage on the platform.

Note: To run the exerciser tests on a UEFI Based platform with Exerciser, the Exerciser PAL API's need to be implemented. For details on the reference Exerciser implementation and support, see the [Exerciser.md](docs/pcie/Exerciser.md) and [Exerciser_API_porting_guide.md](docs/pcie/Exerciser_API_porting_guide.md)

### Release details
 - Code quality: v1.1.1
 - The tests are written for version 1.1 of the BSA specification.
 - The tests can be run at both the Pre-Silicon and Silicon level.
 - For complete coverage of the BSA rules, availability of an Exerciser is required for Exerciser tests to be run during verficiation at Pre-Silicon level.
 - The compliance suite is not a substitute for design verification.
 - To review the BSA ACS logs, Arm licensees can contact Arm directly through their partner managers.
 - To know about the BSA rules not implemented in this release, see the [Test Scenario Document](docs/bsa/arm_bsa_architecture_compliance_test_scenario.pdf).

### Limitations

 - For systems that present firmware compliant to SBBR, BSA depends on SPCR acpi table to get UART information.
   UEFI console setting must be set to "serial" on these systems.
 - ITS test are available only for systems that present firmware compliant to SBBR.
 - Some PCIe and Exerciser test are dependent on PCIe features supported by the test system.
   Please fill the required API's with test system information.
   - pal_pcie_p2p_support : If the test system PCIe supports peer to peer transaction.
   - pal_pcie_is_cache_present : If the test system supports PCIe address translation cache.
   - pal_pcie_get_legacy_irq_map : Fill system legacy irq map

   Below exerciser capabilities are required by exerciser test.
   - MSI-X interrupt generation.
   - Incoming Transaction Monitoring(order, type).
   - Initiating transacions from and to the exerciser.
   - Ability to check on BDF and register address seen for each configuration address along with access type.

#### Linux Limitations
⚠️ **Note:**: DMA-related tests have not been verified.

For cross-compilation platforms, if you want compatibility with the target system, ensure that the Linux source version matches the version running on the target device.

Example:
 - Linux source version: 5.15
 - Target AArch64 machine kernel version: 5.15.0-139-generic

-  If the versions do not match exactly, the module may fail to load due to an invalid module format.

- ✅ If both versions are identical (e.g., both are 5.15), the build will work correctly — similar to how it works for a SystemReady image.

## SBSA Architecture Compliance Suite

SBSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [SBSA](https://developer.arm.com/documentation/den0029/i/?lang=en) specification, so that implementers can verify if these behaviours have been interpreted correctly.

Most of the tests are executed from UEFI (Unified Extensible Firmware Interface) Shell by executing the ACS UEFI shell application.
A few tests are executed by running the SBSA ACS Linux application which in turn depends on the SBSA ACS Linux kernel module.
The tests can also be executed in a Bare-metal environment. The initialization of the Bare-metal environment is specific to the environment and is out of scope of this document.

### Build steps
#### UEFI Shell application


##### 1. Building from source
    Before you start the ACS build, ensure that the following requirements are met.

- Any mainstream Linux-based OS distribution running on a x86 or AArch64 machine.
- git clone the [EDK2 tree](https://github.com/tianocore/edk2). Recommended edk2 commit is 836942fbadb629050b866a8052e6af755bcdf623
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) to local <edk2_path>.
- Install GCC-ARM 13.2 [toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
- Install the build prerequisite packages to build EDK2.<br />
Note:<br />
- The details of the packages are beyond the scope of this document.

##### 1.1 Target Platform
###### To start the ACS build for platform using ACPI table, perform the following steps:
1.  cd local_edk2_path
2.  git submodule update --init --recursive
3.  git clone https://github.com/ARM-software/sysarch-acs.git ShellPkg/Application/sysarch-acs

#####    1.2 Build environment
###### If the build environment is Linux, perform the following steps:
1.  export GCC49_AARCH64_PREFIX= GCC13.2 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine.<br /> For an AArch64 build it should point to /usr/bin/
2.  export PACKAGES_PATH= path pointing to edk2-libc
3.  source edksetup.sh
4.  make -C BaseTools/Source/C
5.  source ShellPkg/Application/sysarch-acs/tools/scripts/acsbuild.sh sbsa

#### 1.3 Build output

The EFI executable file is generated at <edk2_path>/Build/Shell/DEBUG_GCC49/AARCH64/Sbsa.efi

### 3. Execution Steps

The execution of the compliance suite varies depending on the test environment.
These steps assume that the test suite is invoked through the ACS UEFI shell application.

#### Firmware Dependencies
- If the system supports LPI’s (Interrupt ID > 8192), then Firmware should have support for installing handler for LPI interrupts.
    - If you are using edk2, change the ArmGic driver in the ArmPkg to obtain support for installing handler for LPI’s.
    - Add the following in edk2/ArmPkg/Drivers/ArmGic/GicV3/ArmGicV3Dxe.c
>        - After [#define ARM_GIC_DEFAULT_PRIORITY  0x80]
>          +#define ARM_GIC_MAX_NUM_INTERRUPT 16384
>        - Change this in GicV3DxeInitialize Function.
>          -mGicNumInterrupts      = ArmGicGetMaxNumInterrupts (mGicDistributorBase);
>          +mGicNumInterrupts      = ARM_GIC_MAX_NUM_INTERRUPT;

### 3.1 On Silicon
On a system where a USB port is available and functional, perform the following steps:

1. Copy 'Sbsa.efi' to a USB Flash drive. Path for 'Sbsa.efi' is present in step 2.
2. Plug in the USB Flash drive to one of the functional USB ports on the system.
3. Boot the system to UEFI shell.
4. To determine the file system number of the plugged in USB drive, execute 'map -r' command.
5. Type 'fsx' where 'x' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Sbsa.efi with the appropriate parameters.
   For details on the parameters, refer to [arm SBSA User Guide Document](docs/sbsa/arm_sbsa_architecture_compliance_user_guide.pdf)
> shell> Sbsa.efi
7. Copy the UART console output to a log file for analysis and certification.


### 3.2 Emulation environment with secondary storage
On an emulation environment with secondary storage, perform the following steps:

1. Create an image file which contains the 'Sbsa.efi' file. For Example:
  - mkfs.vfat -C -n HD0 \<name of image\>.img 2097152.
    Here 2097152 is the size of the image.
  - sudo mount -o rw,loop=/dev/loop0,uid=`whoami`,gid=`whoami` \<name of image\>.img /mnt/sbsa.
    If loop0 is busy, specify the loop that is free
  - cp  "\<path to application\>/Sbsa.efi" /mnt/sbsa/
  - sudo umount /mnt/sbsa
2. Load the image file to the secondary storage using a backdoor.
   The steps followed to load the image file are emulation environment-specific and beyond the scope of this document.
3. Boot the system to UEFI shell.
4. To determine the file system number of the secondary storage, execute 'map -r' command.
5. Type 'fsx' where 'x' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Sbsa.efi with the appropriate parameters.
   For details on the parameters, see the [arm SBSA User Guide Document](docs/sbsa/arm_sbsa_architecture_compliance_user_guide.pdf)
> shell> Sbsa.efi
7. Copy the UART console output to a log file for analysis and certification.


### 3.3 Emulation environment without secondary storage
On an emulation platform where secondary storage is not available, perform the following steps:

1. Add the path to 'Sbsa.efi' file in the UEFI FD file.
2. Build UEFI image including the UEFI Shell.
3. Boot the system to UEFI shell.
4. Run the executable 'Sbsa.efi' to start the compliance tests.
   For details about the parameters, see the [arm SBSA User Guide Document](docs/sbsa/arm_sbsa_architecture_compliance_user_guide.pdf).
> shell> Sbsa.efi
5. Copy the UART console output to a log file for analysis and certification.

#### Linux application

##### Build steps
1. cd <sysarch-acs 1path>/linux_app/sbsa-acs-app
2. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
3. make

Successful completion of above steps will generate executable file sbsa

##### Steps for Running SBSA Tests in Linux

##### Loading the kernel module
Before the SBSA ACS Linux application can be run, load the SBSA ACS kernel module respectively using the insmod command.

```sh
shell> insmod sbsa_acs.ko
```

##### Running SBSA ACS

```sh
shell> ./sbsa_app or ./sbsa
```

  - For information on the SBSA Linux application parameters, see the [SBSA User Guide](docs/sbsa/arm_sbsa_architecture_compliance_user_guide.pdf)

##### SBSA Linux Test Log View

```sh
shell> sudo dmesg | tail -500 # print last 500 kernel logs
```

After the run is complete, you can remove the SBSA module from the system if it is no longer needed.

```sh
shell> sudo rmmod sbsa_acs
```

##### Build Script aguments
The following arguments can be used when running the build.sh script:

- __-v or --version__ \- Specifies the Linux kernel version to be used for cross-compilation.
                    If not provided, the default version is 6.8.

- __--GCC_TOOLS__     \-  Specifies the GCC toolchain version for cross-compilation.
                    The default version is 13.2.rel1.

- __--help__          \-  Displays information about the ACS build environment, including default values,
                    usage instructions, and additional notes.

- __--clean__         \-  Removes the output folder build, which contains the resulting modules
                    and applications from the build.

- __--clean_all__     \-  Removes all downloaded repositories and build-related files,
                    including the output directory.

#### Running Exerciser tests for complete coverage

Exerciser is a client device wrapped up by PCIe Endpoint. This device is created to meet BSA and SBSA requirements for various PCIe capability validation tests. Running the Exerciser tests provides additional test coverage on the platform.

Note: To run the exerciser tests on a UEFI Based platform with Exerciser, the Exerciser PAL API's need to be implemented. For details on the reference Exerciser implementation and support, see the [Exerciser.md](docs/pcie/Exerciser.md) and [Exerciser_API_porting_guide.md](docs/pcie/Exerciser_API_porting_guide.md)

#### Bare-metal abstraction

The Bare-metal build environment is platform specific.

To provide a baseline, the build steps to integrate and run the Bare-metal tests from UEFI shell are provided in [README.md](pal/baremetal/target/RDN2/README.md).

For details on generating the binaries to run on Bare-metal environment, refer [README.md](pal/baremetal/README.md)

### Release details
 - Code Quality: REL v7.2.3
 - The tests are written for version 7.2 of the SBSA specification.
 - For complete coverage of the SBSA rules, availability of an Exerciser is required for Exerciser tests to be run during verficiation at Pre-Silicon level.
 - For complete coverage, both SBSA and BSA ACS should be run.
 - The SBSA specification compliments the [BSA specification](https://developer.arm.com/documentation/den0094/d/?lang=en).
 - The tests can be run at both the Pre-Silicon and Silicon level.
 - The compliance suite is not a substitute for design verification.
 - To review the SBSA ACS logs, Arm licensees can contact Arm directly through their partner managers.
 - To know about the SBSA rules not implemented in this release, see [Test Scenario Document](docs/sbsa/arm_sbsa_architecture_compliance_test_scenario.pdf).

### Security implication
SBSA ACS test suite may run at higher privilege level. An attacker may utilize these tests as a means to elevate privilege which can potentially reveal the platform security assets. To prevent the leakage of secure information, it is strongly recommended that the ACS test suite is run only on development platforms. If it is run on production systems, the system should be scrubbed after running the test suite.

### Limitations
- Validating the compliance of certain PCIe rules defined in the SBSA specification requires the PCIe end-point to generate specific stimulus during the runtime of the test. Examples of such stimulus are  P2P, PASID, ATC, etc. The tests that requires these stimuli are grouped together in the exerciser module. The exerciser layer is an abstraction layer that enables the integration of hardware capable of generating such stimuli to the test framework.
The details of the hardware or Verification IP which enable these exerciser tests are platform specific and are beyond the scope of this document.

- The MPAM MSC PCC (ACPI Platform Communication Channel) support has been implemented but not yet verified on any platform. Please raise an issue if any failures or errors are encountered during the ACS run.

#### Linux Limitations
⚠️ **Note:**: DMA-related tests have not been verified.

For cross-compilation platforms, if you want compatibility with the target system, ensure that the Linux source version matches the version running on the target device.

Example:
 - Linux source version: 5.15
 - Target AArch64 machine kernel version: 5.15.0-139-generic

-  If the versions do not match exactly, the module may fail to load due to an invalid module format.

- ✅ If both versions are identical (e.g., both are 5.15), the build will work correctly — similar to how it works for a SystemReady image.

#### SBSA Tests dependencies
 - MPAM tests will require EL3 firmware to enable access to MPAM registers from lower EL's.
   If arm trusted firmware is used as EL3 fimrware, enable ENABLE_MPAM_FOR_LOWER_ELS=1 during arm TF build.
   If the above flags are not enabled, MPAM tests can lead to exception at EL3.
 - RAS test will require EL3 firmware to enable access to RAS registers from lower EL's and forward RAS related exceptions to lower EL's.
   If arm trusted firmware is used as EL3 fimrware, enable EL3_EXCEPTION_HANDLING=1 RAS_EXTENSION=1 HANDLE_EA_EL3_FIRST=1 RAS_TRAP_LOWER_EL_ERR_ACCESS=0 during arm TF build
   If the above flags are not enabled, RAS tests can lead to exception at EL3.
 - SBSA Future Requirements ETE test will require EL3 firmware to enable access to Trace registers from lower EL's.
   If arm trusted firmware is used as EL3 fimrware, ENABLE_TRF_FOR_NS=1 ENABLE_TRBE_FOR_NS=1 ENABLE_SYS_REG_TRACE_FOR_NS=1 during arm TF build
   If the above flags are not enabled, ETE tests can lead to exception at EL3.
 - MPAM test have dependency on MPAM, SRAT, HMAT, PPTT tables.
 - RAS test have dependency on AEST, RAS2, SRAT, HMAT, PPTT tables.
 - PMU test have dependency on APMT table.
 - Entrophy rule will require ACS to build with NIST-STS package

**Note:** To build the ACS with NIST Statistical Test Suite, see the [arm SBSA_NIST_User_Guide Document](docs/sbsa/arm_sbsa_nist_user_guide.md)

|APIs                         |Description                                                                   |Affected tests          |
|-----------------------------|------------------------------------------------------------------------------|------------------------|
|pal_pcie_dev_p2p_support     |Return 0 if the test system PCIe supports peer to peer transaction, else 1    |881, 882                |
|pal_pcie_is_cache_present    |Return 1 if the test system supports PCIe address translation cache, else 0   |880                     |
|pal_pcie_get_legacy_irq_map  |Return 0 if system legacy irq map is filled, else 1                           |878                     |

   Below exerciser capabilities are required by exerciser test.
   - MSI-X interrupt generation.
   - Incoming Transaction Monitoring (order, type).
   - Initiating transactions from and to the exerciser.
   - Ability to check on BDF and register address seen for each configuration address along with access type.

 - SBSA Test 803 (Check ECAM Memory accessibility) execution time depends on the system PCIe hierarchy. For systems with multiple ECAMs the time taken to complete can be long which is normal. Please wait until the test completes.

## GitHub branch
  - To pick up the release version of the code, checkout the corresponding tag from the main branch.
  - To get the latest version of the code with bug fixes and new features, use the main branch.

## Additional reading
  - For information about the implementable SBSA rules test algorithm and for unimplemented SBSA rules, see the [arm SBSA Test Scenario Document](docs/sbsa/arm_sbsa_architecture_compliance_test_scenario.pdf).
  - For information on test category(UEFI, Linux, Bare-metal) and applicable systems(IR,ES,SR,Pre-Silicon), see the [arm SBSA Test Checklist Document](docs/sbsa/arm_sbsa_testcase_checklist.rst)
  - For details on the design of the ACS, see the [arm SBSA Validation Methodology Document](docs/sbsa/arm_sbsa_architecture_compliance_validation_methodology.pdf).
  - For details on the SBSA ACS UEFI Shell Application and Linux Application see the [arm SBSA User Guide Document](docs/sbsa/arm_sbsa_architecture_compliance_user_guide.pdf).
  - For details on the ACS Bare-metal support, see the
    - [arm SBSA Bare-metal User Guide Document](docs/sbsa/arm_sbsa_architecture_compliance_bare-metal_user_guide.pdf).
    - [Bare-metal Code](pal/baremetal/). <br />
Note: The Bare-metal PCIe enumeration code provided as part of the SYSARCH ACS should be used and should not be replaced. This code is vital in analysis of the test result.


## DRTM, System MPAM and mem_test
**Note:** 
- To run DRTM ACS, please see the [DRTM README](docs/drtm/README.md)
- To run System MPAM tests, see the [MPAM README](docs/mpam/README.md)
- To evaluate the correctness and consistency of a system's memory model, memory model consistency tests (litmus tests) can be optionally built into UEFI application. To run mem_test, see the [MEMTEST_README](mem_test/README.md)

## Security implication
The Arm SystemReady ACS test suite may run at a higher privilege level. An attacker may utilize these tests to elevate the privilege which can potentially reveal the platform security assets. To prevent the leakage of Secure information, Arm strongly recommends that you run the ACS test suite only on development platforms. If it is run on production systems, the system should be scrubbed after running the test suite.

SYSARCH ACS is distributed under Apache v2.0 License.


## Feedback, contributions, and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to "support-systemready-acs@arm.com" with details.
 - Arm licensees may contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests. See the GitHub documentation on how to raise pull requests.

--------------
---

## 📄 License

Arm Sysarch ACS is distributed under the [Apache v2.0 License](https://www.apache.org/licenses/LICENSE-2.0).

---

## 💬 Feedback, Contributions, and Support

* For feedback, please use the [GitHub Issue Tracker](../../issues) associated with this repository.
* For support, contact **[support-systemready-acs@arm.com](mailto:support-systemready-acs@arm.com)** with relevant details.
* Arm licensees can reach out via their Arm partner managers.
* Contributions are welcome via [GitHub pull requests](../../pulls).

---

Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
