//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PowerLawSoftening.h"

#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", PowerLawSoftening);

InputParameters
PowerLawSoftening::validParams()
{
  InputParameters params = SmearedCrackSofteningBase::validParams();
  params.addClassDescription("Softening model with an abrupt stress release upon cracking. This "
                             "class is intended to be used with ComputeSmearedCrackingStress.");
  params.addRequiredRangeCheckedParam<Real>(
      "stiffness_reduction",
      "stiffness_reduction <= 1 & stiffness_reduction >= 0",
      "Factor multiplied by the current stiffness each time a new crack forms");
  return params;
}

PowerLawSoftening::PowerLawSoftening(const InputParameters & parameters)
  : SmearedCrackSofteningBase(parameters),
    _stiffness_reduction(getParam<Real>("stiffness_reduction"))
{
}

void
PowerLawSoftening::computeCrackingRelease(Real & stress,
                                          Real & stiffness_ratio,
                                          const Real strain,
                                          const Real /*crack_initiation_strain*/,
                                          const Real /*crack_max_strain*/,
                                          const Real cracking_stress,
                                          const Real youngs_modulus)
{
  if (stress > cracking_stress)
  {
    // This is equivalent to k = k_0 * (stiffness_reduction)^n, where n is the number of cracks
    stiffness_ratio *= _stiffness_reduction;
    stress = stiffness_ratio * youngs_modulus * strain;
  }
}
