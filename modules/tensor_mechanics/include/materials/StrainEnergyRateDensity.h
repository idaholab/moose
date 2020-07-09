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

/**
 * StrainEnergyRateDensity calculates the strain energy rate density.
 */
class StrainEnergyRateDensity : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  StrainEnergyRateDensity(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;
  virtual void initialSetup() override;
  virtual void computeQpProperties() override;

private:
  /// Base name of the material system
  const std::string _base_name;

  /// The strain energy density material property
  MaterialProperty<Real> & _strain_energy_rate_density;

  /// Current and old values of stress
  const MaterialProperty<RankTwoTensor> & _stress;

  /// Current value of the strain increment for incremental models
  const MaterialProperty<RankTwoTensor> & _strain_rate;

  /// Exponent on the effective stress
  const Real _n_exponent;
};
