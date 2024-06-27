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
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} \\vec{u}$ which in its weak "
                             "form is given by: $(-\\nabla \\psi_i, \\vec{v} \\vec{u})$.");
  params.addRequiredCoupledVar("velocity", "The velocity");
  return params;
}

INSADConservativeMass::INSADConservativeMass(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _velocity(adCoupledVectorValue("velocity"))
{
}

ADRealVectorValue
INSADConservativeMass::precomputeQpResidual()
{
  return _velocity[_qp];
}