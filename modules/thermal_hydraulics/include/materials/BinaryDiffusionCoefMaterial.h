//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class VaporMixtureFluidProperties;
class SinglePhaseFluidProperties;

/**
 * Computes the Stefan-Maxwell binary diffusion coefficient.
 */
class BinaryDiffusionCoefMaterial : public Material
{
public:
  static InputParameters validParams();

  BinaryDiffusionCoefMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const VaporMixtureFluidProperties & _fp_mix;
  const SinglePhaseFluidProperties & _fp1;
  const SinglePhaseFluidProperties & _fp2;

  const Real _M1;
  const Real _M2;

  const Real _collision_diam1;
  const Real _collision_diam2;
  const Real _collision_diam;

  const ADMaterialProperty<Real> & _p;
  const ADMaterialProperty<Real> & _T;

  ADMaterialProperty<Real> & _diff_coef;
};
