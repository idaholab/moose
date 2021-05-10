//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADLowerBoundNodalKernel.h"

registerMooseObject("MooseTestApp", ADLowerBoundNodalKernel);

InputParameters
ADLowerBoundNodalKernel::validParams()
{
  InputParameters params = ADNodalKernel::validParams();
  params.addRequiredCoupledVar(
      "v", "The coupled variable we require to be greater than the lower bound");
  params.addParam<Real>("lower_bound", "The lower bound on the coupled variable");
  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundaries",
      "Boundaries on which not to execute the NodalKernel. This can be useful for avoiding "
      "singularity in the matrix in case a constraint is active in the same place that a "
      "DirichletBC is set");
  return params;
}

ADLowerBoundNodalKernel::ADLowerBoundNodalKernel(const InputParameters & parameters)
  : ADNodalKernel(parameters),
    _v_var(coupled("v")),
    _v(adCoupledValue("v")),
    _lower_bound(getParam<Real>("lower_bound"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with "
               "ADLowerBoundNodalKernel");

  const auto & bnd_names = getParam<std::vector<BoundaryName>>("exclude_boundaries");
  for (const auto & bnd_name : bnd_names)
    _bnd_ids.insert(_mesh.getBoundaryID(bnd_name));
}

ADReal
ADLowerBoundNodalKernel::computeQpResidual()
{
  for (auto bnd_id : _bnd_ids)
    if (_mesh.isBoundaryNode(_current_node->id(), bnd_id))
      return _u[_qp];

  return std::min(_u[_qp], _v[_qp] - _lower_bound);
}
