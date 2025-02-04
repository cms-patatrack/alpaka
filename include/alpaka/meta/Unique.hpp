/* Copyright 2022 Benjamin Worpitz, Bernhard Manfred Gruber
 *
 * This file is part of alpaka.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <type_traits>

namespace alpaka::meta
{
    namespace detail
    {
        template<typename T, typename... Ts>
        struct UniqueHelper
        {
            using type = T;
        };

        template<template<typename...> class TList, typename... Ts, typename U, typename... Us>
        struct UniqueHelper<TList<Ts...>, U, Us...>
            : std::conditional_t<
                  (std::is_same_v<U, Ts> || ...),
                  UniqueHelper<TList<Ts...>, Us...>,
                  UniqueHelper<TList<Ts..., U>, Us...>>
        {
        };

        template<typename T>
        struct UniqueImpl;

        template<template<typename...> class TList, typename... Ts>
        struct UniqueImpl<TList<Ts...>>
        {
            using type = typename UniqueHelper<TList<>, Ts...>::type;
        };
    } // namespace detail

    //! Trait that returns a list with only unique (no equal) types (a set). Duplicates will be filtered out.
    template<typename TList>
    using Unique = typename detail::UniqueImpl<TList>::type;
} // namespace alpaka::meta
