//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeIntegratedPostprocessor.h"
#include "ExplicitMidpoint.h"
#include "ImplicitMidpoint.h"
#include "TransientBase.h"

registerMooseObject("MooseApp", TimeIntegratedPostprocessor);
registerMooseObjectRenamed("MooseApp",
                           TotalVariableValue,
                           "04/01/2022 00:00",
                           TimeIntegratedPostprocessor);

InputParameters
TimeIntegratedPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Integrate a Postprocessor value over time using trapezoidal rule.");
  params.addParam<PostprocessorName>("value", "The name of the postprocessor");
  return params;
}

TimeIntegratedPostprocessor::TimeIntegratedPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value(0),
    _value_old(getPostprocessorValueOldByName(name())),
    _pps_value(getPostprocessorValue("value")),
    _pps_value_old(getPostprocessorValueOld("value"))
{
  if (!dynamic_cast<TransientBase *>(_app.getExecutioner()))
    mooseError(
        "Time integration of postprocessor has only been implemented with a transient executioner");
  const auto time_integrators =
      dynamic_cast<TransientBase *>(_app.getExecutioner())->getTimeIntegrators();
  for (const auto & ti : time_integrators)
    if (!dynamic_cast<ExplicitMidpoint *>(ti) && !dynamic_cast<ImplicitMidpoint *>(ti))
      mooseDoOnce(mooseWarning("The time integration coded in this postprocessor uses the midpoint method. The variable time integration, notably with time integrator '" + ti->name()+"' is not using the mid point method. If the postprocessor uses variable values, even indirectly, we would recommend you code the same time integration method for the time-integrated postprocessor."));
}

void
TimeIntegratedPostprocessor::initialize()
{
}

void
TimeIntegratedPostprocessor::execute()
{
  _value = _value_old + 0.5 * (_pps_value + _pps_value_old) * _dt;
}

Real
TimeIntegratedPostprocessor::getValue() const
{
  return _value;
}
