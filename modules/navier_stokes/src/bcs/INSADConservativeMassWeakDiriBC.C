//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADConservativeMassWeakDiriBC.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSADConservativeMassWeakDiriBC);

InputParameters
INSADConservativeMassWeakDiriBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription("This class computes the mass equation residual and Jacobian "
                             "contributions (the latter using automatic differentiation) for the "
                             "incompressible Navier-Stokes "
                             "equations.");
  params.addRequiredParam<FunctionName>("velocity", "The prescribed velocity");
  return params;
}

INSADConservativeMassWeakDiriBC::INSADConservativeMassWeakDiriBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _diri_vel(getFunction("velocity"))

{
}

ADReal
INSADConservativeMassWeakDiriBC::computeQpResidual()
{
  return -_diri_vel.vectorValue(_t, _q_point[_qp]) * _normals[_qp] * _test[_i][_qp];
}
