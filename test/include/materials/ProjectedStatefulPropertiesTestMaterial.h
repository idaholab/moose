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
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

/**
 * Test the projected old state capability in ProjectedStatefulMaterialStorageAction
 */
template <bool is_ad>
class ProjectedStatefulPropertiesTestMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  ProjectedStatefulPropertiesTestMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  /// Real
  GenericMaterialProperty<Real, is_ad> & _prop_real;
  const MaterialProperty<Real> & _prop_real_old;
  /// RealVectorValue
  GenericMaterialProperty<RealVectorValue, is_ad> & _prop_realvectorvalue;
  const MaterialProperty<RealVectorValue> & _prop_realvectorvalue_old;
  /// RankTwoTensor
  GenericMaterialProperty<RankTwoTensor, is_ad> & _prop_ranktwotensor;
  const MaterialProperty<RankTwoTensor> & _prop_ranktwotensor_old;
  /// RankFourTensor
  GenericMaterialProperty<RankFourTensor, is_ad> & _prop_rankfourtensor;
  const MaterialProperty<RankFourTensor> & _prop_rankfourtensor_old;

  /// diagnostic outout
  MaterialProperty<Real> & _diff_norm;
};

typedef ProjectedStatefulPropertiesTestMaterialTempl<false> ProjectedStatefulPropertiesTestMaterial;
typedef ProjectedStatefulPropertiesTestMaterialTempl<true>
    ADProjectedStatefulPropertiesTestMaterial;
