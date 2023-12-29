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
#include "RankTwoTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * GBRelaxationStrainIncrement computes strain increment due to lattice relaxation at GB
 * Forest et. al. MSMSE 2015
 */
class GBRelaxationStrainIncrement : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  GBRelaxationStrainIncrement(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  const MaterialProperty<Real> & _prefactor;
  const MaterialProperty<RankTwoTensor> & _gb_normal_tensor;
  MaterialProperty<RankTwoTensor> & _strain_increment;
};
