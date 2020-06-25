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
 * StrainEnergyDensity calculates the strain energy density.
 */
class StrainEnergyDensity : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  StrainEnergyDensity(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;
  virtual void initialSetup() override;
  virtual void computeQpProperties() override;

protected:
  /// Base name of the material system
  const std::string _base_name;

  /// Whether the material model is a total or incremental model
  bool _incremental;

  /// The strain energy density material property
  MaterialProperty<Real> & _strain_energy_density;
  const MaterialProperty<Real> & _strain_energy_density_old;

  ///{@ Current and old values of stress
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _stress_old;
  ///@}

  /// Current value of mechanical strain which includes elastic and
  /// inelastic components of the strain
  const MaterialProperty<RankTwoTensor> & _mechanical_strain;

  /// Current value of the strain increment for incremental models
  const MaterialProperty<RankTwoTensor> * const _strain_increment;
};
