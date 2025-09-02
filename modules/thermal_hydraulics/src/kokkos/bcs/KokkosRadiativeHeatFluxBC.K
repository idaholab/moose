//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosRadiativeHeatFluxBC.h"

registerMooseObject("ThermalHydraulicsApp", KokkosRadiativeHeatFluxBC);

InputParameters
KokkosRadiativeHeatFluxBC::validParams()
{
  InputParameters params = KokkosRadiativeHeatFluxBCBase::validParams();

  params.addRequiredParam<Real>("boundary_emissivity", "Emissivity of the boundary.");
  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a plate heat structure");

  return params;
}

KokkosRadiativeHeatFluxBC::KokkosRadiativeHeatFluxBC(const InputParameters & parameters)
  : KokkosRadiativeHeatFluxBCBase(parameters),
    _eps_boundary(getParam<Real>("boundary_emissivity")),
    _scale_pp(getPostprocessorValue("scale_pp"))
{
}
