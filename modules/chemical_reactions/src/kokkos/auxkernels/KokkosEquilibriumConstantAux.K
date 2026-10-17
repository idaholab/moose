//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosEquilibriumConstantAux.h"
#include "EquilibriumConstantFit.h"
#include "PolynomialFit.h"

registerKokkosAuxKernel("ChemicalReactionsApp", KokkosEquilibriumConstantAux);

InputParameters
KokkosEquilibriumConstantAux::validParams()
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

KokkosEquilibriumConstantAux::KokkosEquilibriumConstantAux(const InputParameters & parameters)
  : AuxKernel(parameters), _temperature(kokkosCoupledValue("temperature"))
{
  const auto & temperature_points = getParam<std::vector<Real>>("temperature_points");
  const auto & logk_points = getParam<std::vector<Real>>("logk_points");

  // Check that the number of temperature_points and logk_points are equal
  if (temperature_points.size() != logk_points.size())
    mooseError("The number of temperature_points and logk_points must be equal in ", _name);

  if (temperature_points.size() >= 5)
  {
    // If there at least 5 values, then use the Maier-Kelley fit
    EquilibriumConstantFit fit(temperature_points, logk_points);
    fit.generate();
    _fit_type = FitType::MAIER_KELLEY;
    _coeffs = fit.getCoefficients();
  }
  else if (temperature_points.size() >= 2)
  {
    // If between 2 and 4 values are provided, use a linear fit
    PolynomialFit fit(temperature_points, logk_points, 1);
    fit.generate();
    _fit_type = FitType::LINEAR;
    _coeffs = fit.getCoefficients();
  }
  else
  {
    _fit_type = FitType::CONSTANT;
    _coeffs = logk_points;
  }
}
