//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OverrideCliArgs.h"

registerMooseObject("MooseTestApp", OverrideCliArgs);

InputParameters
OverrideCliArgs::validParams()
{
  InputParameters params = FullSolveMultiApp::validParams();
  params.addRequiredParam<Real>("xmax", "Max x dimension of mesh in subapp");
  params.addClassDescription("Tests the ability to override cli_args param.");
  return params;
}

OverrideCliArgs::OverrideCliArgs(const InputParameters & parameters) : FullSolveMultiApp(parameters)
{
}

std::vector<std::string>
OverrideCliArgs::cliArgs() const
{
  std::stringstream stream;
  stream << "Mesh/gen/xmax=" << getParam<Real>("xmax");
  std::vector<std::string> r = {stream.str()};
  return r;
}
