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

// Forward Declarations
template <ComputeStage>
class ADDynamicStressDivergenceTensors;

declareADValidParams(ADDynamicStressDivergenceTensors);

/**
 * ADDynamicStressDivergenceTensors is the automatic
 * differentiation version of DynamicStressDivergenceTensors.
 * This kernel derives from ADStressDivergenceTensors and
 * adds stress related Rayleigh and HHT time integration terms.
 */
template <ComputeStage compute_stage>
class ADDynamicStressDivergenceTensors : public ADStressDivergenceTensors<compute_stage>
{
public:
  ADDynamicStressDivergenceTensors(const InputParameters & parameters);

protected:
  ADReal computeQpResidual();

  const MaterialProperty<RankTwoTensor> & _stress_older;
  const MaterialProperty<RankTwoTensor> & _stress_old;

  // Rayleigh damping parameter _zeta and HHT time integration parameter _alpha
  const MaterialProperty<Real> & _zeta;
  const Real _alpha;
  const bool _static_initialization;

  usingStressDivergenceTensorsMembers;
};
