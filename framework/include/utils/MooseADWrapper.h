//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include <vector>

template <typename T, bool is_ad>
struct MooseADWrapperStruct
{
  typedef T type;
};

template <>
struct MooseADWrapperStruct<Real, true>
{
  typedef ADReal type;
};

template <>
struct MooseADWrapperStruct<Point, true>
{
  typedef ADPoint type;
};

template <>
struct MooseADWrapperStruct<ChainedReal, true>
{
  typedef ChainedADReal type;
};

// W<T> -> W<ADT>, e.g. RankTwoTensorTempl<Real> -> RankTwoTensorTempl<ADReal>
template <template <typename> class W, typename T, bool is_ad>
struct MooseADWrapperStruct<W<T>, is_ad>
{
  typedef W<typename MooseADWrapperStruct<T, is_ad>::type> type;
};

// e.g. std::vector<Real> -> std::vector<ADReal>
template <typename T, bool is_ad>
struct MooseADWrapperStruct<std::vector<T>, is_ad>
{
  typedef std::vector<typename MooseADWrapperStruct<T, is_ad>::type> type;
};

// std::array and MooseUtils::SemidynamicVector support
template <typename T, std::size_t N, bool is_ad>
struct MooseADWrapperStruct<std::array<T, N>, is_ad>
{
  typedef std::array<typename MooseADWrapperStruct<T, is_ad>::type, N> type;
};

namespace MooseUtils
{
template <typename, std::size_t, bool>
class SemidynamicVector;
}

template <typename T, std::size_t N, bool is_ad, bool zero_initialize>
struct MooseADWrapperStruct<MooseUtils::SemidynamicVector<T, N, zero_initialize>, is_ad>
{
  typedef MooseUtils::
      SemidynamicVector<typename MooseADWrapperStruct<T, is_ad>::type, N, zero_initialize>
          type;
};

template <typename T, bool is_ad>
using MooseADWrapper = typename MooseADWrapperStruct<T, is_ad>::type;

template <typename T>
struct MooseIsADType
{
  static constexpr bool value = false;
};

template <>
struct MooseIsADType<ADReal>
{
  static constexpr bool value = true;
};

template <>
struct MooseIsADType<ADPoint>
{
  static constexpr bool value = true;
};

template <template <typename> class W, typename T>
struct MooseIsADType<W<T>>
{
  static constexpr bool value = MooseIsADType<T>::value;
};

template <typename T>
struct MooseIsADType<std::vector<T>>
{
  static constexpr bool value = MooseIsADType<T>::value;
};

template <template <typename, std::size_t> class W, typename T, std::size_t N>
struct MooseIsADType<W<T, N>>
{
  static constexpr bool value = MooseIsADType<T>::value;
};
