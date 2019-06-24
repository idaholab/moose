//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParametricMaterialBasePD.h"

class ComputeSmallStrainMaterialBaseOSPD;

template <>
InputParameters validParams<ComputeSmallStrainMaterialBaseOSPD>();

/**
 * Base material class for ordinary state based peridynamic solid mechanics models
 */
class ComputeSmallStrainMaterialBaseOSPD : public ParametricMaterialBasePD
{
public:
  ComputeSmallStrainMaterialBaseOSPD(const InputParameters & parameters);

protected:
  virtual void computeBondForce() override;

  ///@{ Material properties to store
  MaterialProperty<Real> & _bond_force_i_j;
  MaterialProperty<Real> & _bond_dfdU_i_j;
  MaterialProperty<Real> & _bond_dfdT_i_j;
  MaterialProperty<Real> & _bond_dfdE_i_j;
  ///@}

  ///@{ Model parameters
  Real _a;
  Real _b;
  std::vector<Real> _d;
  ///@}
};
