//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UpperBoundNodalKernel.h"

registerMooseObject("MooseApp", UpperBoundNodalKernel);

InputParameters
UpperBoundNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Used to prevent a coupled variable from going above a upper bound");
  params.addRequiredCoupledVar(
      "v", "The coupled variable we require to be greater than the upper bound");
  params.addParam<Real>("upper_bound", 0, "The upper bound on the coupled variable");
  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundaries",
      "Boundaries on which not to execute the NodalKernel. This can be useful for avoiding "
      "singuarility in the matrix in case a constraint is active in the same place that a "
      "DirichletBC is set");
  return params;
}

UpperBoundNodalKernel::UpperBoundNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v")),
    _upper_bound(getParam<Real>("upper_bound"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with "
               "UpperBoundNodalKernel");

  const auto & bnd_names = getParam<std::vector<BoundaryName>>("exclude_boundaries");
  for (const auto & bnd_name : bnd_names)
    _bnd_ids.insert(_mesh.getBoundaryID(bnd_name));
}

Real
UpperBoundNodalKernel::computeQpResidual()
{
  for (auto bnd_id : _bnd_ids)
    if (_mesh.isBoundaryNode(_current_node->id(), bnd_id))
      return _u[_qp];

  return std::min(_u[_qp], _upper_bound - _v[_qp]);
}

Real
UpperBoundNodalKernel::computeQpJacobian()
{
  for (auto bnd_id : _bnd_ids)
    if (_mesh.isBoundaryNode(_current_node->id(), bnd_id))
      return 1;

  if (_u[_qp] <= _upper_bound - _v[_qp])
    return 1;
  return 0;
}

Real
UpperBoundNodalKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (auto bnd_id : _bnd_ids)
    if (_mesh.isBoundaryNode(_current_node->id(), bnd_id))
      return 0;

  if (jvar == _v_var)
    if (_upper_bound - _v[_qp] < _u[_qp])
      return -1;

  return 0.0;
}
