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
  params.addParam<MaterialPropertyName>(
      "advected_quantity",
      "An optional parameter for specifying an advected quantity from a material property. If this "
      "is not specified, then the advected quantity will simply be the variable that this object "
      "is acting on");
  return params;
}

FVMatAdvection::FVMatAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _vel_elem(getADMaterialProperty<RealVectorValue>("vel")),
    _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>("vel")),
    _adv_quant_elem(isParamValid("advected_quantity")
                        ? &getADMaterialProperty<Real>("advected_quantity").get()
                        : &_u_elem),
    _adv_quant_neighbor(isParamValid("advected_quantity")
                            ? &getNeighborADMaterialProperty<Real>("advected_quantity").get()
                            : &_u_neighbor)
{
}

ADReal
FVMatAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal u_interface;
  interpolate(InterpMethod::Average, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  interpolate(
      InterpMethod::Upwind, u_interface, (*_adv_quant_elem)[_qp], (*_adv_quant_neighbor)[_qp], v);
  return _normal * v * u_interface;
}
