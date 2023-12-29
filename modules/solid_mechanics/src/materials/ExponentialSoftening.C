//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExponentialSoftening.h"

#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", ExponentialSoftening);

InputParameters
ExponentialSoftening::validParams()
{
  InputParameters params = SmearedCrackSofteningBase::validParams();
  params.addClassDescription(
      "Softening model with an exponential softening response upon cracking. This "
      "class is intended to be used with ComputeSmearedCrackingStress.");
  params.addRangeCheckedParam<Real>(
      "residual_stress",
      0.0,
      "residual_stress <= 1 & residual_stress >= 0",
      "The fraction of the cracking stress allowed to be maintained following a crack.");
  params.addRangeCheckedParam<Real>(
      "alpha",
      -1.0,
      "alpha <= 0",
      "Initial slope of the exponential softening curve at crack initiation. "
      "If not specified, it is equal to the negative of the Young's modulus.");
  params.addRangeCheckedParam<Real>(
      "beta",
      1.0,
      "beta >= 0",
      "Multiplier applied to alpha to control the exponential softening "
      "behavior.");
  return params;
}

ExponentialSoftening::ExponentialSoftening(const InputParameters & parameters)
  : SmearedCrackSofteningBase(parameters),
    _residual_stress(getParam<Real>("residual_stress")),
    _alpha(getParam<Real>("alpha")),
    _alpha_set_by_user(parameters.isParamSetByUser("alpha")),
    _beta(getParam<Real>("beta"))
{
}

void
ExponentialSoftening::computeCrackingRelease(Real & stress,
                                             Real & stiffness_ratio,
                                             const Real /*strain*/,
                                             const Real crack_initiation_strain,
                                             const Real crack_max_strain,
                                             const Real cracking_stress,
                                             const Real youngs_modulus)
{
  mooseAssert(crack_max_strain >= crack_initiation_strain,
              "crack_max_strain must be >= crack_initiation_strain");

  Real alpha = 0.0;
  if (_alpha_set_by_user)
    alpha = _alpha;
  else
    alpha = -youngs_modulus;

  // Compute stress that follows exponental curve
  stress = cracking_stress *
           (_residual_stress +
            (1.0 - _residual_stress) * std::exp(alpha * _beta / cracking_stress *
                                                (crack_max_strain - crack_initiation_strain)));
  // Compute ratio of current stiffness to original stiffness
  stiffness_ratio = stress * crack_initiation_strain / (crack_max_strain * cracking_stress);
}
