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
  switch (_type)
  {
    case ExtremeType::MAX:
      _proxy_value =
          std::make_pair(-std::numeric_limits<Real>::max(), -std::numeric_limits<Real>::max());
      break;

    case ExtremeType::MIN:
      _proxy_value =
          std::make_pair(std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max());
      break;
  }
}

template <class T>
void
ExtremeValueBase<T>::computeExtremeValue()
{
  const auto pv = getProxyValuePair();
  switch (_type)
  {
    case ExtremeType::MAX:
      if (pv > _proxy_value)
        _proxy_value = pv;
      break;

    case ExtremeType::MIN:
      if (pv < _proxy_value)
        _proxy_value = pv;
      break;
  }
}

template <class T>
Real
ExtremeValueBase<T>::getValue()
{
  return _proxy_value.second;
}

template <class T>
void
ExtremeValueBase<T>::finalize()
{
  switch (_type)
  {
    case ExtremeType::MAX:
      if (_use_proxy)
        this->gatherProxyValueMax(_proxy_value.first, _proxy_value.second);
      else
      {
        this->gatherMax(_proxy_value.first);
        _proxy_value.second = _proxy_value.first;
      }
      break;
    case ExtremeType::MIN:
      if (_use_proxy)
        this->gatherProxyValueMin(_proxy_value.first, _proxy_value.second);
      else
      {
        this->gatherMin(_proxy_value.first);
        _proxy_value.second = _proxy_value.first;
      }
      break;
  }
}

template <class T>
void
ExtremeValueBase<T>::threadJoin(const UserObject & y)
{
  const ExtremeValueBase<T> & pps = static_cast<const ExtremeValueBase<T> &>(y);

  switch (_type)
  {
    case ExtremeType::MAX:
      if (pps._proxy_value > _proxy_value)
        _proxy_value = pps._proxy_value;
      break;
    case ExtremeType::MIN:
      if (pps._proxy_value < _proxy_value)
        _proxy_value = pps._proxy_value;
      break;
  }
}

template class ExtremeValueBase<ElementPostprocessor>;
template class ExtremeValueBase<ElementVariablePostprocessor>;
template class ExtremeValueBase<NodalVariablePostprocessor>;
template class ExtremeValueBase<SideVariablePostprocessor>;
