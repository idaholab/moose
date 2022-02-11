//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatFluxBaseBC.h"
#include "ADHeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"

InputParameters
ADHeatFluxBaseBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "q_uo", "The name of the user object that computes the heat flux");
  params.addRequiredParam<Real>("P_hs_unit", "Perimeter of a single unit of heat structure");
  params.addRequiredParam<unsigned int>("n_unit", "Number of units of heat structure");
  params.addRequiredParam<bool>("hs_coord_system_is_cylindrical",
                                "Is the heat structure coordinate system cylindrical?");
  return params;
}

ADHeatFluxBaseBC::ADHeatFluxBaseBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _q_uo(getUserObject<ADHeatFluxFromHeatStructureBaseUserObject>("q_uo")),
    _P_hs_unit(getParam<Real>("P_hs_unit")),
    _n_unit(getParam<unsigned int>("n_unit")),
    _hs_coord_system_is_cylindrical(getParam<bool>("hs_coord_system_is_cylindrical")),
    _hs_coord(_hs_coord_system_is_cylindrical ? _P_hs_unit : 1.0),
    _hs_scale(-_hs_coord / (_n_unit * _P_hs_unit))
{
}
