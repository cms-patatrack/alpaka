"""Copyright 2023 Simeon Ehrig

This file is part of alpaka.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.


This module contains constants used for the alpaka job generation.
"""

from typing import List

# additional alpaka specific parameters
BUILD_TYPE: str = "build_type"
JOB_EXECUTION_TYPE: str = "job_execution_type"

# possible values of BUILD_TYPE
CMAKE_RELEASE: str = "Release"
CMAKE_DEBUG: str = "Debug"
BUILD_TYPES: List[str] = [CMAKE_RELEASE, CMAKE_DEBUG]

# possible values of TEST_TYPE
JOB_EXECUTION_RUNTIME: str = "runtime"
JOB_EXECUTION_COMPILE_ONLY: str = "compile_only"
JOB_EXECUTION_TYPES: List[str] = [JOB_EXECUTION_COMPILE_ONLY, JOB_EXECUTION_RUNTIME]

# CUDA SM level of the job
# is empty, if there is no CUDA backend enabled
SM_LEVEL: str = "sm_level"
