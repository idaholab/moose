//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADVaporRecoilPressureMomentumFluxBC.h"

registerMooseObject("NavierStokesApp", INSADVaporRecoilPressureMomentumFluxBC);

InputParameters
INSADVaporRecoilPressureMomentumFluxBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addClassDescription("Vapor recoil pressure momentum flux");
  params.addParam<MaterialPropertyName>("rc_pressure_name", "rc_pressure", "The recoil pressure");
  return params;
}

INSADVaporRecoilPressureMomentumFluxBC::INSADVaporRecoilPressureMomentumFluxBC(
    const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters), _rc_pressure(getADMaterialProperty<Real>("rc_pressure_name"))
{
}

ADReal
INSADVaporRecoilPressureMomentumFluxBC::computeQpResidual()
{
  return _test[_i][_qp] * _normals[_qp] * _rc_pressure[_qp];
}
