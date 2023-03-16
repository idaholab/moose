//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEMaterial.h"

/**
 * Computes material properties relevant to simulation of fluid flow in a porous medium
 */
class PINSFEMaterial : public INSFEMaterial
{
public:
  static InputParameters validParams();

  PINSFEMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  Real _alpha;
  Real _beta;
  Real _pm_htc_const;
  Real _pm_aw_const;

  MaterialProperty<Real> & _pm_htc;
  MaterialProperty<Real> & _pm_aw;
};
