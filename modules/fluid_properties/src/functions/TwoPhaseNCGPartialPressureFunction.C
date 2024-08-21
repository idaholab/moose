//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseNCGPartialPressureFunction.h"
#include "TwoPhaseNCGPartialPressureFluidProperties.h"
#include "MooseUtils.h"

registerMooseObject("FluidPropertiesApp", TwoPhaseNCGPartialPressureFunction);

const std::map<std::string, unsigned int> TwoPhaseNCGPartialPressureFunction::_n_expected_args{
    {"p_sat", 1}, {"x_sat_ncg_from_p_T", 2}};

InputParameters
TwoPhaseNCGPartialPressureFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Computes a property from a TwoPhaseNCGPartialPressureFluidProperties object.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The TwoPhaseNCGPartialPressureFluidProperties object");

  MooseEnum property_call("p_sat x_sat_ncg_from_p_T");
  params.addRequiredParam<MooseEnum>("property_call", property_call, "Which function to call");

  params.addParam<FunctionName>("arg1", 0, "The first argument for the property call, if any");
  params.addParam<FunctionName>("arg2", 0, "The second argument for the property call, if any");

  return params;
}

TwoPhaseNCGPartialPressureFunction::TwoPhaseNCGPartialPressureFunction(
    const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _property_call(getParam<MooseEnum>("property_call")),
    _arg1_fn(getFunction("arg1")),
    _arg2_fn(getFunction("arg2"))
{
  // Check that the provided arguments matches the expected number
  if (_n_expected_args.find(_property_call) != _n_expected_args.end())
  {
    bool args_are_valid = true;
    const unsigned int n_arg_params = 2;
    const auto n_expected_args = _n_expected_args.at(_property_call);
    std::vector<std::string> expected_args, provided_args;
    for (unsigned int i = 0; i < n_arg_params; i++)
    {
      const std::string arg_param = "arg" + std::to_string(i + 1);
      const std::string arg_str = "'" + arg_param + "'";

      const bool arg_is_expected = i + 1 <= n_expected_args;
      if (arg_is_expected)
        expected_args.push_back(arg_str);

      if (isParamSetByUser(arg_param))
      {
        provided_args.push_back(arg_str);
        if (!arg_is_expected)
          args_are_valid = false;
      }
      else
      {
        if (arg_is_expected)
          args_are_valid = false;
      }
    }

    if (!args_are_valid)
      mooseError("The property call '",
                 _property_call,
                 "' expects the parameter(s) {",
                 MooseUtils::join(expected_args, ", "),
                 "} to be provided, but the provided argument(s) were {",
                 MooseUtils::join(provided_args, ", "),
                 "}.");
  }
  else
    mooseError("Property call in MooseEnum but not _n_expected_args.");
}

void
TwoPhaseNCGPartialPressureFunction::initialSetup()
{
  _fp = &getUserObject<TwoPhaseNCGPartialPressureFluidProperties>("fluid_properties");
}

Real
TwoPhaseNCGPartialPressureFunction::value(Real t, const Point & point) const
{
  const Real arg1 = _arg1_fn.value(t, point);
  const Real arg2 = _arg2_fn.value(t, point);

  if (_property_call == "p_sat")
    return _fp->p_sat(arg1);
  else if (_property_call == "x_sat_ncg_from_p_T")
    return _fp->x_sat_ncg_from_p_T(arg1, arg2);
  else
    mooseError("Unimplemented property call.");
}
