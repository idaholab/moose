//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBoundNodalKernel.h"

InputParameters
KokkosBoundNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addRequiredCoupledVar(
      "v", "The coupled variable we require to be greater than the lower bound");
  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundaries",
      {},
      "Boundaries on which not to execute the nodal kernel. This can be useful for avoiding "
      "singuarility in the matrix in case a constraint is active in the same place that a "
      "Dirichlet BC is set");
  return params;
}

KokkosBoundNodalKernel::KokkosBoundNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _v_var(coupled("v")), _v(kokkosCoupledNodalValue("v"))
{
  if (_var.number() == _v_var)
    paramError("v", "Coupled variable needs to be different from 'variable'");
}

void
KokkosBoundNodalKernel::initialSetup()
{
  std::set<ContiguousBoundaryID> bnd_ids;

  const auto & bnd_names = getParam<std::vector<BoundaryName>>("exclude_boundaries");
  for (const auto & bnd_id : _mesh.getBoundaryIDs(bnd_names))
    bnd_ids.insert(kokkosMesh().getContiguousBoundaryID(bnd_id));

  _bnd_ids = bnd_ids;
}
