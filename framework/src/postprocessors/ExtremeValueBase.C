//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeFunctorValue.h"

#include "ElementPostprocessor.h"
#include "ElementVariablePostprocessor.h"
#include "NodalVariablePostprocessor.h"
#include "SideVariablePostprocessor.h"

#include <limits>

template <class T>
InputParameters
ExtremeValueBase<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addParam<MooseEnum>(
      "value_type",
      MooseEnum("max=0 min=1 max_abs=2", "max"),
      "Type of extreme value to return. 'max' "
      "returns the maximum value. 'min' returns "
      "the minimum value. 'max_abs' returns the maximum of the absolute value.");
  return params;
}

template <class T>
ExtremeValueBase<T>::ExtremeValueBase(const InputParameters & parameters)
  : T(parameters), _type(parameters.get<MooseEnum>("value_type").getEnum<ExtremeType>())
{
}

template <class T>
void
ExtremeValueBase<T>::initialize()
{
  if (_type == ExtremeType::MAX || _type == ExtremeType::MAX_ABS)
    _proxy_value =
        std::make_pair(-std::numeric_limits<Real>::max(), -std::numeric_limits<Real>::max());
  else if (_type == ExtremeType::MIN)
    _proxy_value =
        std::make_pair(std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max());
}

template <class T>
void
ExtremeValueBase<T>::computeExtremeValue()
{
  const auto pv = getProxyValuePair();

  if ((_type == ExtremeType::MAX && pv > _proxy_value) ||
      (_type == ExtremeType::MIN && pv < _proxy_value))
    _proxy_value = pv;
  else if (_type == ExtremeType::MAX_ABS && std::abs(pv.first) > _proxy_value.first)
    _proxy_value = std::make_pair(std::abs(pv.first), pv.second);
}

template <class T>
Real
ExtremeValueBase<T>::getValue() const
{
  return _proxy_value.second;
}

template <class T>
void
ExtremeValueBase<T>::finalize()
{
  if (_type == ExtremeType::MAX || _type == ExtremeType::MAX_ABS)
    this->gatherProxyValueMax(_proxy_value.first, _proxy_value.second);
  else if (_type == ExtremeType::MIN)
    this->gatherProxyValueMin(_proxy_value.first, _proxy_value.second);
}

template <class T>
void
ExtremeValueBase<T>::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const ExtremeValueBase<T> &>(y);

  if (((_type == ExtremeType::MAX || _type == ExtremeType::MAX_ABS) &&
       pps._proxy_value > _proxy_value) ||
      (_type == ExtremeType::MIN && pps._proxy_value < _proxy_value))
    _proxy_value = pps._proxy_value;
}

template class ExtremeValueBase<ElementPostprocessor>;
template class ExtremeValueBase<ElementVariablePostprocessor>;
template class ExtremeValueBase<NodalVariablePostprocessor>;
template class ExtremeValueBase<SideVariablePostprocessor>;
