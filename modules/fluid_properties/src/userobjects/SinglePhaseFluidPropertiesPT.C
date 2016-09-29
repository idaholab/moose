/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SinglePhaseFluidPropertiesPT.h"

template<>
InputParameters validParams<SinglePhaseFluidPropertiesPT>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidPropertiesPT::SinglePhaseFluidPropertiesPT(const InputParameters & parameters) :
    FluidProperties(parameters),
    _R(8.3144621),
    _T_c2k(273.15)
{
}

SinglePhaseFluidPropertiesPT::~SinglePhaseFluidPropertiesPT()
{
}

Real
SinglePhaseFluidPropertiesPT::gamma(Real pressure, Real temperature) const
{
  return cp(pressure, temperature) / cv(pressure, temperature);
}

Real
SinglePhaseFluidPropertiesPT::henryConstantIAPWS(Real temperature, Real A, Real B, Real C) const
{
  Real Tr = temperature / 647.096;
  Real tau = 1.0 - Tr;

  Real lnkh = A / Tr + B * std::pow(tau, 0.355) / Tr + C * std::pow(Tr, -0.41) * std::exp(tau);

  // The vapor pressure used in this formulation
  std::vector<Real> a {-7.85951783, 1.84408259, -11.7866497, 22.6807411,
    -15.9618719, 1.80122502};
  std::vector<Real> b {1.0, 1.5, 3.0, 3.5, 4.0, 7.5};
  Real sum = 0.0;

  for (unsigned int i = 0; i < a.size(); ++i)
    sum += a[i] * std::pow(tau, b[i]);

  return 22.064e6 * std::exp(sum / Tr) * std::exp(lnkh);
}
