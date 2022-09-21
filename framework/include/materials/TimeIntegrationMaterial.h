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

#include <vector>
#include <tuple>

/**
 * Time integrate a rate material property
 */
template <typename T, bool is_ad>
class TimeIntegrationMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  TimeIntegrationMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /**
   * Vector of tuples containing
   * 1. Input properties to be time integrated
   * 2. Current value of integrated property
   * 3. Old state of the integrated property
   */
  std::vector<std::tuple<const GenericMaterialProperty<T, is_ad> *,
                         GenericMaterialProperty<T, is_ad> *,
                         const MaterialProperty<T> *>>
      _prop_tuples;
};

typedef TimeIntegrationMaterialTempl<Real, false> TimeIntegrationMaterial;
typedef TimeIntegrationMaterialTempl<Real, true> ADTimeIntegrationMaterial;
typedef TimeIntegrationMaterialTempl<RankTwoTensor, false> TimeIntegrationRankTwoMaterial;
typedef TimeIntegrationMaterialTempl<RankTwoTensor, true> ADTimeIntegrationRankTwoMaterial;
