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

/**
 * Base material class for ordinary state based peridynamic solid mechanics models
 */
class ComputeSmallStrainMaterialBaseOSPD : public ParametricMaterialBasePD
{
public:
  static InputParameters validParams();

  ComputeSmallStrainMaterialBaseOSPD(const InputParameters & parameters);

protected:
  virtual void computeBondForce() override;

  ///@{ Material properties to store
  MaterialProperty<Real> & _bond_nonlocal_force;
  MaterialProperty<Real> & _bond_nonlocal_dfdU;
  MaterialProperty<Real> & _bond_nonlocal_dfdT;
  MaterialProperty<Real> & _bond_nonlocal_dfdE;
  ///@}

  ///@{ Model parameters
  Real _a;
  Real _b;
  std::vector<Real> _d;
  ///@}
};
