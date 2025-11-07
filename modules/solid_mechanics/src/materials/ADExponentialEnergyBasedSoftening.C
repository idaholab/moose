//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADExponentialEnergyBasedSoftening.h"

#include "MooseMesh.h"

registerMooseObject("SolidMechanicsApp", ADExponentialEnergyBasedSoftening);

InputParameters
ADExponentialEnergyBasedSoftening::validParams()
{
  InputParameters params = ADSmearedCrackSofteningBase::validParams();
  params.addClassDescription(
      "Softening model with an exponential softening response upon cracking. This "
      "class is intended to be used with ADComputeSmearedCrackingStress and relies on automatic "
      "differentiation.");
  params.addRangeCheckedParam<Real>(
      "residual_stress",
      0.0,
      "residual_stress <= 1 & residual_stress >= 0",
      "The fraction of the cracking stress allowed to be maintained following a crack.");
  params.addRequiredRangeCheckedParam<Real>(
      "fracture_toughness",
      "fracture_toughness > 0",
      "Fracture toughness used to calculate the softening slope. ");
  return params;
}

ADExponentialEnergyBasedSoftening::ADExponentialEnergyBasedSoftening(const InputParameters & parameters)
  : ADSmearedCrackSofteningBase(parameters),
    _residual_stress(getParam<Real>("residual_stress")),
    _fracture_toughness(getParam<Real>("fracture_toughness"))
{
}

void
ADExponentialEnergyBasedSoftening::computeCrackingRelease(ADReal & stress,
                                               ADReal & stiffness_ratio,
                                               const ADReal & /*strain*/,
                                               const ADReal & crack_initiation_strain,
                                               const ADReal & crack_max_strain,
                                               const ADReal & cracking_stress,
                                               const ADReal & youngs_modulus, 
                                               const ADReal & poissons_ratio)
{
  mooseAssert(crack_max_strain >= crack_initiation_strain,
              "crack_max_strain must be >= crack_initiation_strain");

  unsigned int dim = _current_elem->dim();
  
  // Get estimate of element size
  ADReal ele_len = 0.0;
  if (dim == 3) {
    ele_len = std::cbrt(_current_elem->volume());
  } else {
    ele_len = std::sqrt(_current_elem->volume());
  }

  // Calculate initial slope of exponential curve
  const ADReal energy_release_rate = (_fracture_toughness * _fracture_toughness) * (1 - poissons_ratio * poissons_ratio) / youngs_modulus;
  const ADReal frac_stress_sqr = cracking_stress * cracking_stress;
  const ADReal l_max = 2 * energy_release_rate * youngs_modulus / frac_stress_sqr;

  // Check against maximum allowed element size - avoid the divide by zero by capping at a large slope
  ADReal initial_slope = -1e5*youngs_modulus;
  if (ele_len < l_max) // TODO: need to log if this isn't true
    initial_slope = -frac_stress_sqr / (energy_release_rate / ele_len - frac_stress_sqr / (2*youngs_modulus));

  // Compute stress that follows exponental curve
  stress = cracking_stress *
           (_residual_stress + (1.0 - _residual_stress)
             * std::exp(initial_slope / cracking_stress *
                       (crack_max_strain - crack_initiation_strain)));
  // Compute ratio of current stiffness to original stiffness
  stiffness_ratio = stress * crack_initiation_strain / (crack_max_strain * cracking_stress);
}
