//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageWallTemperature3EqnMaterial.h"

registerMooseObject("ThermalHydraulicsApp", AverageWallTemperature3EqnMaterial);

InputParameters
AverageWallTemperature3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription(
      "Weighted average wall temperature from multiple sources for 1-phase flow");

  params.addRequiredCoupledVar("T_wall_sources",
                               "Vector of wall temperatures from individual sources");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Hw_sources", "Vector of wall heat transfer coefficients from individual sources");
  params.addRequiredCoupledVar("P_hf_sources",
                               "Vector of heated perimeters from individual sources");
  params.addRequiredCoupledVar("T_fluid", "Fluid temperature");
  params.addRequiredCoupledVar("P_hf_total", "Total heat flux perimeter from all sources");

  return params;
}

AverageWallTemperature3EqnMaterial::AverageWallTemperature3EqnMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _T_wall(declareProperty<Real>("T_wall")),
    _n_values(coupledComponents("T_wall_sources")),
    _T_fluid(coupledValue("T_fluid")),
    _P_hf_total(coupledValue("P_hf_total"))
{
  // make sure that numbers of values are consistent
  if (getParam<std::vector<MaterialPropertyName>>("Hw_sources").size() != _n_values)
    mooseError(name(),
               ": The number of wall heat transfer coefficient values"
               " must equal the number of wall temperature values");
  if (coupledComponents("P_hf_sources") != _n_values)
    mooseError(name(),
               ": The number of heated perimeter values"
               " must equal the number of wall temperature values");

  // get all of the variable values
  const std::vector<MaterialPropertyName> & Hw_prop_names =
      getParam<std::vector<MaterialPropertyName>>("Hw_sources");
  for (unsigned int i = 0; i < _n_values; i++)
  {
    _T_wall_sources.push_back(&coupledValue("T_wall_sources", i));
    _Hw_sources.push_back(&getMaterialProperty<Real>(Hw_prop_names[i]));
    _P_hf_sources.push_back(&coupledValue("P_hf_sources", i));
  }
}

void
AverageWallTemperature3EqnMaterial::computeQpProperties()
{
  Real denominator = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    denominator += (*(_Hw_sources[i]))[_qp] * (*(_P_hf_sources[i]))[_qp];

  if (std::abs(denominator) < 1e-15)
  {
    // use alternate definition to avoid division by zero: heated-perimeter-weighted average
    Real numerator = 0;
    for (unsigned int i = 0; i < _n_values; i++)
      numerator += (*(_T_wall_sources[i]))[_qp] * (*(_P_hf_sources[i]))[_qp];
    _T_wall[_qp] = numerator / _P_hf_total[_qp];
  }
  else
  {
    // use normal definition
    Real numerator = 0;
    for (unsigned int i = 0; i < _n_values; i++)
      numerator += (*(_Hw_sources[i]))[_qp] * ((*(_T_wall_sources[i]))[_qp] - _T_fluid[_qp]) *
                   (*(_P_hf_sources[i]))[_qp];
    _T_wall[_qp] = _T_fluid[_qp] + numerator / denominator;
  }
}
