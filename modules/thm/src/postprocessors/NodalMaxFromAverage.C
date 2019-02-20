#include "NodalMaxFromAverage.h"
#include <algorithm>
#include <limits>

registerMooseObject("THMApp", NodalMaxFromAverage);

template <>
InputParameters
validParams<NodalMaxFromAverage>()
{
  InputParameters params = validParams<NodalVariablePostprocessor>();
  params.addRequiredParam<std::string>("average_name_pps",
                                       "name of the postprocessor computing the mean value");

  return params;
}

NodalMaxFromAverage::NodalMaxFromAverage(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters),
    _value(-std::numeric_limits<Real>::max()),
    _average_name(getParam<std::string>("average_name_pps")),
    _average(getPostprocessorValueByName(_average_name))
{
}

void
NodalMaxFromAverage::initialize()
{
  _value = -std::numeric_limits<Real>::max();
}

void
NodalMaxFromAverage::execute()
{
  _value = std::max(_value, std::abs(_u[_qp] - _average));
}

Real
NodalMaxFromAverage::getValue()
{
  gatherMax(_value);
  return _value;
}

void
NodalMaxFromAverage::threadJoin(const UserObject & y)
{
  const NodalMaxFromAverage & pps = static_cast<const NodalMaxFromAverage &>(y);
  _value = std::max(_value, pps._value);
}
