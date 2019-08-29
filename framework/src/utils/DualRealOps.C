//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualRealOps.h"

#ifdef SPARSE_AD
namespace Moose
{
void
derivInsert(DNDerivativeType & derivs, std::size_t index, Real value)
{
  derivs.insert(index) = value;
}
}
#else
namespace Moose
{
void
derivInsert(DNDerivativeType & derivs, std::size_t index, Real value)
{
  derivs[index] = value;
}
}
#endif
