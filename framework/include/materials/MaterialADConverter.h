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
 * This material converts regular material properties to AD properties and AD properties to regular
 * properties
 */
template <typename T>
class MaterialADConverterTempl : public Material
{
public:
  static InputParameters validParams();

  MaterialADConverterTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  void initQpStatefulProperties() override;

  std::size_t _num_reg_props_to_convert;
  std::size_t _num_ad_props_to_convert;

  std::vector<MaterialProperty<T> *> _reg_props_out;
  std::vector<const MaterialProperty<T> *> _reg_props_in;
  std::vector<ADMaterialProperty<T> *> _ad_props_out;
  std::vector<const ADMaterialProperty<T> *> _ad_props_in;

  const bool _intra_convert;
};

typedef MaterialADConverterTempl<Real> MaterialADConverter;
typedef MaterialADConverterTempl<RankFourTensor> RankFourTensorMaterialADConverter;
typedef MaterialADConverterTempl<RankTwoTensor> RankTwoTensorMaterialADConverter;
