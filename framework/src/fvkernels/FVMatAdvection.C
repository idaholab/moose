//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMatAdvection.h"

registerADMooseObject("MooseApp", FVMatAdvection);

InputParameters
FVMatAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("vel", "advection velocity");
  return params;
}

FVMatAdvection::FVMatAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _vel_elem(getADMaterialProperty<RealVectorValue>("vel")),
    _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>("vel"))
{
}

ADReal
FVMatAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal u_interface;
  interpolate(InterpMethod::Average, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  interpolate(InterpMethod::Upwind, u_interface, advQuantity(), advQuantityNeighbor(), v);
  return _normal * v * u_interface;
}

const ADReal &
FVMatAdvection::advQuantity()
{
  return _u_elem[_qp];
}

const ADReal &
FVMatAdvection::advQuantityNeighbor()
{
  return _u_neighbor[_qp];
}
