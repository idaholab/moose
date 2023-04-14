//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDirichletBCBase.h"

InputParameters
ADDirichletBCBase::validParams()
{
  InputParameters params = ADNodalBC::validParams();
  params.addParam<bool>(
      "preset", true, "Whether or not to preset the BC (apply the value before the solve begins).");
  return params;
}

ADDirichletBCBase::ADDirichletBCBase(const InputParameters & parameters)
  : ADNodalBC(parameters), _preset(getParam<bool>("preset"))
{
}

void
ADDirichletBCBase::computeValue(NumericVector<Number> & current_solution)
{
  mooseAssert(_preset, "BC is not preset");

  if (_var.isNodalDefined())
  {
    const auto dof_idx = _var.nodalDofIndex();
    current_solution.set(dof_idx, MetaPhysicL::raw_value(computeQpValue()));
  }
}

ADReal
ADDirichletBCBase::computeQpResidual()
{
  return _u - computeQpValue();
}
