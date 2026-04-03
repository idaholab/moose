//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDirichletBCBase.h"

namespace Moose::Kokkos
{

InputParameters
DirichletBCBase::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<bool>(
      "preset", true, "Whether or not to preset the BC (apply the value before the solve begins).");
  return params;
}

DirichletBCBase::DirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters), _preset(getParam<bool>("preset"))
{
}

void
DirichletBCBase::presetSolution(TagID tag)
{
  _solution_tag = tag;

  Policy policy(0, numKokkosBoundaryNodes());

  auto dispatcher = DispatcherRegistry::build<PresetLoop>(this, type());

  dispatcher->parallelFor(policy);
}

} // namespace Moose::Kokkos
