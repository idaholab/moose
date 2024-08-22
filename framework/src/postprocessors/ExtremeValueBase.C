//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.addParam<MooseEnum>("value_type",
                             MooseEnum("max=0 min=1", "max"),
                             "Type of extreme value to return. 'max' "
                             "returns the maximum value. 'min' returns "
                             "the minimum value.");
  return params;
}

template <class T>
ExtremeValueBase<T>::ExtremeValueBase(const InputParameters & parameters)
  : T(parameters),
    _type(parameters.get<MooseEnum>("value_type").getEnum<ExtremeType>()),
    _use_proxy(true)
{
}

template <class T>
void
ExtremeValueBase<T>::initialize()
{
  if (_type == ExtremeType::MAX)
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
  if (_type == ExtremeType::MAX)
  {
    if (_use_proxy)
      this->gatherProxyValueMax(_proxy_value.first, _proxy_value.second);
    else
      this->gatherMax(_proxy_value.second);
  }
  else if (_type == ExtremeType::MIN)
  {
    if (_use_proxy)
      this->gatherProxyValueMin(_proxy_value.first, _proxy_value.second);
    else
      this->gatherMin(_proxy_value.second);
  }
}

template <class T>
void
ExtremeValueBase<T>::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const ExtremeValueBase<T> &>(y);

  if ((_type == ExtremeType::MAX && pps._proxy_value > _proxy_value) ||
      (_type == ExtremeType::MIN && pps._proxy_value < _proxy_value))
    _proxy_value = pps._proxy_value;
}

template class ExtremeValueBase<ElementPostprocessor>;
template class ExtremeValueBase<ElementVariablePostprocessor>;
template class ExtremeValueBase<NodalVariablePostprocessor>;
template class ExtremeValueBase<SideVariablePostprocessor>;
