//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"
#include "ComponentUtils.h"

/**
 * Computes the integral of a material property over a side set.
 */
template <typename T, bool is_ad>
class SideIntegralMaterialPropertyTempl : public SideIntegralPostprocessor, public ComponentUtils<T>
{
public:
  static InputParameters validParams();

  SideIntegralMaterialPropertyTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  const GenericMaterialProperty<T, is_ad> & _prop;
};

typedef SideIntegralMaterialPropertyTempl<Real, false> SideIntegralMaterialRealProperty;
typedef SideIntegralMaterialPropertyTempl<Real, true> ADSideIntegralMaterialRealProperty;

typedef SideIntegralMaterialPropertyTempl<RealVectorValue, false>
    SideIntegralMaterialRealVectorValueProperty;
typedef SideIntegralMaterialPropertyTempl<RealVectorValue, true>
    ADSideIntegralMaterialRealVectorValueProperty;

typedef SideIntegralMaterialPropertyTempl<RankTwoTensor, false>
    SideIntegralMaterialRankTwoTensorProperty;
typedef SideIntegralMaterialPropertyTempl<RankTwoTensor, true>
    ADSideIntegralMaterialRankTwoTensorProperty;

typedef SideIntegralMaterialPropertyTempl<RankThreeTensor, false>
    SideIntegralMaterialRankThreeTensorProperty;
typedef SideIntegralMaterialPropertyTempl<RankThreeTensor, true>
    ADSideIntegralMaterialRankThreeTensorProperty;

typedef SideIntegralMaterialPropertyTempl<RankFourTensor, false>
    SideIntegralMaterialRankFourTensorProperty;
typedef SideIntegralMaterialPropertyTempl<RankFourTensor, true>
    ADSideIntegralMaterialRankFourTensorProperty;
