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
#include "ADRankThreeTensorForward.h"
#include "ADRankFourTensorForward.h"

/**
 * Switches between materials in a multi-material system where the
 * interfaces are defined by multiple geometric cut userobjects.
 */
template <typename T, bool is_ad>
class XFEMCutSwitchingMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  XFEMCutSwitchingMaterialTempl(const InputParameters & parameters);

protected:
  // At the time of initializing this switching material, the initQpStatefulProperties methods of
  // the base materials are already called. So we only need to assign the current values at _qp to
  // the switching material property.
  virtual void initQpStatefulProperties() override { computeProperties(); }

  virtual void computeProperties() override;

  virtual void computeQpProperties() override { _prop[_qp] = (*_mapped_prop)[_qp]; }

private:
  /// The geometric cut userobject that provides the cut subdomain IDs
  const GeometricCutUserObject * _cut;

  ///{@ map keys and values
  const std::vector<CutSubdomainID> _keys;
  const std::vector<std::string> _vals;
  ///@}

  /// global material property base name
  const std::string _base_name;

  /// property name
  const std::string _prop_name;

  /// map of keys to material property
  std::unordered_map<unsigned int, const GenericMaterialProperty<T, is_ad> *> _prop_map;

  /// the global material property
  GenericMaterialProperty<T, is_ad> & _prop;

  /// shared pointer to XFEM
  std::shared_ptr<XFEM> _xfem;

  /// current mapped material property
  const GenericMaterialProperty<T, is_ad> * _mapped_prop;
};

typedef XFEMCutSwitchingMaterialTempl<Real, false> XFEMCutSwitchingMaterialReal;
typedef XFEMCutSwitchingMaterialTempl<RankTwoTensor, false> XFEMCutSwitchingMaterialRankTwoTensor;
typedef XFEMCutSwitchingMaterialTempl<RankThreeTensor, false>
    XFEMCutSwitchingMaterialRankThreeTensor;
typedef XFEMCutSwitchingMaterialTempl<RankFourTensor, false> XFEMCutSwitchingMaterialRankFourTensor;

typedef XFEMCutSwitchingMaterialTempl<Real, true> ADXFEMCutSwitchingMaterialReal;
typedef XFEMCutSwitchingMaterialTempl<RankTwoTensor, true> ADXFEMCutSwitchingMaterialRankTwoTensor;
typedef XFEMCutSwitchingMaterialTempl<RankThreeTensor, true>
    ADXFEMCutSwitchingMaterialRankThreeTensor;
typedef XFEMCutSwitchingMaterialTempl<RankFourTensor, true>
    ADXFEMCutSwitchingMaterialRankFourTensor;
