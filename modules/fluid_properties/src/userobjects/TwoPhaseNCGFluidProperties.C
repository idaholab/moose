//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseNCGFluidProperties.h"

InputParameters
TwoPhaseNCGFluidProperties::validParams()
{
  InputParameters params = TwoPhaseFluidProperties::validParams();
  params.set<std::string>("fp_type") = "two-phase-ncg-fp";
  params.addParam<UserObjectName>("fp_2phase",
                                  "Name of fluid properties user object(s) for two-phase model");
  return params;
}

TwoPhaseNCGFluidProperties::TwoPhaseNCGFluidProperties(const InputParameters & parameters)
  : TwoPhaseFluidProperties(parameters),
    _2phase_name(isParamValid("fp_2phase") ? getParam<UserObjectName>("fp_2phase")
                                           : UserObjectName(name() + ":2phase")),
    _vapor_mixture_name(name() + ":vapor_mixture")
{
  // check that the user has not already created user objects of the same name
  if (_tid == 0 && _fe_problem.hasUserObject(_vapor_mixture_name))
    mooseError("A user object with the name '", _vapor_mixture_name, "' already exists.");
}
