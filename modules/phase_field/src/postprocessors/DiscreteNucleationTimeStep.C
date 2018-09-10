//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationTimeStep.h"
#include "DiscreteNucleationInserter.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationTimeStep);

template <>
InputParameters
validParams<DiscreteNucleationTimeStep>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription(
      "Return a timestep limit for nucleation event to be used by IterationAdaptiveDT");
  params.addRequiredParam<Real>("dt_max",
                                "Timestep to cut back to at the start of sa nucleation event");
  params.addRequiredParam<UserObjectName>("inserter", "DiscreteNucleationInserter user object");
  return params;
}

DiscreteNucleationTimeStep::DiscreteNucleationTimeStep(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _inserter(getUserObject<DiscreteNucleationInserter>("inserter")),
    _dt_nucleation(getParam<Real>("dt_max"))
{
}

PostprocessorValue
DiscreteNucleationTimeStep::getValue()
{
  if (_inserter.isMapUpdateRequired())
    return _dt_nucleation;
  return std::numeric_limits<Real>::max();
}
