//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageVariableChange.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", AverageVariableChange);

InputParameters
AverageVariableChange::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();

  params.addClassDescription("Computes the volume-weighted L1 or L2 norm of the change of a "
                             "variable over a time step or between nonlinear iterations.");

  MooseEnum change_over("time_step nonlinear_iteration");
  params.addRequiredParam<MooseEnum>(
      "change_over", change_over, "Interval over which to compute the change");

  MooseEnum norm("L1 L2");
  params.addRequiredParam<MooseEnum>("norm", norm, "Type of norm to compute");

  return params;
}

AverageVariableChange::AverageVariableChange(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _change_over(getParam<MooseEnum>("change_over")),
    _u_change_old(_change_over == "time_step" ? coupledValueOld("variable")
                                              : coupledValuePreviousNL("variable")),
    _norm(getParam<MooseEnum>("norm")),
    _norm_exponent(_norm == "L1" ? 1 : 2),
    _volume(0)
{
}

void
AverageVariableChange::initialize()
{
  ElementIntegralVariablePostprocessor::initialize();
  _volume = 0;
}

void
AverageVariableChange::execute()
{
  ElementIntegralVariablePostprocessor::execute();
  _volume += _current_elem_volume;
}

void
AverageVariableChange::threadJoin(const UserObject & y)
{
  ElementIntegralVariablePostprocessor::threadJoin(y);
  const auto & pps = static_cast<const AverageVariableChange &>(y);
  _volume += pps._volume;
}

void
AverageVariableChange::finalize()
{
  gatherSum(_volume);
  gatherSum(_integral_value);
}

Real
AverageVariableChange::getValue() const
{
  mooseAssert(!MooseUtils::absoluteFuzzyEqual(_volume, 0.0), "Volume must be nonzero.");
  return std::pow(_integral_value / _volume, 1.0 / _norm_exponent);
}

Real
AverageVariableChange::computeQpIntegral()
{
  return std::pow(std::abs(_u[_qp] - _u_change_old[_qp]), _norm_exponent);
}
