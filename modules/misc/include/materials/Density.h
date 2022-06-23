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
 * Compute density, which may changed based on a deforming mesh.
 */
template <bool is_ad>
class DensityTempl : public Material
{
public:
  static InputParameters validParams();

  DensityTempl(const InputParameters & params);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const bool _is_coupled;
  const Moose::CoordinateSystemType _coord_system;
  const GenericVariableValue<is_ad> & _disp_r;

  const std::string _base_name;
  const Real _initial_density;

private:
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp;
  GenericMaterialProperty<Real, is_ad> & _density;
};

typedef DensityTempl<false> Density;
typedef DensityTempl<true> ADDensity;
