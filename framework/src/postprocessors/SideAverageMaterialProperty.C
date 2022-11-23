//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideAverageMaterialProperty.h"

registerMooseObject("MooseApp", SideAverageMaterialProperty);
registerMooseObject("MooseApp", ADSideAverageMaterialProperty);

template <bool is_ad>
InputParameters
SideAverageMaterialPropertyTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralMaterialPropertyTempl<is_ad>::validParams();
  params.addClassDescription("Computes the average of a material property over a side set.");
  return params;
}

template <bool is_ad>
SideAverageMaterialPropertyTempl<is_ad>::SideAverageMaterialPropertyTempl(
    const InputParameters & parameters)
  : SideIntegralMaterialPropertyTempl<is_ad>(parameters), _area(0.0)
{
}

template <bool is_ad>
void
SideAverageMaterialPropertyTempl<is_ad>::initialize()
{
  SideIntegralMaterialPropertyTempl<is_ad>::initialize();
  _area = 0.0;
}

template <bool is_ad>
void
SideAverageMaterialPropertyTempl<is_ad>::execute()
{
  SideIntegralMaterialPropertyTempl<is_ad>::execute();

  _area += this->_current_side_volume;
}

template <bool is_ad>
Real
SideAverageMaterialPropertyTempl<is_ad>::getValue()
{
  return _integral_value / _area;
}

template <bool is_ad>
void
SideAverageMaterialPropertyTempl<is_ad>::finalize()
{
  SideIntegralMaterialPropertyTempl<is_ad>::gatherSum(_area);
  SideIntegralMaterialPropertyTempl<is_ad>::gatherSum(_integral_value);
}

template <bool is_ad>
void
SideAverageMaterialPropertyTempl<is_ad>::threadJoin(const UserObject & y)
{
  SideIntegralMaterialPropertyTempl<is_ad>::threadJoin(y);

  const SideAverageMaterialPropertyTempl<is_ad> & pps =
      static_cast<const SideAverageMaterialPropertyTempl<is_ad> &>(y);
  _area += pps._area;
}

template class SideAverageMaterialPropertyTempl<false>;
template class SideAverageMaterialPropertyTempl<true>;
