//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSCoupler2D2DRadiationRZBC.h"
#include "HSCoupler2D2DRadiationUserObject.h"

registerMooseObject("ThermalHydraulicsApp", HSCoupler2D2DRadiationRZBC);

InputParameters
HSCoupler2D2DRadiationRZBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params += RZSymmetry::validParams();

  params.addRequiredParam<UserObjectName>("hs_coupler_2d2d_uo",
                                          "The HSCoupler2D2DRadiation user object");
  params.addClassDescription("Adds boundary heat flux terms for HSCoupler2D2DRadiation");

  return params;
}

HSCoupler2D2DRadiationRZBC::HSCoupler2D2DRadiationRZBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    RZSymmetry(this, parameters),

    _hs_coupler_2d2d_uo(getUserObject<HSCoupler2D2DRadiationUserObject>("hs_coupler_2d2d_uo"))
{
}

ADReal
HSCoupler2D2DRadiationRZBC::computeQpResidual()
{
  return _hs_coupler_2d2d_uo.getHeatFlux(_current_elem->id())[_qp] *
         computeCircumference(_q_point[_qp]) * _test[_i][_qp];
}
