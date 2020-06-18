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
 * ConstantAnisotropicMobility provides a simple RealTensorValue type
 * MaterialProperty that can be used as a mobility in a phase field simulation.
 */
template <bool is_ad>
class ConstantAnisotropicMobilityTempl : public Material
{
public:
  static InputParameters validParams();

  ConstantAnisotropicMobilityTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// raw tensor values as passed in from the input file
  const std::vector<Real> & _mobility_values;

  /// Name of the mobility tensor material property
  const MaterialPropertyName & _mobility_name;
  GenericMaterialProperty<RealTensorValue, is_ad> & _mobility;
};

typedef ConstantAnisotropicMobilityTempl<false> ConstantAnisotropicMobility;
typedef ConstantAnisotropicMobilityTempl<true> ADConstantAnisotropicMobility;
