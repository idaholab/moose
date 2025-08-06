//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FluxBasedStrainIncrement.h"

/**
 * MyFluxBasedStrainIncrementVolcorrLocalConcExplicit computes strain increment based on flux (vacancy)
 * Forest et. al. MSMSE 2015
 */
class VolumeStrainIncrement : public FluxBasedStrainIncrement
{
public:
  static InputParameters validParams();

  VolumeStrainIncrement(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  virtual void computeIdentityTensor();

  const MaterialProperty<Real> & _Lambda_prefactor_J;
  const MaterialProperty<Real> & _Lambda_prefactor_P;
  const VariableValue & _derivative_conc;
  const MaterialProperty<Real> & _source;

  RankTwoTensor _Identity_tensor;
};
