//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVBurgerOutflowBC.h"

registerADMooseObject("MooseTestApp", FVBurgerOutflowBC);

template <ComputeStage compute_stage>
InputParameters
FVBurgerOutflowBC<compute_stage>::validParams()
{
  InputParameters params = FVFluxBC<compute_stage>::validParams();
  return params;
}

template <ComputeStage compute_stage>
FVBurgerOutflowBC<compute_stage>::FVBurgerOutflowBC(const InputParameters & parameters)
  : FVFluxBC<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADReal
FVBurgerOutflowBC<compute_stage>::computeQpResidual()
{
  mooseAssert(_face_info->leftElem().dim() == 1, "FVBurgerOutflowBC works only in 1D");

  ADReal r = 0;
  // only add this on outflow faces
  if (_u[_qp] * _normal(0) > 0)
    r = 0.5 * _u[_qp] * _u[_qp] * _normal(0);
  return r;
}
