//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

InputParameters
KKSAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::string>("c_name_base", "c", "base name of the concentration variables");
  params.addParam<std::string>("eta_name", "eta", "name of the order parameter");
  params.addRequiredParam<std::vector<std::string>>("phase_names", "short names for the phases");
  params.addRequiredParam<std::vector<std::string>>("c_names",
                                                    "short names for the concentrations");
  return params;
}

KKSAction::KKSAction(const InputParameters & params) : Action(params)
// _z2(getParam<Real>("z2"))
{
}

void
KKSAction::act()
{
  mooseError("Not implemented");
}
