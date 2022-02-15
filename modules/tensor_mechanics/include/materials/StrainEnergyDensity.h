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
#include "MooseTypes.h"

/**
 * StrainEnergyDensity calculates the strain energy density.
 */
template <bool is_ad>
class StrainEnergyDensityTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  StrainEnergyDensityTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

protected:
  /// Base name of the material system
  const std::string _base_name;

  /// The strain energy density material property
  MaterialProperty<Real> & _strain_energy_density;
  const MaterialProperty<Real> & _strain_energy_density_old;

  ///{@ Current and old values of stress
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _stress;
  const MaterialProperty<RankTwoTensor> & _stress_old;
  ///@}

  /// Current value of mechanical strain which includes elastic and
  /// inelastic components of the strain
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _mechanical_strain;

  /// Current value of the strain increment for incremental models
  const GenericOptionalMaterialProperty<RankTwoTensor, is_ad> & _strain_increment;
};

typedef StrainEnergyDensityTempl<false> StrainEnergyDensity;
typedef StrainEnergyDensityTempl<true> ADStrainEnergyDensity;
