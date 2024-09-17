//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

#include <unordered_map>

class MaterialBase;

template <typename T, bool is_ad>
class ProjectedStatefulMaterialAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  ProjectedStatefulMaterialAuxTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeValue() override;

  /// ID of the subdomain currently being iterated over
  const SubdomainID & _current_subdomain_id;

  /// A sorted list of all material objects the processed property depends on on each subdomain
  std::unordered_map<SubdomainID, std::vector<MaterialBase *>> _required_materials;

  /// The property a component of which is being projected
  const GenericMaterialProperty<T, is_ad> & _prop;

  /// Property component (index into a serialized representation of the property)
  const unsigned int _component;
};

typedef ProjectedStatefulMaterialAuxTempl<Real, false> ProjectedStatefulMaterialRealAux;
typedef ProjectedStatefulMaterialAuxTempl<Real, true> ADProjectedStatefulMaterialRealAux;
typedef ProjectedStatefulMaterialAuxTempl<RealVectorValue, false>
    ProjectedStatefulMaterialRealVectorValueAux;
typedef ProjectedStatefulMaterialAuxTempl<RealVectorValue, true>
    ADProjectedStatefulMaterialRealVectorValueAux;
typedef ProjectedStatefulMaterialAuxTempl<RankTwoTensor, false>
    ProjectedStatefulMaterialRankTwoTensorAux;
typedef ProjectedStatefulMaterialAuxTempl<RankTwoTensor, true>
    ADProjectedStatefulMaterialRankTwoTensorAux;
typedef ProjectedStatefulMaterialAuxTempl<RankFourTensor, false>
    ProjectedStatefulMaterialRankFourTensorAux;
typedef ProjectedStatefulMaterialAuxTempl<RankFourTensor, true>
    ADProjectedStatefulMaterialRankFourTensorAux;

// Prevent implicit instantiation in other translation units where these classes are used
extern template class ProjectedStatefulMaterialAuxTempl<Real, false>;
extern template class ProjectedStatefulMaterialAuxTempl<Real, true>;
extern template class ProjectedStatefulMaterialAuxTempl<RealVectorValue, false>;
extern template class ProjectedStatefulMaterialAuxTempl<RealVectorValue, true>;
extern template class ProjectedStatefulMaterialAuxTempl<RankTwoTensor, false>;
extern template class ProjectedStatefulMaterialAuxTempl<RankTwoTensor, true>;
extern template class ProjectedStatefulMaterialAuxTempl<RankFourTensor, false>;
extern template class ProjectedStatefulMaterialAuxTempl<RankFourTensor, true>;
