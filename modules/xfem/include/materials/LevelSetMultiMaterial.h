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
#include "XFEM.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"

/**
 * switches between materials in a multi-material system where the
 * interfaces are defined by multiple level set functions.
 */
template <typename T, bool is_ad>
class LevelSetMultiMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  LevelSetMultiMaterialTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override {}
  virtual void computeProperties() override;
  virtual void computeQpProperties() override;

  /// hashes a string of encodings, e.g. '++---' into an unsigned integer
  virtual unsigned int encodeLevelSetKey(const std::string key);

  /// locate a physical node in the element
  virtual const Node * pickOnePhysicalNode();

  /// levelset variable names
  const std::vector<VariableName> _level_set_var_names;

  /// number of levelset variables
  const unsigned int _nvars;

  /// levelset variable numbers
  std::vector<unsigned int> _level_set_var_numbers;

  /// the systems and solution vectors
  std::vector<const System *> _systems;
  std::vector<const NumericVector<Number> *> _solutions;

  ///{@ map keys and values
  const std::vector<std::string> _keys;
  const std::vector<std::string> _vals;
  ///@}

  /// global material property base name
  const std::string _base_name;

  /// property name
  const std::string _prop_name;

  /// map of keys to material property
  std::map<unsigned int, const GenericMaterialProperty<T, is_ad> *> _prop_map;

  /// the global material property
  GenericMaterialProperty<T, is_ad> & _prop;

  /// shared pointer to XFEM
  std::shared_ptr<XFEM> _xfem;

  /// map of keys to base name
  std::map<unsigned int, const std::string> _base_name_map;

  /// current mapped material property
  const GenericMaterialProperty<T, is_ad> * _mapped_prop;
};

typedef LevelSetMultiMaterialTempl<Real, false> LevelSetMultiRealMaterial;
typedef LevelSetMultiMaterialTempl<RankTwoTensor, false> LevelSetMultiRankTwoTensorMaterial;
typedef LevelSetMultiMaterialTempl<RankFourTensor, false> LevelSetMultiRankFourTensorMaterial;

typedef LevelSetMultiMaterialTempl<Real, true> ADLevelSetMultiRealMaterial;
typedef LevelSetMultiMaterialTempl<RankTwoTensor, true> ADLevelSetMultiRankTwoTensorMaterial;
typedef LevelSetMultiMaterialTempl<RankFourTensor, true> ADLevelSetMultiRankFourTensorMaterial;
