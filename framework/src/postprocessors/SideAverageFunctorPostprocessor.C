//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideAverageFunctorPostprocessor.h"

registerMooseObject("MooseApp", SideAverageFunctorPostprocessor);
registerMooseObject("MooseApp", ADSideAverageFunctorPostprocessor);

template <bool is_ad>
InputParameters
SideAverageFunctorPostprocessorTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralFunctorPostprocessorTempl<is_ad>::validParams();
  params.addClassDescription("Computes the average of a functor over a side set.");
  return params;
}

template <bool is_ad>
SideAverageFunctorPostprocessorTempl<is_ad>::SideAverageFunctorPostprocessorTempl(
    const InputParameters & parameters)
  : SideIntegralFunctorPostprocessorTempl<is_ad>(parameters), _area(0.0)
{
}

template <bool is_ad>
void
SideAverageFunctorPostprocessorTempl<is_ad>::initialize()
{
  SideIntegralFunctorPostprocessorTempl<is_ad>::initialize();
  _area = 0.0;
}

template <bool is_ad>
void
SideAverageFunctorPostprocessorTempl<is_ad>::execute()
{
  SideIntegralFunctorPostprocessorTempl<is_ad>::execute();

  _area += this->_current_side_volume;
}

template <bool is_ad>
Real
SideAverageFunctorPostprocessorTempl<is_ad>::getValue() const
{
  return _integral_value / _area;
}

template <bool is_ad>
void
SideAverageFunctorPostprocessorTempl<is_ad>::finalize()
{
  SideIntegralFunctorPostprocessorTempl<is_ad>::gatherSum(_area);
  SideIntegralFunctorPostprocessorTempl<is_ad>::gatherSum(_integral_value);
}

template <bool is_ad>
void
SideAverageFunctorPostprocessorTempl<is_ad>::threadJoin(const UserObject & y)
{
  SideIntegralFunctorPostprocessorTempl<is_ad>::threadJoin(y);

  const auto & pps = static_cast<const SideAverageFunctorPostprocessorTempl<is_ad> &>(y);
  _area += pps._area;
}

template class SideAverageFunctorPostprocessorTempl<false>;
template class SideAverageFunctorPostprocessorTempl<true>;
