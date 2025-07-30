//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLowerBoundNodalKernel.h"

registerMooseObject("MooseApp", KokkosLowerBoundNodalKernel);

InputParameters
KokkosLowerBoundNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Used to prevent a coupled variable from going below a lower bound");
  params.addRequiredCoupledVar(
      "v", "The coupled variable we require to be greater than the lower bound");
  params.addParam<Real>("lower_bound", 0, "The lower bound on the coupled variable");
  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundaries",
      {},
      "Boundaries on which not to execute the NodalKernel. This can be useful for avoiding "
      "singuarility in the matrix in case a constraint is active in the same place that a "
      "DirichletBC is set");
  return params;
}

KokkosLowerBoundNodalKernel::KokkosLowerBoundNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters),
    _v_var(coupled("v")),
    _v(kokkosCoupledNodalValue("v")),
    _lower_bound(getParam<Real>("lower_bound"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with "
               "LowerBoundNodalKernel");

  std::set<BoundaryID> bnd_ids;

  const auto & bnd_names = getParam<std::vector<BoundaryName>>("exclude_boundaries");
  for (const auto & bnd_id : _mesh.getBoundaryIDs(bnd_names))
    bnd_ids.insert(kokkosMesh().getBoundaryID(bnd_id));

  _bnd_ids = bnd_ids;
}
