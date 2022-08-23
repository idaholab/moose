//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideDiffusiveFluxAverage.h"

registerMooseObject("MooseApp", SideDiffusiveFluxAverage);
registerMooseObject("MooseApp", ADSideDiffusiveFluxAverage);
registerMooseObjectRenamed("MooseApp",
                           SideFluxAverage,
                           "06/30/2021 24:00",
                           SideDiffusiveFluxAverage);
registerMooseObjectRenamed("MooseApp",
                           ADSideFluxAverage,
                           "06/30/2021 24:00",
                           ADSideDiffusiveFluxAverage);

template <bool is_ad>
InputParameters
SideDiffusiveFluxAverageTempl<is_ad>::validParams()
{
  InputParameters params = SideDiffusiveFluxIntegralTempl<is_ad, Real>::validParams();
  return params;
}

template <bool is_ad>
SideDiffusiveFluxAverageTempl<is_ad>::SideDiffusiveFluxAverageTempl(
    const InputParameters & parameters)
  : SideDiffusiveFluxIntegralTempl<is_ad, Real>(parameters), _volume(0)
{
}

template <bool is_ad>
void
SideDiffusiveFluxAverageTempl<is_ad>::initialize()
{
  SideDiffusiveFluxIntegralTempl<is_ad, Real>::initialize();
  _volume = 0;
}

template <bool is_ad>
void
SideDiffusiveFluxAverageTempl<is_ad>::execute()
{
  SideDiffusiveFluxIntegralTempl<is_ad, Real>::execute();
  _volume += this->_current_side_volume;
}

template <bool is_ad>
Real
SideDiffusiveFluxAverageTempl<is_ad>::getValue()
{
  return _integral_value / _volume;
}

template <bool is_ad>
void
SideDiffusiveFluxAverageTempl<is_ad>::finalize()
{
  SideDiffusiveFluxIntegralTempl<is_ad, Real>::gatherSum(_integral_value);
  SideDiffusiveFluxIntegralTempl<is_ad, Real>::gatherSum(_volume);
}

template <bool is_ad>
void
SideDiffusiveFluxAverageTempl<is_ad>::threadJoin(const UserObject & y)
{
  SideDiffusiveFluxIntegralTempl<is_ad, Real>::threadJoin(y);
  const SideDiffusiveFluxAverageTempl<is_ad> & pps =
      static_cast<const SideDiffusiveFluxAverageTempl<is_ad> &>(y);
  _volume += pps._volume;
}

template class SideDiffusiveFluxAverageTempl<false>;
template class SideDiffusiveFluxAverageTempl<true>;
