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

/**
 * Compute density depending on strains from deforming mesh.
 */
template <bool is_ad>
class StrainAdjustedDensityTempl : public Material
{
public:
  static InputParameters validParams();

  StrainAdjustedDensityTempl(const InputParameters & params);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const Moose::CoordinateSystemType _coord_system;
  const GenericVariableValue<is_ad> & _disp_r;

  const std::string _base_name;
  const GenericMaterialProperty<Real, is_ad> & _strain_free_density;

private:
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp;
  GenericMaterialProperty<Real, is_ad> & _density;
};

typedef StrainAdjustedDensityTempl<false> StrainAdjustedDensity;
typedef StrainAdjustedDensityTempl<true> ADStrainAdjustedDensity;
