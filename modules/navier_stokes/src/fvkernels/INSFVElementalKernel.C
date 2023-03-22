//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVElementalKernel.h"

InputParameters
INSFVElementalKernel::validParams()
{
  auto params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  return params;
}

INSFVElementalKernel::INSFVElementalKernel(const InputParameters & params)
  : FVElementalKernel(params), INSFVMomentumResidualObject(*this)
{
}

void
INSFVElementalKernel::processResidualAndJacobian(const ADReal & residual,
                                                 const dof_id_type dof_index)
{
  _assembly.processResidualAndJacobian(residual, dof_index, _vector_tags, _matrix_tags);
}
