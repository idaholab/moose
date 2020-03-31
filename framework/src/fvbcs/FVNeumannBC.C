//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVNeumannBC.h"

registerADMooseObject("MooseApp", FVNeumannBC);

template <ComputeStage compute_stage>
InputParameters
FVNeumannBC<compute_stage>::validParams()
{
  InputParameters params = FVFluxBC<compute_stage>::validParams();
  params.addParam<Real>("value", 0.0, "The value of the flux crossing the boundary.");
  return params;
}

template <ComputeStage compute_stage>
FVNeumannBC<compute_stage>::FVNeumannBC(const InputParameters & parameters)
  : FVFluxBC<compute_stage>(parameters), _value(getParam<Real>("value"))
{
}

template <ComputeStage compute_stage>
ADReal
FVNeumannBC<compute_stage>::computeQpResidual()
{
  return -_value;
}
