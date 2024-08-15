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
  // Check that the number of provided arguments matches the expected number
  const auto n_provided_args = getNumberOfProvidedArguments();
  if (_n_expected_args.find(_property_call) != _n_expected_args.end())
  {
    const auto n_expected_args = _n_expected_args.at(_property_call);
    if (n_provided_args != n_expected_args)
      mooseError("The property call '",
                 _property_call,
                 "' expects ",
                 n_expected_args,
                 " argument parameter(s) to be provided, but the provided number of arguments was ",
                 n_provided_args,
                 ".");
  }
  else
    mooseError("Property call in MooseEnum but not _n_expected_args.");
}

void
TwoPhaseNCGPartialPressureFunction::initialSetup()
{
  _fp = &getUserObject<TwoPhaseNCGPartialPressureFluidProperties>("fluid_properties");
}

unsigned int
TwoPhaseNCGPartialPressureFunction::getNumberOfProvidedArguments() const
{
  if (isParamSetByUser("arg1"))
  {
    if (isParamSetByUser("arg2"))
      return 2;
    else
      return 1;
  }
  else
    return 0;
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
