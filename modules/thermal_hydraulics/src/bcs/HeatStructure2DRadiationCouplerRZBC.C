//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DRadiationCouplerRZBC.h"
#include "HeatConductionNames.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructure2DRadiationCouplerRZBC);

InputParameters
HeatStructure2DRadiationCouplerRZBC::validParams()
{
  InputParameters params = HeatStructure2DCouplerBCBase::validParams();
  params += RZSymmetry::validParams();

  params.addRequiredParam<Real>("emissivity", "Emissivity function of this boundary");
  params.addRequiredParam<Real>("coupled_emissivity",
                                "Emissivity function of the coupled boundary");
  params.addRequiredParam<Real>("view_factor", "View factor of this boundary");
  params.addRequiredParam<Real>("area", "Area of this boundary");
  params.addRequiredParam<Real>("coupled_area", "Area of the coupled boundary");
  params.addParam<Real>("stefan_boltzmann_constant",
                        HeatConduction::Constants::sigma,
                        "Stefan Boltzmann constant [W/(m^2-K^4)]. This constant is provided as a "
                        "parameter to allow different precisions.");

  params.addClassDescription("Applies BC for HeatStructure2DRadiationCouplerRZ");

  return params;
}

HeatStructure2DRadiationCouplerRZBC::HeatStructure2DRadiationCouplerRZBC(
    const InputParameters & parameters)
  : HeatStructure2DCouplerBCBase(parameters),
    RZSymmetry(this, parameters),

    _emissivity(getParam<Real>("emissivity")),
    _coupled_emissivity(getParam<Real>("coupled_emissivity")),
    _view_factor(getParam<Real>("view_factor")),
    _area(getParam<Real>("area")),
    _coupled_area(getParam<Real>("coupled_area")),
    _sigma(getParam<Real>("stefan_boltzmann_constant")),
    _radiation_resistance((1.0 - _emissivity) / _emissivity + 1.0 / _view_factor +
                          (1.0 - _coupled_emissivity) / _coupled_emissivity * _area / _coupled_area)
{
}

ADReal
HeatStructure2DRadiationCouplerRZBC::computeQpResidual()
{
  const auto T_coupled = computeCoupledTemperature();
  const Real circumference = computeCircumference(_q_point[_qp]);
  return _sigma * (std::pow(_u[_qp], 4) - std::pow(T_coupled, 4)) / _radiation_resistance *
         circumference * _test[_i][_qp];
}
