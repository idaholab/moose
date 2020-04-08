//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDirichletBC.h"

registerADMooseObject("MooseApp", ADDirichletBC);

defineADLegacyParams(ADDirichletBC);

template <ComputeStage compute_stage>
InputParameters
ADDirichletBC<compute_stage>::validParams()
{
  InputParameters params = ADDirichletBCBase<compute_stage>::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC");
  params.declareControllable("value");
  params.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                             "is a constant, controllable value.");
  return params;
}

template <ComputeStage compute_stage>
ADDirichletBC<compute_stage>::ADDirichletBC(const InputParameters & parameters)
  : ADDirichletBCBase<compute_stage>(parameters), _value(getParam<Real>("value"))
{
}

template <ComputeStage compute_stage>
ADReal
ADDirichletBC<compute_stage>::computeQpValue()
{
  return _value;
}

// explicit instantiation is required for AD base classes
adBaseClass(ADDirichletBC);
