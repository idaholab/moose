//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADConservativeMassImplicitBC.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSADConservativeMassImplicitBC);

InputParameters
INSADConservativeMassImplicitBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription("This class computes the mass equation residual and Jacobian "
                             "contributions (the latter using automatic differentiation) for the "
                             "incompressible Navier-Stokes "
                             "equations.");
  params.addRequiredCoupledVar("velocity", "The velocity");
  return params;
}

INSADConservativeMassImplicitBC::INSADConservativeMassImplicitBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _vel(adCoupledVectorValue("velocity"))

{
}

ADReal
INSADConservativeMassImplicitBC::computeQpResidual()
{
  return -_vel[_qp] * _normals[_qp] * _test[_i][_qp];
}
