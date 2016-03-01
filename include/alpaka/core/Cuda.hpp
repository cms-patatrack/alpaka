/**
 * \file
 * Copyright 2014-2015 Benjamin Worpitz
 *
 * This file is part of alpaka.
 *
 * alpaka is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alpaka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with alpaka.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED

#ifndef __CUDACC__
    #error If ALPAKA_ACC_GPU_CUDA_ENABLED is set, the compiler has to support CUDA!
#endif

#include <alpaka/elem/Traits.hpp>           // ElemType
#include <alpaka/offset/Traits.hpp>         // GetOffset/SetOffset
#include <alpaka/extent/Traits.hpp>         // GetExtent/SetExtent
#include <alpaka/size/Traits.hpp>           // SizeType
#include <alpaka/vec/Vec.hpp>               // Vec
#include <alpaka/meta/IntegerSequence.hpp>  // IntegerSequence
#include <alpaka/meta/Metafunctions.hpp>    // meta::Conjunction
#include <alpaka/core/Common.hpp>           // ALPAKA_FN_HOST

// cuda_runtime_api.h: CUDA Runtime API C-style interface that does not require compiling with nvcc.
// cuda_runtime.h: CUDA Runtime API  C++-style interface built on top of the C API.
//  It wraps some of the C API routines, using overloading, references and default arguments.
//  These wrappers can be used from C++ code and can be compiled with any C++ compiler.
//  The C++ API also has some CUDA-specific wrappers that wrap C API routines that deal with symbols, textures, and device functions.
//  These wrappers require the use of \p nvcc because they depend on code being generated by the compiler.
//  For example, the execution configuration syntax to invoke kernels is only available in source code compiled with nvcc.
#include <cuda_runtime.h>
// cuda.h: CUDA Driver API
//#include <cuda.h>

#include <array>                            // std::array
#include <type_traits>                      // std::enable_if
#include <utility>                          // std::forward
#include <iostream>                         // std::cerr
#include <string>                           // std::string, std::to_string
#include <stdexcept>                        // std::runtime_error
#include <cstddef>                          // std::size_t

#if (!defined(CUDART_VERSION) || (CUDART_VERSION < 7000))
    #error "CUDA version 7.0 or greater required!"
#endif

/*#if (!defined(CUDA_VERSION) || (CUDA_VERSION < 7000))
    #error "CUDA version 7.0 or greater required!"
#endif*/

namespace alpaka
{
    namespace cuda
    {
        namespace detail
        {
            //-----------------------------------------------------------------------------
            //! CUDA runtime API error checking with log and exception, ignoring specific error values
            // NOTE: All ignored errors have to be convertible to cudaError_t.
            //-----------------------------------------------------------------------------
            template<
                typename... TErrors,
                typename = typename std::enable_if<
                    meta::Conjunction<
                        std::true_type,
                        std::is_convertible<
                            TErrors,
                            cudaError_t
                        >...
                    >::value>::type>
            ALPAKA_FN_HOST auto cudaRtCheckIgnore(
                cudaError_t const & error,
                char const * cmd,
                char const * file,
                int const & line,
                TErrors && ... ignoredErrorCodes)
            -> void
            {
                if(error != cudaSuccess)
                {
                    // If the error code is not one of the ignored ones.
                    std::array<cudaError_t, sizeof...(ignoredErrorCodes)> const aIgnoredErrorCodes{std::forward<TErrors>(ignoredErrorCodes)...};
                    if(std::find(aIgnoredErrorCodes.cbegin(), aIgnoredErrorCodes.cend(), error) == aIgnoredErrorCodes.cend())
                    {
                        std::string const sError(std::string(file) + "(" + std::to_string(line) + ") '" + std::string(cmd) + "' returned error: '" + std::string(cudaGetErrorString(error)) + "'!");
                        std::cerr << sError << std::endl;
                        ALPAKA_DEBUG_BREAK;
                        throw std::runtime_error(sError);
                    }
                }
            }
            //-----------------------------------------------------------------------------
            //! CUDA runtime API last error checking with log and exception.
            //-----------------------------------------------------------------------------
            ALPAKA_FN_HOST auto cudaRtCheckLastError(
                char const * cmd,
                char const * file,
                int const & line)
            -> void
            {
                cudaError_t const lastError(cudaGetLastError());
                if(lastError != cudaSuccess)
                {
                    std::string const sError(std::string(file) + "(" + std::to_string(line) + ") '" + std::string(cmd) + "' A previous CUDA call (not this one) set the error: '" + std::string(cudaGetErrorString(lastError)) + "'!");
                    std::cerr << sError << std::endl;
                    ALPAKA_DEBUG_BREAK;
                    throw std::runtime_error(sError);
                }
            }
        }
    }
}

