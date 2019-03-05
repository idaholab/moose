//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumTimeDerivative.h"

registerADMooseObject("NavierStokesApp", INSADMomentumTimeDerivative);

defineADValidParams(
    INSADMomentumTimeDerivative,
    ADTimeKernel,
    params.addClassDescription("This class computes the time derivative for the incompressible "
                               "Navier-Stokes momentum equation.");
    params.addCoupledVar("temperature",
                         "The temperature on which material properties may depend. If properties "
                         "do depend on temperature, this variable must be coupled in in order to "
                         "correctly resize the element matrix");
    params.addParam<MaterialPropertyName>("rho_name", "rho", "density name"););

template <ComputeStage compute_stage>
INSADMomentumTimeDerivative<compute_stage>::INSADMomentumTimeDerivative(
    const InputParameters & parameters)
  : ADVectorTimeKernel<compute_stage>(parameters), _rho(adGetADMaterialProperty<Real>("rho_name"))
{
}

template <ComputeStage compute_stage>
ADResidual
INSADMomentumTimeDerivative<compute_stage>::computeQpResidual()
{
  return _test[_i][_qp] * _rho[_qp] * _u_dot[_qp];
}
