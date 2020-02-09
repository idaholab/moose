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
#include "MooseTypes.h"

#include "metaphysicl/numberarray.h"
#include "metaphysicl/dualnumber.h"

/**
 * ADMaterials compute ADMaterialProperties.
 */
class ADMaterial : public Material
{
public:
  static InputParameters validParams();

  ADMaterial(const InputParameters & parameters);

  /**
   * declare the ad property named "prop_name"
   */
  template <typename T>
  ADMaterialProperty<T> & declareADProperty(const std::string & prop_name);
};

template <typename T>
ADMaterialProperty<T> &
ADMaterial::declareADProperty(const std::string & prop_name)
{
  _fe_problem.usingADMatProps(true);
  registerPropName(prop_name, false, Material::CURRENT, /*declared_ad=*/true);
  return _material_data->declareADProperty<T>(prop_name);
}
