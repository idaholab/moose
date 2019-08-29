//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "metaphysicl/dualsemidynamicsparsenumberarray.h"

namespace Moose
{
template <std::size_t N>
inline void
derivInsert(SemiDynamicSparseNumberArray<Real, unsigned int, NWrapper<N>> & derivs,
            unsigned int index,
            Real value)
{
  derivs.insert(index) = value;
}
}
