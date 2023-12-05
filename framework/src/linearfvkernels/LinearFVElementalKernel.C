//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVElementalKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
LinearFVElementalKernel::validParams()
{
  InputParameters params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("LinearFVElementalKernel");
  return params;
}

LinearFVElementalKernel::LinearFVElementalKernel(const InputParameters & params)
  : LinearFVKernel(params), _current_elem_info(nullptr)
{
}

void
LinearFVElementalKernel::addMatrixContribution()
{
  const auto dof_id = _current_elem_info->dofIndices()[_var->sys().number()][_var->number()];
  (*_linear_system.matrix).add(dof_id, dof_id, computeMatrixContribution());
}

void
LinearFVElementalKernel::addRightHandSideContribution()
{
  const auto dof_id = _current_elem_info->dofIndices()[_var->sys().number()][_var->number()];
  (*_linear_system.rhs).add(dof_id, computeRightHandSideContribution());
}
