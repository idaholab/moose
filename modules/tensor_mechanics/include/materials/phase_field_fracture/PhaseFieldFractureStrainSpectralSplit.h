//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhaseFieldFractureSpectralSplitBase.h"
#include "GuaranteeConsumer.h"

/**
 * The phase-field fracture model with a strain-based spectral split.
 */
class PhaseFieldFractureStrainSpectralSplit : public PhaseFieldFractureSpectralSplitBase,
                                              public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  PhaseFieldFractureStrainSpectralSplit(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void updateJacobianMultForDamage(RankFourTensor & jacobian_mult) override;

protected:
  virtual void computeDamagedStress(RankTwoTensor & stress_new) override;

  virtual void computeElasticEnergy() override;

  MaterialProperty<RankTwoTensor> & _strain_pos;

  MaterialProperty<RankFourTensor> & _P_pos;
};
