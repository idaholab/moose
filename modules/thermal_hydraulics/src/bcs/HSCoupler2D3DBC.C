//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSCoupler2D3DBC.h"
#include "HSCoupler2D3DUserObject.h"

registerMooseObject("ThermalHydraulicsApp", HSCoupler2D3DBC);

InputParameters
HSCoupler2D3DBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();

  params.addRequiredParam<UserObjectName>("hs_coupler_2d3d_uo", "The HSCoupler2D3D user object");
  params.addClassDescription("Adds boundary heat flux terms for HSCoupler2D3D");

  return params;
}

HSCoupler2D3DBC::HSCoupler2D3DBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),

    _hs_coupler_2d3d_uo(getUserObject<HSCoupler2D3DUserObject>("hs_coupler_2d3d_uo"))
{
}

ADReal
HSCoupler2D3DBC::computeQpResidual()
{
  return -_hs_coupler_2d3d_uo.getHeatFlux(_current_elem->id())[_qp] * _test[_i][_qp];
}
