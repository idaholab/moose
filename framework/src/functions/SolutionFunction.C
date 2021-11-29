//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "SolutionFunction.h"
#include "SolutionUserObject.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", SolutionFunction);

InputParameters
SolutionFunction::validParams()
{
  // Get the Function input parameters
  InputParameters params = Function::validParams();
  params.addClassDescription("Function for reading a solution from file.");

  // Add required parameters
  params.addRequiredParam<UserObjectName>("solution",
                                          "The SolutionUserObject to extract data from.");
  params.addParam<std::string>("from_variable",
                               "The name of the variable in the file that is to be extracted");

  // Add optional paramters
  params.addParam<Real>(
      "scale_factor",
      1.0,
      "Scale factor (a) to be applied to the solution (x): ax+b, where b is the 'add_factor'");
  params.addParam<Real>(
      "add_factor",
      0.0,
      "Add this value (b) to the solution (x): ax+b, where a is the 'scale_factor'");

  // Return the parameters object
  return params;
}

SolutionFunction::SolutionFunction(const InputParameters & parameters)
  : Function(parameters),
    _solution_object_ptr(NULL),
    _scale_factor(getParam<Real>("scale_factor")),
    _add_factor(getParam<Real>("add_factor"))
{
  for (unsigned int d = 0; d < _ti_feproblem.mesh().dimension(); ++d)
    _add_grad(d) = _add_factor;
}

void
SolutionFunction::initialSetup()
{
  // Get a pointer to the SolutionUserObject. A pointer is used because the UserObject is not
  // available during the
  // construction of the function
  _solution_object_ptr = &getUserObject<SolutionUserObject>("solution");

  std::string var_name;

  // If 'from_variable' is supplied, use the value
  if (isParamValid("from_variable"))
    var_name = getParam<std::string>("from_variable");

  // If not, get the value from the SolutionUserObject
  else
  {
    // Get all the variables from the SolutionUserObject
    const std::vector<std::string> & vars = _solution_object_ptr->variableNames();

    // If there are more than one, throw an error
    if (vars.size() > 1)
      mooseError("The SolutionUserObject contains multiple variables, the SolutionFunction must "
                 "specifiy the desired variable in the input file with 'from_variable'");

    // Define the variable
    var_name = vars[0];
  }
  _solution_object_var_index = _solution_object_ptr->getLocalVarIndex(var_name);
}

Real
SolutionFunction::value(Real t, const Point & p) const
{
  return _scale_factor * (_solution_object_ptr->pointValue(t, p, _solution_object_var_index)) +
         _add_factor;
}

RealGradient
SolutionFunction::gradient(Real t, const Point & p) const
{
  return _scale_factor *
             (_solution_object_ptr->pointValueGradient(t, p, _solution_object_var_index)) +
         _add_grad;
}
