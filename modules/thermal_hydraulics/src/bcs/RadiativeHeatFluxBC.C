//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadiativeHeatFluxBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", RadiativeHeatFluxBC);

InputParameters
RadiativeHeatFluxBC::validParams()
{
  InputParameters params = RadiativeHeatFluxBCBase::validParams();

  params.addRequiredParam<Real>("boundary_emissivity", "Emissivity of the boundary.");
  params.addParam<FunctionName>("view_factor", "1", "View factor function");
  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a plate heat structure");

  return params;
}

RadiativeHeatFluxBC::RadiativeHeatFluxBC(const InputParameters & parameters)
  : RadiativeHeatFluxBCBase(parameters),
    _eps_boundary(getParam<Real>("boundary_emissivity")),
    _view_factor_fn(getFunction("view_factor")),
    _scale_pp(getPostprocessorValue("scale_pp"))
{
}

Real
RadiativeHeatFluxBC::coefficient() const
{
  return _scale_pp * _eps_boundary * _view_factor_fn.value(_t, _q_point[_qp]);
}
