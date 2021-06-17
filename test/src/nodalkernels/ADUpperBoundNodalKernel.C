//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADUpperBoundNodalKernel.h"

registerMooseObject("MooseTestApp", ADUpperBoundNodalKernel);

InputParameters
ADUpperBoundNodalKernel::validParams()
{
  InputParameters params = ADNodalKernel::validParams();
  params.addRequiredCoupledVar("v",
                               "The coupled variable we require to be lower than the upper bound");
  params.addParam<Real>("upper_bound", "The upper bound on the coupled variable");
  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundaries",
      "Boundaries on which not to execute the NodalKernel. This can be useful for avoiding "
      "singularity in the matrix in case a constraint is active in the same place that a "
      "DirichletBC is set");
  return params;
}

ADUpperBoundNodalKernel::ADUpperBoundNodalKernel(const InputParameters & parameters)
  : ADNodalKernel(parameters),
    _v_var(coupled("v")),
    _v(adCoupledValue("v")),
    _upper_bound(getParam<Real>("upper_bound"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with "
               "ADUpperBoundNodalKernel");

  const auto & bnd_names = getParam<std::vector<BoundaryName>>("exclude_boundaries");
  for (const auto & bnd_name : bnd_names)
    _bnd_ids.insert(_mesh.getBoundaryID(bnd_name));
}

ADReal
ADUpperBoundNodalKernel::computeQpResidual()
{
  for (auto bnd_id : _bnd_ids)
    if (_mesh.isBoundaryNode(_current_node->id(), bnd_id))
      return _u[_qp];

  return std::min(_u[_qp], _upper_bound - _v[_qp]);
}
