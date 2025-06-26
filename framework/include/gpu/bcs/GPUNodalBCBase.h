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
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

namespace Moose
{
namespace Kokkos
{

class NodalBCBase : public BoundaryCondition,
                    public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();

  NodalBCBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  NodalBCBase(const NodalBCBase & object);

  virtual bool preset() const { return false; }
  virtual void presetSolution(TagID tag) {}

  std::vector<dof_id_type> getNodes() const;
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosNodalBCBaseMembers usingKokkosBoundaryConditionMembers;
