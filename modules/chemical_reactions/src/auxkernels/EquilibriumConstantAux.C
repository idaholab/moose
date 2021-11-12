//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EquilibriumConstantAux.h"

registerMooseObject("ChemicalReactionsApp", EquilibriumConstantAux);

InputParameters
EquilibriumConstantAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addCoupledVar(
      "temperature", 298.15, "The temperature of the aqueous phase (K). Default is 298.15K");
  params.addRequiredParam<std::vector<Real>>(
      "temperature_points", "Temperature points where log(Keq) data is evaluated (K)");
  params.addRequiredParam<std::vector<Real>>(
      "logk_points", "log(Keq) data evaluated at each value of temperature_points");
  params.addClassDescription(
      "Equilibrium constant for a given equilibrium species (in form log10(Keq))");
  return params;
}

EquilibriumConstantAux::EquilibriumConstantAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _temperature(coupledValue("temperature")),
    _temperature_points(getParam<std::vector<Real>>("temperature_points")),
    _logk_points(getParam<std::vector<Real>>("logk_points"))
{
  // Check that the number of temperature_points and logk_points are equal
  if (_temperature_points.size() != _logk_points.size())
    mooseError("The number of temperature_points and logk_points must be equal in ", _name);

  if (_temperature_points.size() >= 5)
  {
    // If there at least 5 values, then use the Maier-Kelley fit
    _logk = std::make_unique<EquilibriumConstantFit>(_temperature_points, _logk_points);
    _logk->generate();
  }
  else if ((_temperature_points.size() >= 2) && (_temperature_points.size() <= 4))
  {
    // If between 2 and 4 values are provided, use a linear fit
    _linear_logk = std::make_unique<PolynomialFit>(_temperature_points, _logk_points, 1);
    _linear_logk->generate();
  }
}

Real
EquilibriumConstantAux::computeValue()
{
  if (_temperature_points.size() == 1)
    return -_logk_points[0];

  if (_temperature_points.size() > 5)
    return -_logk->sample(_temperature[_qp]);

  return -_linear_logk->sample(_temperature[_qp]);
}
