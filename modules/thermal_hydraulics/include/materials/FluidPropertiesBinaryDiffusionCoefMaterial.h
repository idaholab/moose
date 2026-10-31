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

/**
 * Computes the binary diffusion coefficient of the primary and secondary gases of a binary gas
 * mixture from its fluid properties
 */
class FluidPropertiesBinaryDiffusionCoefMaterial : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesBinaryDiffusionCoefMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Gas mixture fluid properties
  const VaporMixtureFluidProperties & _fp_mix;

  /// Pressure
  const ADMaterialProperty<Real> & _p;
  /// Temperature
  const ADMaterialProperty<Real> & _T;

  /// Binary diffusion coefficient
  ADMaterialProperty<Real> & _diff_coef;
};
