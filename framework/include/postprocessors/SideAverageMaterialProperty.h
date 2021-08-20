//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralMaterialProperty.h"

/**
 * Computes the average of a material property over a side set.
 */
template <typename T, bool is_ad>
class SideAverageMaterialPropertyTempl : public SideIntegralMaterialPropertyTempl<T, is_ad>
{
public:
  static InputParameters validParams();

  SideAverageMaterialPropertyTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Side set area
  Real _area;
};

typedef SideAverageMaterialPropertyTempl<Real, false> SideAverageMaterialRealProperty;
typedef SideAverageMaterialPropertyTempl<Real, true> ADSideAverageMaterialRealProperty;

typedef SideAverageMaterialPropertyTempl<RealVectorValue, false>
    SideAverageMaterialRealVectorValueProperty;
typedef SideAverageMaterialPropertyTempl<RealVectorValue, true>
    ADSideAverageMaterialRealVectorValueProperty;

typedef SideAverageMaterialPropertyTempl<RankTwoTensor, false>
    SideAverageMaterialRankTwoTensorProperty;
typedef SideAverageMaterialPropertyTempl<RankTwoTensor, true>
    ADSideAverageMaterialRankTwoTensorProperty;

typedef SideAverageMaterialPropertyTempl<RankThreeTensor, false>
    SideAverageMaterialRankThreeTensorProperty;
typedef SideAverageMaterialPropertyTempl<RankThreeTensor, true>
    ADSideAverageMaterialRankThreeTensorProperty;

typedef SideAverageMaterialPropertyTempl<RankFourTensor, false>
    SideAverageMaterialRankFourTensorProperty;
typedef SideAverageMaterialPropertyTempl<RankFourTensor, true>
    ADSideAverageMaterialRankFourTensorProperty;
