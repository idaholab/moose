//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorDirichletBCBase.h"

namespace Moose::MFEM
{
InputParameters
VectorDirichletBCBase::validParams()
{
  InputParameters params = EssentialBC::validParams();
  params.addParam<Moose::MFEM::VectorCoefficientName>(
      "vector_coefficient",
      "0. 0. 0.",
      "Vector coefficient specifying the values the variable takes on the boundary");
  return params;
}

VectorDirichletBCBase::VectorDirichletBCBase(const InputParameters & parameters)
  : EssentialBC(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

} // namespace Moose::MFEM
#endif
