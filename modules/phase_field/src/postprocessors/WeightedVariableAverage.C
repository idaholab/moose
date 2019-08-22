//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedVariableAverage.h"

registerMooseObject("PhaseFieldApp", WeightedVariableAverage);

InputParameters
WeightedVariableAverage::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription(
      "Average a variable value using a weight mask given by a material property.");
  params.addCoupledVar("v", "Variable to perform weighted average on.");
  params.addRequiredParam<MaterialPropertyName>("weight",
                                                "Weight material property for averaging.");
  return params;
}

WeightedVariableAverage::WeightedVariableAverage(const InputParameters & parameters)
  : ElementPostprocessor(parameters), _v(coupledValue("v")), _w(getMaterialProperty<Real>("weight"))
{
}

void
WeightedVariableAverage::initialize()
{
  _var_integral = 0.0;
  _weight_integral = 0.0;
}

void
WeightedVariableAverage::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    auto j = _JxW[qp] * _coord[qp];
    _var_integral += j * _v[qp] * _w[qp];
    _weight_integral += j * _w[qp];
  }
}

void
WeightedVariableAverage::finalize()
{
  gatherSum(_var_integral);
  gatherSum(_weight_integral);
}

PostprocessorValue
WeightedVariableAverage::getValue()
{
  if (_weight_integral == 0.0)
    return 0.0;

  return _var_integral / _weight_integral;
}

void
WeightedVariableAverage::threadJoin(const UserObject & y)
{
  const WeightedVariableAverage & pp = static_cast<const WeightedVariableAverage &>(y);
  _var_integral += pp._var_integral;
  _weight_integral += pp._weight_integral;
}
