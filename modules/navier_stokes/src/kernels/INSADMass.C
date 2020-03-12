//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMass.h"

registerADMooseObject("NavierStokesApp", INSADMass);

defineADLegacyParams(INSADMass);

template <ComputeStage compute_stage>
InputParameters
INSADMass<compute_stage>::validParams()
{
  InputParameters params = ADKernelValue<compute_stage>::validParams();
  params.addClassDescription("This class computes the mass equation residual and Jacobian "
                             "contributions (the latter using automatic differentiation) for the "
                             "incompressible Navier-Stokes "
                             "equations.");
  return params;
}

template <ComputeStage compute_stage>
INSADMass<compute_stage>::INSADMass(const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters),
    _mass_strong_residual(getADMaterialProperty<Real>("mass_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADReal
INSADMass<compute_stage>::precomputeQpResidual()
{
  return _mass_strong_residual[_qp];
}
