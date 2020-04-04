//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateFlash.h"

InputParameters
PorousFlowFluidStateFlash::validParams()
{
  InputParameters params = PorousFlowFluidStateBase::validParams();
  params.addClassDescription("Compositional flash calculations for use in fluid state classes");
  return params;
}

PorousFlowFluidStateFlash::PorousFlowFluidStateFlash(const InputParameters & parameters)
  : PorousFlowFluidStateBase(parameters), _nr_max_its(42), _nr_tol(1.0e-12)
{
}

Real
PorousFlowFluidStateFlash::rachfordRice(Real x,
                                        std::vector<Real> & Zi,
                                        std::vector<Real> & Ki) const
{
  const std::size_t num_z = Zi.size();
  // Check that the sizes of the mass fractions and equilibrium constant vectors are correct
  if (Ki.size() != num_z + 1)
    mooseError("The number of mass fractions or equilibrium components passed to rachfordRice is "
               "not correct");

  Real f = 0.0;
  Real Z_total = 0.0;

  for (std::size_t i = 0; i < num_z; ++i)
  {
    f += Zi[i] * (Ki[i] - 1.0) / (1.0 + x * (Ki[i] - 1.0));
    Z_total += Zi[i];
  }

  // Add the last component (with total mass fraction = 1 - z_total)
  f += (1.0 - Z_total) * (Ki[num_z] - 1.0) / (1.0 + x * (Ki[num_z] - 1.0));

  return f;
}

Real
PorousFlowFluidStateFlash::rachfordRiceDeriv(Real x,
                                             std::vector<Real> & Zi,
                                             std::vector<Real> & Ki) const
{
  const std::size_t num_Z = Zi.size();
  // Check that the sizes of the mass fractions and equilibrium constant vectors are correct
  if (Ki.size() != num_Z + 1)
    mooseError("The number of mass fractions or equilibrium components passed to rachfordRice is "
               "not correct");

  Real df = 0.0;
  Real Z_total = 0.0;

  for (std::size_t i = 0; i < num_Z; ++i)
  {
    df -= Zi[i] * (Ki[i] - 1.0) * (Ki[i] - 1.0) / (1.0 + x * (Ki[i] - 1.0)) /
          (1.0 + x * (Ki[i] - 1.0));
    Z_total += Zi[i];
  }

  // Add the last component (with total mass fraction = 1 - z_total)
  df -= (1.0 - Z_total) * (Ki[num_Z] - 1.0) * (Ki[num_Z] - 1.0) / (1.0 + x * (Ki[num_Z] - 1.0)) /
        (1.0 + x * (Ki[num_Z] - 1.0));

  return df;
}

Real
PorousFlowFluidStateFlash::vaporMassFraction(Real Z0, Real K0, Real K1) const
{
  return (Z0 * (K1 - K0) - (K1 - 1.0)) / ((K0 - 1.0) * (K1 - 1.0));
}

DualReal
PorousFlowFluidStateFlash::vaporMassFraction(const DualReal & Z0,
                                             const DualReal & K0,
                                             const DualReal & K1) const
{
  return (Z0 * (K1 - K0) - (K1 - 1.0)) / ((K0 - 1.0) * (K1 - 1.0));
}

Real
PorousFlowFluidStateFlash::vaporMassFraction(std::vector<Real> & Zi, std::vector<Real> & Ki) const
{
  // Check that the sizes of the mass fractions and equilibrium constant vectors are correct
  if (Ki.size() != Zi.size() + 1)
    mooseError("The number of mass fractions or equilibrium components passed to rachfordRice is "
               "not correct");
  Real v;

  // If there are only two components, an analytical solution is possible
  if (Ki.size() == 2)
    v = vaporMassFraction(Zi[0], Ki[0], Ki[1]);
  else
  {
    // More than two components - solve the Rachford-Rice equation using
    // Newton-Raphson method
    // Initial guess for vapor mass fraction
    Real v0 = 0.5;
    unsigned int iter = 0;

    while (std::abs(rachfordRice(v0, Zi, Ki)) > _nr_tol)
    {
      v0 = v0 - rachfordRice(v0, Zi, Ki) / rachfordRiceDeriv(v0, Zi, Ki);
      iter++;

      if (iter > _nr_max_its)
        break;
    }
    v = v0;
  }
  return v;
}
