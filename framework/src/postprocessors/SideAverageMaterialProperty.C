//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideAverageMaterialProperty.h"

registerMooseObject("MooseApp", SideAverageMaterialRealProperty);
registerMooseObject("MooseApp", ADSideAverageMaterialRealProperty);
registerMooseObject("MooseApp", SideAverageMaterialRealVectorValueProperty);
registerMooseObject("MooseApp", ADSideAverageMaterialRealVectorValueProperty);
registerMooseObject("MooseApp", SideAverageMaterialRankTwoTensorProperty);
registerMooseObject("MooseApp", ADSideAverageMaterialRankTwoTensorProperty);
registerMooseObject("MooseApp", SideAverageMaterialRankThreeTensorProperty);
registerMooseObject("MooseApp", ADSideAverageMaterialRankThreeTensorProperty);
registerMooseObject("MooseApp", SideAverageMaterialRankFourTensorProperty);
registerMooseObject("MooseApp", ADSideAverageMaterialRankFourTensorProperty);

template <typename T, bool is_ad>
InputParameters
SideAverageMaterialPropertyTempl<T, is_ad>::validParams()
{
  InputParameters params = SideIntegralMaterialPropertyTempl<T, is_ad>::validParams();
  params.addClassDescription("Computes the average of a material property over a side set.");
  return params;
}

template <typename T, bool is_ad>
SideAverageMaterialPropertyTempl<T, is_ad>::SideAverageMaterialPropertyTempl(
    const InputParameters & parameters)
  : SideIntegralMaterialPropertyTempl<T, is_ad>(parameters), _area(0.0)
{
}

template <typename T, bool is_ad>
void
SideAverageMaterialPropertyTempl<T, is_ad>::initialize()
{
  SideIntegralMaterialPropertyTempl<T, is_ad>::initialize();

  _area = 0.0;
}

template <typename T, bool is_ad>
void
SideAverageMaterialPropertyTempl<T, is_ad>::execute()
{
  SideIntegralMaterialPropertyTempl<T, is_ad>::execute();

  _area += this->_current_side_volume;
}

template <typename T, bool is_ad>
Real
SideAverageMaterialPropertyTempl<T, is_ad>::getValue()
{
  const Real integral = SideIntegralMaterialPropertyTempl<T, is_ad>::getValue();

  SideIntegralMaterialPropertyTempl<T, is_ad>::gatherSum(_area);

  return integral / _area;
}

template <typename T, bool is_ad>
void
SideAverageMaterialPropertyTempl<T, is_ad>::threadJoin(const UserObject & y)
{
  SideIntegralMaterialPropertyTempl<T, is_ad>::threadJoin(y);

  const SideAverageMaterialPropertyTempl<T, is_ad> & pps =
      static_cast<const SideAverageMaterialPropertyTempl<T, is_ad> &>(y);
  _area += pps._area;
}

template class SideAverageMaterialPropertyTempl<Real, false>;
template class SideAverageMaterialPropertyTempl<Real, true>;
template class SideAverageMaterialPropertyTempl<RealVectorValue, false>;
template class SideAverageMaterialPropertyTempl<RealVectorValue, true>;
template class SideAverageMaterialPropertyTempl<RankTwoTensor, false>;
template class SideAverageMaterialPropertyTempl<RankTwoTensor, true>;
template class SideAverageMaterialPropertyTempl<RankThreeTensor, false>;
template class SideAverageMaterialPropertyTempl<RankThreeTensor, true>;
template class SideAverageMaterialPropertyTempl<RankFourTensor, false>;
template class SideAverageMaterialPropertyTempl<RankFourTensor, true>;
