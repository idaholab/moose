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
 * FluxBasedStrainIncrement computes strain increment based on flux (vacancy)
 * Forest et. al. MSMSE 2015
 */
class FluxBasedStrainIncrement : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  FluxBasedStrainIncrement(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  virtual void computeFluxGradTensor();

  const VariableGradient * const _grad_jx;
  bool _has_yflux;
  bool _has_zflux;
  const VariableGradient * const _grad_jy;
  const VariableGradient * const _grad_jz;

  const VariableValue & _gb;

  MaterialProperty<RankTwoTensor> & _strain_increment;

  RankTwoTensor _flux_grad_tensor;
};
