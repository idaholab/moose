//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAdvection.h"

registerADMooseObject("MooseApp", FVAdvection);

InputParameters
FVAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  return params;
}

FVAdvection::FVAdvection(const InputParameters & params)
  : FVFluxKernel(params), _velocity(getParam<RealVectorValue>("velocity"))
{
}

ADReal
FVAdvection::computeQpResidual()
{
  ADReal u_interface;
  interpolate(InterpMethod::Upwind, u_interface, _u_elem[_qp], _u_neighbor[_qp], _velocity);
  return _normal * _velocity * u_interface;
}
