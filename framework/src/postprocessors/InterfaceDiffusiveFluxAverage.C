//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceDiffusiveFluxAverage.h"

registerMooseObject("MooseApp", InterfaceDiffusiveFluxAverage);
registerMooseObject("MooseApp", ADInterfaceDiffusiveFluxAverage);

template <bool is_ad>
InputParameters
InterfaceDiffusiveFluxAverageTempl<is_ad>::validParams()
{
  InputParameters params = InterfaceDiffusiveFluxIntegralTempl<is_ad>::validParams();
  return params;
}

template <bool is_ad>
InterfaceDiffusiveFluxAverageTempl<is_ad>::InterfaceDiffusiveFluxAverageTempl(
    const InputParameters & parameters)
  : InterfaceDiffusiveFluxIntegralTempl<is_ad>(parameters), _volume(0)
{
}

template <bool is_ad>
void
InterfaceDiffusiveFluxAverageTempl<is_ad>::initialize()
{
  InterfaceDiffusiveFluxIntegralTempl<is_ad>::initialize();
  _volume = 0;
}

template <bool is_ad>
void
InterfaceDiffusiveFluxAverageTempl<is_ad>::execute()
{
  InterfaceDiffusiveFluxIntegralTempl<is_ad>::execute();
  _volume += this->_current_side_volume;
}

template <bool is_ad>
Real
InterfaceDiffusiveFluxAverageTempl<is_ad>::getValue()
{
  return _integral_value / _volume;
}

template <bool is_ad>
void
InterfaceDiffusiveFluxAverageTempl<is_ad>::finalize()
{
  this->gatherSum(_volume);
  this->gatherSum(_integral_value);
}

template <bool is_ad>
void
InterfaceDiffusiveFluxAverageTempl<is_ad>::threadJoin(const UserObject & y)
{
  InterfaceDiffusiveFluxIntegralTempl<is_ad>::threadJoin(y);
  const InterfaceDiffusiveFluxAverageTempl<is_ad> & pps =
      static_cast<const InterfaceDiffusiveFluxAverageTempl<is_ad> &>(y);
  _volume += pps._volume;
}

template class InterfaceDiffusiveFluxAverageTempl<false>;
template class InterfaceDiffusiveFluxAverageTempl<true>;
