//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateFlash.h"

template <>
InputParameters
validParams<PorousFlowFluidStateFlash>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Compositional flash calculations for use in fluid state classes");
  return params;
}

PorousFlowFluidStateFlash::PorousFlowFluidStateFlash(const InputParameters & parameters)
  : GeneralUserObject(parameters), _nr_max_its(42), _nr_tol(1.0e-12)
{
}

void
PorousFlowFluidStateFlash::phaseState(Real zi,
                                      Real Xi,
                                      Real Yi,
                                      FluidStatePhaseEnum & phase_state) const
{
  if (zi <= Xi)
  {
    // In this case, there is not enough component i to form a gas phase,
    // so only a liquid phase is present
    phase_state = FluidStatePhaseEnum::LIQUID;
  }
  else if (zi > Xi && zi < Yi)
  {
    // Two phases are present
    phase_state = FluidStatePhaseEnum::TWOPHASE;
  }
  else // (zi >= Yi)
  {
    // In this case, there is not enough water to form a liquid
    // phase, so only a gas phase is present
    phase_state = FluidStatePhaseEnum::GAS;
  }
}

Real
PorousFlowFluidStateFlash::rachfordRice(Real x,
                                        std::vector<Real> & zi,
                                        std::vector<Real> & Ki) const
{
  const std::size_t num_z = zi.size();
  // Check that the sizes of the mass fractions and equilibrium constant vectors are correct
  if (Ki.size() != num_z + 1)
    mooseError("The number of mass fractions or equilibrium components passed to rachfordRice is "
               "not correct");

  Real f = 0.0;
  Real z_total = 0.0;

  for (std::size_t i = 0; i < num_z; ++i)
  {
    f += zi[i] * (Ki[i] - 1.0) / (1.0 + x * (Ki[i] - 1.0));
    z_total += zi[i];
  }

  // Add the last component (with total mass fraction = 1 - z_total)
  f += (1.0 - z_total) * (Ki[num_z] - 1.0) / (1.0 + x * (Ki[num_z] - 1.0));

  return f;
}

Real
PorousFlowFluidStateFlash::rachfordRiceDeriv(Real x,
                                             std::vector<Real> & zi,
                                             std::vector<Real> & Ki) const
{
  const std::size_t num_z = zi.size();
  // Check that the sizes of the mass fractions and equilibrium constant vectors are correct
  if (Ki.size() != num_z + 1)
    mooseError("The number of mass fractions or equilibrium components passed to rachfordRice is "
               "not correct");

  Real df = 0.0;
  Real z_total = 0.0;

  for (std::size_t i = 0; i < num_z; ++i)
  {
    df -= zi[i] * (Ki[i] - 1.0) * (Ki[i] - 1.0) / (1.0 + x * (Ki[i] - 1.0)) /
          (1.0 + x * (Ki[i] - 1.0));
    z_total += zi[i];
  }

  // Add the last component (with total mass fraction = 1 - z_total)
  df -= (1.0 - z_total) * (Ki[num_z] - 1.0) * (Ki[num_z] - 1.0) / (1.0 + x * (Ki[num_z] - 1.0)) /
        (1.0 + x * (Ki[num_z] - 1.0));

  return df;
}

Real
PorousFlowFluidStateFlash::vaporMassFraction(Real z0, Real K0, Real K1) const
{
  return (z0 * (K1 - K0) - (K1 - 1.0)) / ((K0 - 1.0) * (K1 - 1.0));
}

Real
PorousFlowFluidStateFlash::vaporMassFraction(std::vector<Real> & zi, std::vector<Real> & Ki) const
{
  // Check that the sizes of the mass fractions and equilibrium constant vectors are correct
  if (Ki.size() != zi.size() + 1)
    mooseError("The number of mass fractions or equilibrium components passed to rachfordRice is "
               "not correct");
  Real v;

  // If there are only two components, an analytical solution is possible
  if (Ki.size() == 2)
    v = vaporMassFraction(zi[0], Ki[0], Ki[1]);
  else
  {
    // More than two components - solve the Rachford-Rice equation using
    // Newton-Raphson method
    // Initial guess for vapor mass fraction
    Real v0 = 0.5;
    unsigned int iter = 0;

    while (std::abs(rachfordRice(v0, zi, Ki)) > _nr_tol)
    {
      v0 = v0 - rachfordRice(v0, zi, Ki) / rachfordRiceDeriv(v0, zi, Ki);
      iter++;

      if (iter > _nr_max_its)
        break;
    }
    v = v0;
  }
  return v;
}
