//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexEssentialBC.h"

namespace Moose::MFEM
{
InputParameters
ComplexEssentialBC::validParams()
{
  InputParameters params = BoundaryCondition::validParams();
  return params;
}

ComplexEssentialBC::ComplexEssentialBC(const InputParameters & parameters)
  : BoundaryCondition(parameters)
{
}

} // namespace Moose::MFEM
#endif
