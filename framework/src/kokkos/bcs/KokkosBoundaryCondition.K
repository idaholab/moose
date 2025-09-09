//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBoundaryCondition.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
BoundaryCondition::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += BoundaryRestrictableRequired::validParams();

  params.registerBase("BoundaryCondition");

  return params;
}

BoundaryCondition::BoundaryCondition(const InputParameters & parameters,
                                     Moose::VarFieldType field_type,
                                     bool nodal)
  : ResidualObject(parameters, field_type, nodal), BoundaryRestrictableRequired(this, nodal)
{
}

BoundaryCondition::BoundaryCondition(const BoundaryCondition & object)
  : ResidualObject(object), BoundaryRestrictableRequired(object, {})
{
}

} // namespace Kokkos
} // namespace Moose
