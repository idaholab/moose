//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAbruptSoftening.h"

#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", ADAbruptSoftening);

InputParameters
ADAbruptSoftening::validParams()
{
  InputParameters params = ADSmearedCrackSofteningBase::validParams();
  params.addClassDescription("Softening model with an abrupt stress release upon cracking. This "
                             "class relies on automatic differentiation and is intended to be used "
                             "with ADComputeSmearedCrackingStress.");
  params.addRangeCheckedParam<Real>(
      "residual_stress",
      0.0,
      "residual_stress <= 1 & residual_stress >= 0",
      "The fraction of the cracking stress allowed to be maintained following a crack.");
  return params;
}

ADAbruptSoftening::ADAbruptSoftening(const InputParameters & parameters)
  : ADSmearedCrackSofteningBase(parameters), _residual_stress(getParam<Real>("residual_stress"))
{
}

void
ADAbruptSoftening::computeCrackingRelease(ADReal & stress,
                                          ADReal & stiffness_ratio,
                                          const ADReal & /*strain*/,
                                          const ADReal & crack_initiation_strain,
                                          const ADReal & crack_max_strain,
                                          const ADReal & cracking_stress,
                                          const ADReal & youngs_modulus)
{
  if (_residual_stress == 0.0)
  {
    const Real tiny = 1.e-16;
    stiffness_ratio = tiny;
    stress = tiny * crack_initiation_strain * youngs_modulus;
  }
  else
  {
    stress = _residual_stress * cracking_stress;
    stiffness_ratio = stress / (crack_max_strain * youngs_modulus);
  }
}
