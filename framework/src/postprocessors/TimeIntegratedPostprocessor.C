//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeIntegratedPostprocessor.h"
#include "CrankNicolson.h"
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
  params.addClassDescription("Integrate a Postprocessor value over time.");
  params.addParam<PostprocessorName>("value", "The name of the postprocessor");
  MooseEnum schemes("implicit-euler trapezoidal-rule", "trapezoidal-rule");
  params.addParam<MooseEnum>(
      "time_integration_scheme", schemes, "Time integration scheme to use for the postprocessors");
  return params;
}

TimeIntegratedPostprocessor::TimeIntegratedPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value(0),
    _value_old(getPostprocessorValueOldByName(name())),
    _pps_value(getPostprocessorValue("value")),
    _pps_value_old(getPostprocessorValueOld("value")),
    _time_integration_scheme(getParam<MooseEnum>("time_integration_scheme"))
{
  if (!dynamic_cast<TransientBase *>(_app.getExecutioner()))
    mooseError(
        "Time integration of postprocessor has only been implemented with a transient executioner");

  // Only check if the user did not select it manually
  if (!isParamSetByUser("time_integration_scheme"))
  {
    const auto time_integrators =
        dynamic_cast<TransientBase *>(_app.getExecutioner())->getTimeIntegrators();
    for (const auto & ti : time_integrators)
      if (_time_integration_scheme == TimeIntegration::TrapezoidalRule &&
          !dynamic_cast<CrankNicolson *>(ti))
        mooseInfo(
            "The time integration in this postprocessor uses the trapezoidal rule method. The "
            "equation time integration scheme does not use the trapezoidal rule. If the "
            "postprocessor uses variable values, even indirectly, we would recommend you "
            "code the same time integration method for the time-integrated postprocessor. "
            "Specify the 'time_integration_scheme' parameter to silence this warning.");
  }
}

void
TimeIntegratedPostprocessor::initialize()
{
}

void
TimeIntegratedPostprocessor::execute()
{
  if (_time_integration_scheme == TimeIntegration::ImplicitEuler)
    _value = _value_old + _pps_value * _dt;
  // 2nd order trapezoidal rule is a better default for other integrators
  else
    _value = _value_old + 0.5 * (_pps_value + _pps_value_old) * _dt;
}

Real
TimeIntegratedPostprocessor::getValue() const
{
  return _value;
}
