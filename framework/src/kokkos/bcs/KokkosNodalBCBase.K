//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalBCBase.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
NodalBCBase::validParams()
{
  auto params = BoundaryCondition::validParams();

  return params;
}

NodalBCBase::NodalBCBase(const InputParameters & parameters, Moose::VarFieldType field_type)
  : BoundaryCondition(parameters, field_type, true),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true)
{
}

NodalBCBase::NodalBCBase(const NodalBCBase & object)
  : BoundaryCondition(object), CoupleableMooseVariableDependencyIntermediateInterface(&object, true)
{
}

std::vector<dof_id_type>
NodalBCBase::getNodes() const
{
  std::vector<dof_id_type> nodes(numKokkosBoundaryNodes());

  for (unsigned int i = 0; i < numKokkosBoundaryNodes(); ++i)
    nodes[i] = kokkosBoundaryNodeID(i);

  return nodes;
}

} // namespace Kokkos
} // namespace Moose
