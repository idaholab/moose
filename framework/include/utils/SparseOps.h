//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/metaphysicl_exceptions.h"

namespace Moose
{
template <std::size_t N>
inline void
derivInsert(SemiDynamicSparseNumberArray<Real, libMesh::dof_id_type, NWrapper<N>> & derivs,
            libMesh::dof_id_type index,
            Real value)
{
#ifndef NDEBUG
  try
  {
    derivs.insert(index) = value;
  }
  catch (MetaPhysicL::LogicError &)
  {
    mooseError("The last insertion into the sparse derivative storage container exceeded the "
               "underlying array size. Consider running `configure --with-derivative-size=<n>` to "
               "obtain a larger underlying container");
  }
#else
  derivs.insert(index) = value;
#endif
}
}
