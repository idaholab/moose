//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseConfig.h"
#include "MooseError.h"
#include "metaphysicl/dualnumberarray.h"

namespace Moose
{
template <std::size_t N>
inline void
derivInsert(NumberArray<N, Real> & derivs, dof_id_type index, Real value)
{
  mooseAssert(index < MOOSE_AD_MAX_DOFS_PER_ELEM,
              "The requested derivative index "
                  << index << " is not less than " << MOOSE_AD_MAX_DOFS_PER_ELEM
                  << ". You can run `configure --with-derivative-size=<n>` to request a larger "
                     "derivative container.");
  derivs[index] = value;
}
}
