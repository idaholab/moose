//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUBoundaryCondition.h"

#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

namespace Moose
{
namespace Kokkos
{

class IntegratedBCBase : public BoundaryCondition,
                         public CoupleableMooseVariableDependencyIntermediateInterface,
                         public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  IntegratedBCBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  IntegratedBCBase(const IntegratedBCBase & object);
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosIntegratedBCBaseMembers usingKokkosBoundaryConditionMembers;
