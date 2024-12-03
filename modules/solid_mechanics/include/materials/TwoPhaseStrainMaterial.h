//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

class TwoPhaseStrainMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  TwoPhaseStrainMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  // switching function
  const MaterialProperty<Real> & _h_eta;

  // phase A material properties
  std::string _base_A;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_A;

  // phase B material properties
  std::string _base_B;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_B;

  // global material properties
  MaterialProperty<RankTwoTensor> & _strain;
};
