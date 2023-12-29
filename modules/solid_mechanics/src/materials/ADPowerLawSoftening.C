//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPowerLawSoftening.h"

#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", ADPowerLawSoftening);

InputParameters
ADPowerLawSoftening::validParams()
{
  InputParameters params = ADSmearedCrackSofteningBase::validParams();
  params.addClassDescription("Softening model with an abrupt stress release upon cracking. This "
                             "class is intended to be used with ADComputeSmearedCrackingStress and "
                             "relies on automatic differentiation.");
  params.addRequiredRangeCheckedParam<Real>(
      "stiffness_reduction",
      "stiffness_reduction <= 1 & stiffness_reduction >= 0",
      "Factor multiplied by the current stiffness each time a new crack forms");
  return params;
}

ADPowerLawSoftening::ADPowerLawSoftening(const InputParameters & parameters)
  : ADSmearedCrackSofteningBase(parameters),
    _stiffness_reduction(getParam<Real>("stiffness_reduction"))
{
}

void
ADPowerLawSoftening::computeCrackingRelease(ADReal & stress,
                                            ADReal & stiffness_ratio,
                                            const ADReal & strain,
                                            const ADReal & /*crack_initiation_strain*/,
                                            const ADReal & /*crack_max_strain*/,
                                            const ADReal & cracking_stress,
                                            const ADReal & youngs_modulus)
{
  if (stress > cracking_stress)
  {
    // This is equivalent to k = k_0 * (stiffness_reduction)^n, where n is the number of cracks
    stiffness_ratio *= _stiffness_reduction;
    stress = stiffness_ratio * youngs_modulus * strain;
  }
}
