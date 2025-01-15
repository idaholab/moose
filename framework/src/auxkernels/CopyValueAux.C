//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CopyValueAux.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", CopyValueAux);

InputParameters
CopyValueAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Returns the specified variable as an auxiliary variable with a "
                             "simple copy of the variable values.");
  params.addRequiredCoupledVar("source", "Variable to take the value of.");
  MooseEnum stateEnum("CURRENT=0 OLD=1 OLDER=2", "CURRENT");
  params.addParam<MooseEnum>(
      "state",
      stateEnum,
      "This parameter specifies the state being copied. CURRENT=0 OLD=1 OLDER=2. Copying an older "
      "state allows access to previous solution information if necessary.");
  return params;
}

CopyValueAux::CopyValueAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _state(getParam<MooseEnum>("state")),
    _v(_state == 0   ? coupledValue("source")
       : _state == 1 ? coupledValueOld("source")
                     : coupledValueOlder("source")),
    _source_variable(*getVar("source", 0))
{
  if (_var.feType().family != _source_variable.feType().family)
    paramError("variable",
               "Source (" + Moose::stringify(_source_variable.feType().family) + ") and target (" +
                   Moose::stringify(_var.feType().family) +
                   ") variable have different families. You may use a ProjectionAux "
                   "instead of the CopyValueAux");
  if (_var.feType().order != _source_variable.feType().order)
    paramError("variable",
               "Source (" + Moose::stringify(_source_variable.feType().order) + ") and target (" +
                   Moose::stringify(_var.feType().order) +
                   ") variable are of different orders. You may use a ProjectionAux "
                   "instead of the CopyValueAux");
}

Real
CopyValueAux::computeValue()
{
  return _v[_qp];
}
