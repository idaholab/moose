//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumTimeDerivative.h"

registerMooseObject("NavierStokesApp", INSADMomentumTimeDerivative);

InputParameters
INSADMomentumTimeDerivative::validParams()
{
  InputParameters params = ADTimeKernelValue::validParams();
  params.addClassDescription("This class computes the time derivative for the incompressible "
                             "Navier-Stokes momentum equation.");
  params.addCoupledVar("temperature",
                       "The temperature on which material properties may depend. If properties "
                       "do depend on temperature, this variable must be coupled in in order to "
                       "correctly resize the element matrix");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "density name");
  return params;
}

INSADMomentumTimeDerivative::INSADMomentumTimeDerivative(const InputParameters & parameters)
  : ADVectorTimeKernelValue(parameters), _rho(getADMaterialProperty<Real>("rho_name"))
{
}

ADRealVectorValue
INSADMomentumTimeDerivative::precomputeQpResidual()
{
  return _rho[_qp] * _u_dot[_qp];
}
