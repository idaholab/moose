//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHBase.h"

defineADLegacyParams(ADSplitCHBase);

template <ComputeStage compute_stage>
InputParameters
ADSplitCHBase<compute_stage>::validParams()
{
  InputParameters params = ADKernel<compute_stage>::validParams();
  params.addClassDescription("Base class for split Cahn-Hilliard equation.");
  return params;
}

template <ComputeStage compute_stage>
ADSplitCHBase<compute_stage>::ADSplitCHBase(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADReal
ADSplitCHBase<compute_stage>::computeQpResidual()
{
  return computeDFDC() * _test[_i][_qp];
}

template <ComputeStage compute_stage>
ADReal
ADSplitCHBase<compute_stage>::computeDFDC()
{
  return 0.0;
}

// explicit instantiation is required for AD base classes
adBaseClass(ADSplitCHBase);
