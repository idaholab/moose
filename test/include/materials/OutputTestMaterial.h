//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Material.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"
#include "SymmetricRankTwoTensorForward.h"
#include "SymmetricRankFourTensorForward.h"

template <bool is_ad>
class OutputTestMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  OutputTestMaterialTempl(const InputParameters & parameters);

  // Used for testing if hidden compiler warning shows up
  virtual void computeProperties() { Material::computeProperties(); }

  /**
   * Class destructor
   */
  virtual ~OutputTestMaterialTempl();

protected:
  virtual void computeQpProperties();

  GenericMaterialProperty<Real, is_ad> & _real_property;
  GenericMaterialProperty<RealVectorValue, is_ad> & _vector_property;
  GenericMaterialProperty<RealTensorValue, is_ad> & _tensor_property;
  GenericMaterialProperty<RankTwoTensor, is_ad> & _ranktwotensor_property;
  GenericMaterialProperty<RankFourTensor, is_ad> & _rankfourtensor_property;
  GenericMaterialProperty<SymmetricRankTwoTensor, is_ad> & _symmetricranktwotensor_property;
  GenericMaterialProperty<SymmetricRankFourTensor, is_ad> & _symmetricrankfourtensor_property;
  GenericMaterialProperty<std::vector<Real>, is_ad> * _stdvector_property;
  GenericReal<is_ad> _factor;
  const GenericVariableValue<is_ad> & _variable;
};

typedef OutputTestMaterialTempl<false> OutputTestMaterial;
typedef OutputTestMaterialTempl<true> ADOutputTestMaterial;
