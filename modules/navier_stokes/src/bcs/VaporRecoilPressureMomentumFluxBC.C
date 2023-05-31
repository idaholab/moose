/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

#include "VaporRecoilPressureMomentumFluxBC.h"

registerMooseObject("MalamuteApp", VaporRecoilPressureMomentumFluxBC);

InputParameters
VaporRecoilPressureMomentumFluxBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addClassDescription("Vapor recoil pressure momentum flux");
  params.addParam<MaterialPropertyName>("rc_pressure_name", "rc_pressure", "The recoil pressure");
  params.addCoupledVar("temperature", "The temperature on which the recoil pressure depends");
  return params;
}

VaporRecoilPressureMomentumFluxBC::VaporRecoilPressureMomentumFluxBC(
    const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters), _rc_pressure(getADMaterialProperty<Real>("rc_pressure_name"))
{
}

ADReal
VaporRecoilPressureMomentumFluxBC::computeQpResidual()
{
  return _test[_i][_qp] *
         ADRealVectorValue(
             std::abs(_normals[_qp](0)), std::abs(_normals[_qp](1)), std::abs(_normals[_qp](2))) *
         _rc_pressure[_qp];
}
