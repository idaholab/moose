//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  // These only contribute to the diagonal of the matrix, so we just get
  // the contribution and insert it immediately. We add it to every tagged matrix.
  const auto mx_contrib = computeMatrixContribution();
  for (auto & matrix : _matrices)
    (*matrix).add(_dof_id, _dof_id, mx_contrib);
}

void
LinearFVElementalKernel::addRightHandSideContribution()
{
  // These only contribute to one entry of the right hand side, so we just get
  // the contribution and insert it immediately. We add it to every tagged
  // vector.
  const auto rhs_contrib = computeRightHandSideContribution();
  for (auto & vector : _vectors)
    (*vector).add(_dof_id, rhs_contrib);
}

void
LinearFVElementalKernel::setCurrentElemInfo(const ElemInfo * elem_info)
{
  _current_elem_info = elem_info;
  _dof_id = _current_elem_info->dofIndices()[_sys_num][_var_num];
}
