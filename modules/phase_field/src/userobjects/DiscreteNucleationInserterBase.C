//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationInserterBase.h"

template <>
InputParameters
validParams<DiscreteNucleationInserterBase>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

DiscreteNucleationInserterBase::DiscreteNucleationInserterBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _global_nucleus_list(declareRestartableData("global_nucleus_list", NucleusList(0))),
    _changes_made(0, 0),
    _update_required(_app.isRecovering() || _app.isRestarting())
{
  setRandomResetFrequency(EXEC_TIMESTEP_END);
}
