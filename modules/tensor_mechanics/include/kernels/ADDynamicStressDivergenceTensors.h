//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADStressDivergenceTensors.h"

/**
 * ADDynamicStressDivergenceTensors is the automatic
 * differentiation version of DynamicStressDivergenceTensors.
 * This kernel derives from ADStressDivergenceTensors and
 * adds stress related Rayleigh and HHT time integration terms.
 */
class ADDynamicStressDivergenceTensors : public ADStressDivergenceTensors
{
public:
  static InputParameters validParams();

  ADDynamicStressDivergenceTensors(const InputParameters & parameters);

protected:
  ADReal computeQpResidual();

  ///{@ The old and older states of the stress tensor that the divergence operator operates on
  const MaterialProperty<RankTwoTensor> & _stress_older;
  const MaterialProperty<RankTwoTensor> & _stress_old;
  ///@}

  // Rayleigh damping parameter _zeta and HHT time integration parameter _alpha
  const MaterialProperty<Real> & _zeta;
  const Real _alpha;
  const bool _static_initialization;
};
