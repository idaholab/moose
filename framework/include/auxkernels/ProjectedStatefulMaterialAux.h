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
#include "IndexableProperty.h"

#include <deque>

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

  const SubdomainID & _current_subdomain_id;

  std::map<SubdomainID, std::vector<MaterialBase *>> _required_materials;

  const GenericMaterialProperty<T, is_ad> & _prop;

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
