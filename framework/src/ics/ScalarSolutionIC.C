//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarSolutionIC.h"
#include "SolutionUserObjectBase.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", ScalarSolutionIC);
registerMooseObjectRenamed("MooseApp",
                           ScalarSolutionInitialCondition,
                           "06/30/2024 24:00",
                           ScalarSolutionIC);

InputParameters
ScalarSolutionIC::validParams()
{
  InputParameters params = ScalarInitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("solution_uo",
                                          "The SolutionUserObject to extract data from.");
  params.addRequiredParam<VariableName>(
      "from_variable", "The name of the variable in the file that is to be extracted");
  params.addClassDescription(
      "Sets the initial condition from a scalar variable stored in an Exodus file, "
      "retrieved by a SolutionUserObject");

  return params;
}

ScalarSolutionIC::ScalarSolutionIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters),
    _solution_object(getUserObject<SolutionUserObjectBase>("solution_uo")),
    _solution_object_var_name(getParam<VariableName>("from_variable"))
{
}

Real
ScalarSolutionIC::value()
{
  return _solution_object.scalarValue(0., _solution_object_var_name);
}
