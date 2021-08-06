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
  auto params = FVTimeKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  return params;
}

INSFVTimeKernel::INSFVTimeKernel(const InputParameters & params)
  : FVTimeKernel(params), INSFVMomentumResidualObject(*this)
{
}

void
INSFVTimeKernel::processResidual(const ADReal & residual, const dof_id_type dof_index)
{
  if (_subproblem.currentlyComputingJacobian())
    _assembly.processDerivatives(residual, dof_index, _matrix_tags);
  else
    _assembly.processResidual(residual.value(), dof_index, _vector_tags);
}
