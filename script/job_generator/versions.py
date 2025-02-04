"""Copyright 2023 Simeon Ehrig

This file is part of alpaka.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.


Used software in the CI tests."""

from typing import Dict, List, Tuple
from typeguard import typechecked

from alpaka_job_coverage.globals import *  # pylint: disable=wildcard-import,unused-wildcard-import
from alpaka_globals import *  # pylint: disable=wildcard-import,unused-wildcard-import


# TODO: only an example
sw_versions: Dict[str, List[str]] = {
    GCC: ["9", "10", "11", "12"],
    CLANG: ["9", "10", "11", "12", "13", "14", "15"],
    NVCC: [
        "11.0",
        "11.1",
        "11.2",
        "11.3",
        "11.4",
        "11.5",
        "11.6",
        "11.7",
        "11.8",
        "12.0",
    ],
    HIPCC: ["5.0", "5.1", "5.2", "5.3"],
    BACKENDS: [
        #         ALPAKA_ACC_CPU_B_OMP2_T_SEQ_ENABLE,
        ALPAKA_ACC_GPU_CUDA_ENABLE,
        ALPAKA_ACC_GPU_HIP_ENABLE,
    ],
    UBUNTU: ["20.04"],
    CMAKE: ["3.18.6", "3.19.8", "3.20.6", "3.21.6", "3.22.3", "3.23.2"],
    BOOST: ["1.74.0", "1.75.0", "1.76.0", "1.77.0", "1.78.0", "1.79.0", "1.80.0"],
    CXX_STANDARD: ["17", "20"],
    BUILD_TYPE: BUILD_TYPES,
    # use only TEST_COMPILE_ONLY, because TEST_RUNTIME will be set manually depend on some
    # conditions later
    JOB_EXECUTION_TYPE: [JOB_EXECUTION_COMPILE_ONLY],
}


@typechecked
def get_compiler_versions(clang_cuda: bool = True) -> List[Tuple[str, str]]:
    """Generate a list of compiler name version tuple.

    Args:
        clang_cuda (bool, optional): If true, create entries for cling-cuda basing on the clang
        version. Defaults to True.

    Returns:
        List[Tuple[str, str]]: The compiler name version tuple list.
    """
    compilers: List[Tuple[str, str]] = []

    # only use keys defined in sw_versions
    for compiler_name in set(sw_versions.keys()).intersection(
        [GCC, CLANG, NVCC, HIPCC]
    ):
        for version in sw_versions[compiler_name]:
            compilers.append((compiler_name, version))
            if clang_cuda and compiler_name == CLANG:
                compilers.append((CLANG_CUDA, version))

    return compilers


@typechecked
def get_backend_matrix() -> List[List[Tuple[str, str]]]:
    """Generate backend list, where only backend is active on the same time.

    Returns:
        List[List[Tuple[str, str]]]: The backend list.
    """
    combination_matrix: List[List[Tuple[str, str]]] = []

    # TODO(SimeonEhrig) only working for HIP in the moment
    if HIPCC in sw_versions:
        for rocm_version in sw_versions[HIPCC]:
            combination_matrix.append([(ALPAKA_ACC_GPU_HIP_ENABLE, rocm_version)])

    if NVCC in sw_versions:
        for cuda_version in sw_versions[NVCC]:
            combination_matrix.append([(ALPAKA_ACC_GPU_CUDA_ENABLE, cuda_version)])

    return combination_matrix


@typechecked
def get_sw_tuple_list(name: str) -> List[Tuple[str, str]]:
    """Creates a list of software name version tuples for a software name.

    Args:
        name (str): Name of the software

    Returns:
        List[Tuple[str, str]]: List of software name versions tuples.
    """
    tuple_list: List[Tuple[str, str]] = []
    for version in sw_versions[name]:
        tuple_list.append((name, version))

    return tuple_list
