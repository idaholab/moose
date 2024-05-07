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

template <template <typename T, typename... Args> class W, typename T, typename... Args>
struct MooseIsADType<W<T, Args...>>
{
  static constexpr bool value = MooseIsADType<T>::value;
};

template <typename T, typename... Args>
struct MooseIsADType<MetaPhysicL::DualNumber<T, Args...>>
{
  static constexpr bool value = true;
};
