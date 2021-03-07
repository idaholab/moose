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

/**
 * Porosity calculation from the inelastic strain
 */
template <bool is_ad>
class PorosityFromStrainTempl : public Material
{
public:
  static InputParameters validParams();

  PorosityFromStrainTempl(const InputParameters & parameters);

protected:
  enum class NegativeBehavior
  {
    ZERO,
    INITIAL_CONDITION,
    EXCEPTION
  };

  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  /// Porosity value
  GenericMaterialProperty<Real, is_ad> & _porosity;

  /// Old porosity value
  const MaterialProperty<Real> & _porosity_old;

  /// Initial porosity
  const Real _initial_porosity;

  /// Inelastic strain material property
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _inelastic_strain;

  /// Old value of the inelastic strain
  const MaterialProperty<RankTwoTensor> & _inelastic_strain_old;

  /// Enum for negative porosity handling
  const NegativeBehavior _negative_behavior;
};

typedef PorosityFromStrainTempl<false> PorosityFromStrain;
typedef PorosityFromStrainTempl<true> ADPorosityFromStrain;
