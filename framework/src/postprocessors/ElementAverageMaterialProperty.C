//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAverageMaterialProperty.h"

registerMooseObject("MooseApp", ElementAverageMaterialProperty);
registerMooseObject("MooseApp", ADElementAverageMaterialProperty);

template <bool is_ad>
InputParameters
ElementAverageMaterialPropertyTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralMaterialPropertyTempl<is_ad>::validParams();
  params.addClassDescription("Computes the average of a material property over a volume.");
  return params;
}

template <bool is_ad>
ElementAverageMaterialPropertyTempl<is_ad>::ElementAverageMaterialPropertyTempl(
    const InputParameters & parameters)
  : ElementIntegralMaterialPropertyTempl<is_ad>(parameters), _volume(0.0)
{
}

template <bool is_ad>
void
ElementAverageMaterialPropertyTempl<is_ad>::initialize()
{
  ElementIntegralMaterialPropertyTempl<is_ad>::initialize();

  _volume = 0.0;
}

template <bool is_ad>
void
ElementAverageMaterialPropertyTempl<is_ad>::execute()
{
  ElementIntegralMaterialPropertyTempl<is_ad>::execute();

  _volume += this->_current_elem_volume;
}

template <bool is_ad>
Real
ElementAverageMaterialPropertyTempl<is_ad>::getValue()
{
  return _integral_value / _volume;
}

template <bool is_ad>
void
ElementAverageMaterialPropertyTempl<is_ad>::finalize()
{
  ElementIntegralMaterialPropertyTempl<is_ad>::gatherSum(_volume);
  ElementIntegralMaterialPropertyTempl<is_ad>::gatherSum(_integral_value);
}

template <bool is_ad>
void
ElementAverageMaterialPropertyTempl<is_ad>::threadJoin(const UserObject & y)
{
  ElementIntegralMaterialPropertyTempl<is_ad>::threadJoin(y);

  const ElementAverageMaterialPropertyTempl<is_ad> & pps =
      static_cast<const ElementAverageMaterialPropertyTempl<is_ad> &>(y);
  _volume += pps._volume;
}

template class ElementAverageMaterialPropertyTempl<false>;
template class ElementAverageMaterialPropertyTempl<true>;
