//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADConservativeMass.h"

registerMooseObject("NavierStokesApp", INSADConservativeMass);

InputParameters
INSADConservativeMass::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("This class computes the mass equation residual and Jacobian "
                             "contributions (the latter using automatic differentiation) for the "
                             "incompressible Navier-Stokes "
                             "equations.");
  params.addRequiredCoupledVar("velocity", "The velocity variable");
  return params;
}

INSADConservativeMass::INSADConservativeMass(const InputParameters & parameters)
  : ADKernelGrad(parameters), _vel(adCoupledVectorValue("velocity"))
{
}

ADRealVectorValue
INSADConservativeMass::precomputeQpResidual()
{
  return _vel[_qp];
}
