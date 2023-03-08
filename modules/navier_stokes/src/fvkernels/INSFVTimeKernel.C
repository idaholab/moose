//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTimeKernel.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
INSFVTimeKernel::validParams()
{
  auto params = FVFunctorTimeKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  return params;
}

INSFVTimeKernel::INSFVTimeKernel(const InputParameters & params)
  : FVFunctorTimeKernel(params), INSFVMomentumResidualObject(*this)
{
}

void
INSFVTimeKernel::processResidualAndJacobian(const ADReal & residual, const dof_id_type dof_index)
{
  _assembly.processResidualAndJacobian(residual, dof_index, _vector_tags, _matrix_tags);
}
