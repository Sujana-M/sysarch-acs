## @file
#  Copyright (c) 2023-2025, Arm Limited or its affiliates. All rights reserved.
#  SPDX-License-Identifier : Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
##

if [ $(uname -m) != "aarch64" ] && [ -v $GCC49_AARCH64_PREFIX ]
then
    echo "GCC49_AARCH64_PREFIX is not set"
    echo "set using export GCC49_AARCH64_PREFIX=<lib_path>/bin/aarch64-linux-gnu-"
    return 0
fi

# Get the path of the current shell script. Based on the script path navigate to sysarch-acs path
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

bsa_path="$(dirname "$(dirname "$script_dir")")"
echo "sysarch-acs path is: $(realpath "$bsa_path")"

# Use the default sysarch-acs directory
sbsa_path="$bsa_path/../ssysarch-acs"
echo "ssysarch-acs path is: $(realpath "$sbsa_path")"

export BSA_PATH=$(realpath "$bsa_path")

NISTStatus=1;

function build_with_NIST()
{
    if [ ! -f "sts-2_1_2.zip" ]; then
        wget https://csrc.nist.gov/CSRC/media/Projects/Random-Bit-Generation/documents/sts-2_1_2.zip
        status=$?
        if [ $status -ne 0 ]; then
            echo "wget failed for NIST. Building sbsa without NIST"
            return $status
        fi
    fi

    if [ ! -d "ShellPkg/Application/sysarch-acs/test_pool/nist_sts/sts-2.1.2/" ]; then
        /usr/bin/unzip sts-2_1_2.zip -d ShellPkg/Application/sysarch-acs/test_pool/nist_sts/.
	status=$?
        if [ $status -ne 0 ]; then
            echo "unzip failed for NIST. Building sbsa without NIST"
            return $status
        fi
    fi

    cd ShellPkg/Application/sysarch-acs/test_pool/nist_sts/sts-2.1.2/
    if ! patch -R -p1 -s -f --dry-run < ../../../patches/nist_sbsa_sts.diff; then
        patch -p1 < ../../../patches/nist_sbsa_sts.diff
        status=$?
	if [ $status -ne 0 ]; then
            echo "patch failed for NIST. Building sbsa without NIST"
            return $status
        fi
    fi
    cd -

    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/SbsaNist.inf -D ENABLE_NIST -D SBSA
    status=$?
    if [ $status -ne 0 ]; then
        echo "Build failed for NIST. Building sbsa without NIST"
        return $status
    fi

    return $status
}


if [ "$1" == "nist" ]; then
    build_with_NIST
    NISTStatus=$?
    return 0;
fi

if [ "$1" == "bsa" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/Bsa.inf -D BSA
    return 0;
fi

if [ "$1" == "bsa_dt" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/Bsa.inf -D BSA
    return 0;
fi

if [ "$1" == "sbsa" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/Sbsa.inf -D SBSA
    return 0;
fi

if [ "$1" == "drtm" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/Drtm.inf
    return 0;
fi

if [ "$1" == "mem_test" ]; then
    build -a AARCH64 -t GCC49 -n 1 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/Mem.inf
    return 0;
fi

if [ "$1" == "mpam" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/Mpam.inf
    return 0;
fi

    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/sysarch-acs/apps/uefi/Unified.inf
