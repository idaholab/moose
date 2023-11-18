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
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

/**
 * Reconstitute a materal property from the old and older states of projected AuxVariables. Use
 * though the ProjectedStatefulMaterialStorageAction.
 */
template <typename T>
class InterpolatedStatefulMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  InterpolatedStatefulMaterialTempl(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  /// Old projected state
  const std::vector<const VariableValue *> _old_state;

  /// Older projected state
  const std::vector<const VariableValue *> _older_state;

  /// total number of components
  const std::size_t _size;

  /// emitted property name
  const MaterialPropertyName _prop_name;

  /// Old interpolated property
  MaterialProperty<T> & _prop_old;

  /// Older interpolated properties
  MaterialProperty<T> & _prop_older;
};

typedef InterpolatedStatefulMaterialTempl<Real> InterpolatedStatefulMaterialReal;
typedef InterpolatedStatefulMaterialTempl<RealVectorValue>
    InterpolatedStatefulMaterialRealVectorValue;
typedef InterpolatedStatefulMaterialTempl<RankTwoTensor> InterpolatedStatefulMaterialRankTwoTensor;
typedef InterpolatedStatefulMaterialTempl<RankFourTensor>
    InterpolatedStatefulMaterialRankFourTensor;
