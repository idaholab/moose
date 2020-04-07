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

template <template <typename> class W>
struct MooseADWrapperStruct<W<Real>, true>
{
  typedef W<ADReal> type;
};

template <template <typename> class W>
struct MooseADWrapperStruct<std::vector<W<Real>>, true>
{
  typedef std::vector<W<ADReal>> type;
};

template <typename T, bool is_ad>
using MooseADWrapper = typename MooseADWrapperStruct<T, is_ad>::type;
