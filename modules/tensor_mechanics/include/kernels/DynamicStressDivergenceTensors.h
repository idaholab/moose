//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StressDivergenceTensors.h"

/**
 * DynamicStressDivergenceTensors derives from StressDivergenceTensors and adds stress related
 * Rayleigh and HHT time integration terms.
 */
class DynamicStressDivergenceTensors : public StressDivergenceTensors
{
public:
  static InputParameters validParams();

  DynamicStressDivergenceTensors(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  ///{@ The old and older states of the stress tensor that the divergence operator operates on
  const MaterialProperty<RankTwoTensor> & _stress_older;
  const MaterialProperty<RankTwoTensor> & _stress_old;
  ///@}

  // Rayleigh damping parameter _zeta and HHT time integration parameter _alpha
  const MaterialProperty<Real> & _zeta;
  const Real _alpha;
  const bool _static_initialization;
};
