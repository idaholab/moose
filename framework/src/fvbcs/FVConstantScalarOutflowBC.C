//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVConstantScalarOutflowBC.h"

registerADMooseObject("MooseApp", FVConstantScalarOutflowBC);

template <ComputeStage compute_stage>
InputParameters
FVConstantScalarOutflowBC<compute_stage>::validParams()
{
  InputParameters params = FVFluxBC<compute_stage>::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  return params;
}

template <ComputeStage compute_stage>
FVConstantScalarOutflowBC<compute_stage>::FVConstantScalarOutflowBC(const InputParameters & parameters)
  : FVFluxBC<compute_stage>(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

template <ComputeStage compute_stage>
ADReal
FVConstantScalarOutflowBC<compute_stage>::computeQpResidual()
{
  return _normal * _velocity * _u[_qp];
}
