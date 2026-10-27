//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMultiComponentTestFluidState.h"
#include "PorousFlowCapillaryPressure.h"

registerMooseObject("PorousFlowTestApp", PorousFlowMultiComponentTestFluidState);

InputParameters
PorousFlowMultiComponentTestFluidState::validParams()
{
  InputParameters params = PorousFlowFluidStateMultiComponentBase::validParams();
  params.addRangeCheckedParam<unsigned int>(
      "number_fluid_components",
      3,
      "number_fluid_components>=2",
      "Number of fluid components. There are number_fluid_components - 1 total mass fractions");
  params.addClassDescription(
      "Test-only two-phase fluid state whose properties are smooth, non-physical functions of "
      "pressure, temperature and every total mass fraction");
  return params;
}

PorousFlowMultiComponentTestFluidState::PorousFlowMultiComponentTestFluidState(
    const InputParameters & parameters)
  : PorousFlowFluidStateMultiComponentBase(parameters)
{
  _num_phases = 2;
  _num_components = getParam<unsigned int>("number_fluid_components");
  _gas_fluid_component = 1;

  _empty_fsp = FluidStateProperties(_num_components);
}

std::string
PorousFlowMultiComponentTestFluidState::fluidStateName() const
{
  return "multicomponent-test";
}

void
PorousFlowMultiComponentTestFluidState::thermophysicalProperties(
    Real pressure,
    Real temperature,
    const std::vector<Real> & composition,
    unsigned int qp,
    std::vector<FluidStateProperties> & fsp) const
{
  ADReal p = pressure;
  Moose::derivInsert(p.derivatives(), getPressureIndex(), 1.0);
  ADReal T = temperature;
  Moose::derivInsert(T.derivatives(), getTemperatureIndex(), 1.0);

  std::vector<ADReal> composition_ad(composition.size());
  for (const auto m : index_range(composition))
  {
    composition_ad[m] = composition[m];
    Moose::derivInsert(composition_ad[m].derivatives(), getZIndex(m), 1.0);
  }

  thermophysicalProperties(p, T, composition_ad, qp, fsp);
}

void
PorousFlowMultiComponentTestFluidState::thermophysicalProperties(
    const ADReal & pressure,
    const ADReal & temperature,
    const std::vector<ADReal> & composition,
    unsigned int qp,
    std::vector<FluidStateProperties> & fsp) const
{
  mooseAssert(composition.size() + 1 == _num_components,
              "There must be one fewer total mass fraction than fluid components");

  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Pressure and temperature scaled to O(1) for the test region (MPa and 300 K) so that each
  // term below contributes a derivative of comparable size
  const ADReal pn = pressure / 1.0e6;
  const ADReal Tn = temperature / 300.0;

  // Weighted sum of the total mass fractions, so that the saturation and mass fractions depend on
  // each z_m with a different weight
  ADReal t = 0.0;
  for (const auto m : index_range(composition))
    t += (m + 1.0) * composition[m];

  // The saturation and the phase mass fractions are LINEAR in pressure, temperature and every
  // total mass fraction. PorousFlowFluidState neglects the second derivatives of saturation in the
  // Jacobian of the capillary pressure gradient, and PorousFlowDispersiveFlux neglects the second
  // derivatives of the mass fractions in the Jacobian of their gradients, so with linear
  // dependence those Jacobians are exact. The offsets keep both phases present and every mass
  // fraction in (0, 1) for pressures of 1-2 MPa, a temperature of 300 K and the total mass
  // fractions used in the tests (z0 in [0.3, 0.4], z1 in [0.2, 0.3])
  gas.saturation = 0.5 + 0.5 * (t - 0.85) + 0.01 * (pn - 1.5) - 0.1 * (Tn - 1.0);
  liquid.saturation = 1.0 - gas.saturation;

  // The mass fraction of component m + 1 follows the total mass fraction z_m, and component 0
  // makes each phase sum to one. The liquid and gas coefficients differ so that the phases
  // partition the components differently
  const std::vector<Real> c{0.8, 1.2};
  const std::vector<Real> e{0.05, -0.05};
  for (const auto ph : make_range(_num_phases))
  {
    ADReal sum = 0.0;
    for (const auto m : index_range(composition))
    {
      fsp[ph].mass_fraction[m + 1] =
          c[ph] * composition[m] + e[ph] * t + 0.2 * e[ph] * (m + 1.0) * (pn - Tn);
      sum += fsp[ph].mass_fraction[m + 1];
    }
    fsp[ph].mass_fraction[0] = 1.0 - sum;
  }

  liquid.density = 1000.0 * (1.0 + 0.01 * pn + 0.1 * liquid.mass_fraction[0]);
  gas.density = 100.0 * pn / Tn * (1.0 + 0.2 * gas.mass_fraction[_num_components - 1]);
  liquid.viscosity = 1.0e-3 * (1.0 + 0.1 * liquid.mass_fraction[0]);
  gas.viscosity = 2.0e-5 * (1.0 + 0.1 * Tn + 0.1 * gas.mass_fraction[1]);

  liquid.enthalpy = 4.0e3 * temperature * (1.0 + 0.1 * liquid.mass_fraction[1]);
  gas.enthalpy = 1.0e3 * temperature * (1.0 + 0.1 * gas.mass_fraction[0]);
  for (const auto ph : make_range(_num_phases))
    fsp[ph].internal_energy = fsp[ph].enthalpy - pressure / fsp[ph].density;

  gas.pressure = pressure;
  liquid.pressure = pressure - _pc.capillaryPressure(liquid.saturation, qp);
}

Real
PorousFlowMultiComponentTestFluidState::totalMassFraction(
    Real, Real, Real, Real, unsigned int) const
{
  mooseError(name(), ": totalMassFraction is not implemented");
}
