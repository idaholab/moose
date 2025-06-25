//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMVectorDirichletBCBase.h"

InputParameters
MFEMVectorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "0. 0. 0.",
      "Vector coefficient specifying the values the variable takes on the boundary");
  return params;
}

MFEMVectorDirichletBCBase::MFEMVectorDirichletBCBase(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef(getVectorCoefficient(getParam<MFEMVectorCoefficientName>("vector_coefficient")))
{
}

#endif