#if BOOST_COMP_MSVC
    //-----------------------------------------------------------------------------
    //! CUDA runtime error checking with log and exception, ignoring specific error values
    //-----------------------------------------------------------------------------
    #define ALPAKA_CUDA_RT_CHECK_IGNORE(cmd, ...)\
        ::alpaka::cuda::detail::cudaRtCheckLastError(#cmd, __FILE__, __LINE__);\
        ::alpaka::cuda::detail::cudaRtCheckIgnore(cmd, #cmd, __FILE__, __LINE__, __VA_ARGS__)
#else
    //-----------------------------------------------------------------------------
    //! CUDA runtime error checking with log and exception, ignoring specific error values
    //-----------------------------------------------------------------------------
    #define ALPAKA_CUDA_RT_CHECK_IGNORE(cmd, ...)\
        ::alpaka::cuda::detail::cudaRtCheckLastError(#cmd, __FILE__, __LINE__);\
        ::alpaka::cuda::detail::cudaRtCheckIgnore(cmd, #cmd, __FILE__, __LINE__, ##__VA_ARGS__)
#endif

//-----------------------------------------------------------------------------
//! CUDA runtime error checking with log and exception.
//-----------------------------------------------------------------------------
#define ALPAKA_CUDA_RT_CHECK(cmd)\
    ALPAKA_CUDA_RT_CHECK_IGNORE(cmd)

/*namespace alpaka
{
    namespace cuda
    {
        namespace detail
        {
            //-----------------------------------------------------------------------------
            //! CUDA driver API error checking with log and exception, ignoring specific error values
            //-----------------------------------------------------------------------------
            ALPAKA_FN_HOST auto cudaDrvCheck(
                cudaError_t const & error,
                char const * cmd,
                char const * file,
                int const & line)
            -> void
            {
                // Even if we get the error directly from the command, we have to reset the global error state by getting it.
                if(error != CUDA_SUCCESS)
                {
                    std::string const sError(std::to_string(file) + "(" + std::to_string(line) + ") '" + std::to_string(cmd) + "' returned error: '" + std::to_string(error) + "' (possibly from a previous CUDA call)!");
                    std::cerr << sError << std::endl;
                    ALPAKA_DEBUG_BREAK;
                    throw std::runtime_error(sError);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
//! CUDA driver error checking with log and exception.
//-----------------------------------------------------------------------------
#define ALPAKA_CUDA_DRV_CHECK(cmd)\
    ::cuda::detail::cudaDrvCheck(cmd, #cmd, __FILE__, __LINE__)*/


//-----------------------------------------------------------------------------
// CUDA vector_types.h trait specializations.
//-----------------------------------------------------------------------------
namespace alpaka
{
    //-----------------------------------------------------------------------------
    //! The CUDA specifics.
    //-----------------------------------------------------------------------------
    namespace cuda
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors 1D dimension get trait specialization.
            //#############################################################################
            template<
                typename T>
            struct IsCudaBuiltInType :
                std::integral_constant<
                    bool,
                    std::is_same<T, char1>::value
                    || std::is_same<T, double1>::value
                    || std::is_same<T, float1>::value
                    || std::is_same<T, int1>::value
                    || std::is_same<T, long1>::value
                    || std::is_same<T, longlong1>::value
                    || std::is_same<T, short1>::value
                    || std::is_same<T, uchar1>::value
                    || std::is_same<T, uint1>::value
                    || std::is_same<T, ulong1>::value
                    || std::is_same<T, ulonglong1>::value
                    || std::is_same<T, ushort1>::value
                    || std::is_same<T, char2>::value
                    || std::is_same<T, double2>::value
                    || std::is_same<T, float2>::value
                    || std::is_same<T, int2>::value
                    || std::is_same<T, long2>::value
                    || std::is_same<T, longlong2>::value
                    || std::is_same<T, short2>::value
                    || std::is_same<T, uchar2>::value
                    || std::is_same<T, uint2>::value
                    || std::is_same<T, ulong2>::value
                    || std::is_same<T, ulonglong2>::value
                    || std::is_same<T, ushort2>::value
                    || std::is_same<T, char3>::value
                    || std::is_same<T, dim3>::value
                    || std::is_same<T, double3>::value
                    || std::is_same<T, float3>::value
                    || std::is_same<T, int3>::value
                    || std::is_same<T, long3>::value
                    || std::is_same<T, longlong3>::value
                    || std::is_same<T, short3>::value
                    || std::is_same<T, uchar3>::value
                    || std::is_same<T, uint3>::value
                    || std::is_same<T, ulong3>::value
                    || std::is_same<T, ulonglong3>::value
                    || std::is_same<T, ushort3>::value
                    || std::is_same<T, char4>::value
                    || std::is_same<T, double4>::value
                    || std::is_same<T, float4>::value
                    || std::is_same<T, int4>::value
                    || std::is_same<T, long4>::value
                    || std::is_same<T, longlong4>::value
                    || std::is_same<T, short4>::value
                    || std::is_same<T, uchar4>::value
                    || std::is_same<T, uint4>::value
                    || std::is_same<T, ulong4>::value
                    || std::is_same<T, ulonglong4>::value
                    || std::is_same<T, ushort4>::value>
            {};
        }
    }
    namespace dim
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors 1D dimension get trait specialization.
            //#############################################################################
            template<
                typename T>
            struct DimType<
                T,
                typename std::enable_if<
                    std::is_same<T, char1>::value
                    || std::is_same<T, double1>::value
                    || std::is_same<T, float1>::value
                    || std::is_same<T, int1>::value
                    || std::is_same<T, long1>::value
                    || std::is_same<T, longlong1>::value
                    || std::is_same<T, short1>::value
                    || std::is_same<T, uchar1>::value
                    || std::is_same<T, uint1>::value
                    || std::is_same<T, ulong1>::value
                    || std::is_same<T, ulonglong1>::value
                    || std::is_same<T, ushort1>::value>::type>
            {
                using type = dim::DimInt<1u>;
            };
            //#############################################################################
            //! The CUDA vectors 2D dimension get trait specialization.
            //#############################################################################
            template<
                typename T>
            struct DimType<
                T,
                typename std::enable_if<
                    std::is_same<T, char2>::value
                    || std::is_same<T, double2>::value
                    || std::is_same<T, float2>::value
                    || std::is_same<T, int2>::value
                    || std::is_same<T, long2>::value
                    || std::is_same<T, longlong2>::value
                    || std::is_same<T, short2>::value
                    || std::is_same<T, uchar2>::value
                    || std::is_same<T, uint2>::value
                    || std::is_same<T, ulong2>::value
                    || std::is_same<T, ulonglong2>::value
                    || std::is_same<T, ushort2>::value>::type>
            {
                using type = dim::DimInt<2u>;
            };
            //#############################################################################
            //! The CUDA vectors 3D dimension get trait specialization.
            //#############################################################################
            template<
                typename T>
            struct DimType<
                T,
                typename std::enable_if<
                    std::is_same<T, char3>::value
                    || std::is_same<T, dim3>::value
                    || std::is_same<T, double3>::value
                    || std::is_same<T, float3>::value
                    || std::is_same<T, int3>::value
                    || std::is_same<T, long3>::value
                    || std::is_same<T, longlong3>::value
                    || std::is_same<T, short3>::value
                    || std::is_same<T, uchar3>::value
                    || std::is_same<T, uint3>::value
                    || std::is_same<T, ulong3>::value
                    || std::is_same<T, ulonglong3>::value
                    || std::is_same<T, ushort3>::value>::type>
            {
                using type = dim::DimInt<3u>;
            };
            //#############################################################################
            //! The CUDA vectors 4D dimension get trait specialization.
            //#############################################################################
            template<
                typename T>
            struct DimType<
                T,
                typename std::enable_if<
                    std::is_same<T, char4>::value
                    || std::is_same<T, double4>::value
                    || std::is_same<T, float4>::value
                    || std::is_same<T, int4>::value
                    || std::is_same<T, long4>::value
                    || std::is_same<T, longlong4>::value
                    || std::is_same<T, short4>::value
                    || std::is_same<T, uchar4>::value
                    || std::is_same<T, uint4>::value
                    || std::is_same<T, ulong4>::value
                    || std::is_same<T, ulonglong4>::value
                    || std::is_same<T, ushort4>::value>::type>
            {
                using type = dim::DimInt<4u>;
            };
        }
    }
    namespace elem
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors elem type trait specialization.
            //#############################################################################
            template<
                typename TSize>
            struct ElemType<
                TSize,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TSize>::value>::type>
            {
                using type = decltype(TSize().x);
            };
        }
    }
    namespace extent
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            //#############################################################################
            template<
                typename TExtent>
            struct GetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 1u>,
                TExtent,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 1)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                -> decltype(extent.x)
                {
                    return extent.x;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            //#############################################################################
            template<
                typename TExtent>
            struct GetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 2u>,
                TExtent,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 2)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                -> decltype(extent.y)
                {
                    return extent.y;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            //#############################################################################
            template<
                typename TExtent>
            struct GetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 3u>,
                TExtent,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 3)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                -> decltype(extent.z)
                {
                    return extent.z;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            //#############################################################################
            template<
                typename TExtent>
            struct GetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 4u>,
                TExtent,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 4)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                -> decltype(extent.w)
                {
                    return extent.w;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            //#############################################################################
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 1u>,
                TExtent,
                TExtentVal,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 1)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.x = extentVal;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            //#############################################################################
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 2u>,
                TExtent,
                TExtentVal,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 2)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.y = extentVal;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            //#############################################################################
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 3u>,
                TExtent,
                TExtentVal,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 3)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.z = extentVal;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            //#############################################################################
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                dim::DimInt<dim::Dim<TExtent>::value - 4u>,
                TExtent,
                TExtentVal,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (dim::Dim<TExtent>::value >= 4)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.w = extentVal;
                }
            };
        }
    }
    namespace offset
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors offset get trait specialization.
            //#############################################################################
            template<
                typename TOffsets>
            struct GetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 1u>,
                TOffsets,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 1)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getOffset(
                    TOffsets const & offsets)
                -> decltype(offsets.x)
                {
                    return offsets.x;
                }
            };
            //#############################################################################
            //! The CUDA vectors offset get trait specialization.
            //#############################################################################
            template<
                typename TOffsets>
            struct GetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 2u>,
                TOffsets,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 2)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getOffset(
                    TOffsets const & offsets)
                -> decltype(offsets.y)
                {
                    return offsets.y;
                }
            };
            //#############################################################################
            //! The CUDA vectors offset get trait specialization.
            //#############################################################################
            template<
                typename TOffsets>
            struct GetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 3u>,
                TOffsets,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 3)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getOffset(
                    TOffsets const & offsets)
                -> decltype(offsets.z)
                {
                    return offsets.z;
                }
            };
            //#############################################################################
            //! The CUDA vectors offset get trait specialization.
            //#############################################################################
            template<
                typename TOffsets>
            struct GetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 4u>,
                TOffsets,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 4)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getOffset(
                    TOffsets const & offsets)
                -> decltype(offsets.w)
                {
                    return offsets.w;
                }
            };
            //#############################################################################
            //! The CUDA vectors offset set trait specialization.
            //#############################################################################
            template<
                typename TOffsets,
                typename TOffset>
            struct SetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 1u>,
                TOffsets,
                TOffset,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 1)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setOffset(
                    TOffsets const & offsets,
                    TOffset const & offset)
                -> void
                {
                    offsets.x = offset;
                }
            };
            //#############################################################################
            //! The CUDA vectors offset set trait specialization.
            //#############################################################################
            template<
                typename TOffsets,
                typename TOffset>
            struct SetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 2u>,
                TOffsets,
                TOffset,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 2)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setOffset(
                    TOffsets const & offsets,
                    TOffset const & offset)
                -> void
                {
                    offsets.y = offset;
                }
            };
            //#############################################################################
            //! The CUDA vectors offset set trait specialization.
            //#############################################################################
            template<
                typename TOffsets,
                typename TOffset>
            struct SetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 3u>,
                TOffsets,
                TOffset,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 3)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setOffset(
                    TOffsets const & offsets,
                    TOffset const & offset)
                -> void
                {
                    offsets.z = offset;
                }
            };
            //#############################################################################
            //! The CUDA vectors offset set trait specialization.
            //#############################################################################
            template<
                typename TOffsets,
                typename TOffset>
            struct SetOffset<
                dim::DimInt<dim::Dim<TOffsets>::value - 4u>,
                TOffsets,
                TOffset,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TOffsets>::value
                    && (dim::Dim<TOffsets>::value >= 4)>::type>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setOffset(
                    TOffsets const & offsets,
                    TOffset const & offset)
                -> void
                {
                    offsets.w = offset;
                }
            };
        }
    }
    namespace size
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors size type trait specialization.
            //#############################################################################
            template<
                typename TSize>
            struct SizeType<
                TSize,
                typename std::enable_if<
                    cuda::traits::IsCudaBuiltInType<TSize>::value>::type>
            {
                using type = std::size_t;
            };
        }
    }
}

#endif
