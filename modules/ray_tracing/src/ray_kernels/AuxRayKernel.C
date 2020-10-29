//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxRayKernel.h"

// MOOSE includes
#include "AuxiliarySystem.h"

InputParameters
AuxRayKernel::validParams()
{
  auto params = RayKernelBase::validParams();

  params.addRequiredParam<AuxVariableName>(
      "variable", "The name of the aux variable that this AuxRayKernel applies to");

  return params;
}

// Static mutex definition
Threads::spin_mutex AuxRayKernel::_add_value_mutex;

AuxRayKernel::AuxRayKernel(const InputParameters & params)
  : RayKernelBase(params),
    MooseVariableInterface<Real>(this,
                                 _fe_problem.getAuxiliarySystem()
                                     .getVariable(_tid, params.get<AuxVariableName>("variable"))
                                     .isNodal(),
                                 "variable",
                                 Moose::VarKindType::VAR_AUXILIARY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _aux(_fe_problem.getAuxiliarySystem()),
    _var(*mooseVariable())
{
  if (_var.feType() != FEType(CONSTANT, MONOMIAL))
    paramError("variable", "Only CONSTANT MONOMIAL variables are supported");

  addMooseVariableDependency(mooseVariable());
}

void
AuxRayKernel::addValue(const Real value)
{
  // TODO: this is horribly inefficient. Consider caching and adding later
  Threads::spin_mutex::scoped_lock lock(_add_value_mutex);
  _var.setNodalValue(value);
  _var.add(_aux.solution());
}
