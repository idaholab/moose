//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideFluxAverage.h"

registerMooseObject("MooseApp", SideFluxAverage);
registerMooseObject("MooseApp", ADSideFluxAverage);

template <bool is_ad>
InputParameters
SideFluxAverageTempl<is_ad>::validParams()
{
  InputParameters params = SideFluxIntegralTempl<is_ad>::validParams();
  return params;
}

template <bool is_ad>
SideFluxAverageTempl<is_ad>::SideFluxAverageTempl(const InputParameters & parameters)
  : SideFluxIntegralTempl<is_ad>(parameters), _volume(0)
{
}

template <bool is_ad>
void
SideFluxAverageTempl<is_ad>::initialize()
{
  SideFluxIntegralTempl<is_ad>::initialize();
  _volume = 0;
}

template <bool is_ad>
void
SideFluxAverageTempl<is_ad>::execute()
{
  SideFluxIntegralTempl<is_ad>::execute();
  _volume += this->_current_side_volume;
}

template <bool is_ad>
Real
SideFluxAverageTempl<is_ad>::getValue()
{
  Real integral = SideFluxIntegralTempl<is_ad>::getValue();

  this->gatherSum(_volume);

  return integral / _volume;
}

template <bool is_ad>
void
SideFluxAverageTempl<is_ad>::threadJoin(const UserObject & y)
{
  SideFluxIntegralTempl<is_ad>::threadJoin(y);
  const SideFluxAverageTempl<is_ad> & pps = static_cast<const SideFluxAverageTempl<is_ad> &>(y);
  _volume += pps._volume;
}

template class SideFluxAverageTempl<false>;
template class SideFluxAverageTempl<true>;
