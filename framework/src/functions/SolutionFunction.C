/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseError.h"
#include "SolutionFunction.h"


template<>
InputParameters validParams<SolutionFunction>()
{
  // Get the Function input parameters
  InputParameters params = validParams<Function>();

  // Add required parameters
  params.addRequiredParam<UserObjectName>("solution", "The SolutionUserObject to extract data from.");
  params.addParam<std::string>("from_variable", "The name of the variable in the file that is too be extracted");

  // Add optional paramters
  params.addParam<Real>("scale_factor", 1.0, "Scale factor (a) to be applied to the solution (x): ax+b, where b is the 'add_factor'");
  params.addParam<Real>("add_factor", 0.0, "Add this value (b) to the solution (x): ax+b, where a is the 'scale_factor'");

  // Return the parameters object
  return params;
}

SolutionFunction::SolutionFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _solution_object_ptr(NULL),
    _scale_factor(getParam<Real>("scale_factor")),
    _add_factor(getParam<Real>("add_factor"))
{
}

SolutionFunction::~SolutionFunction()
{
}

void
SolutionFunction::initialSetup()
{
  // Get a pointer to the SolutionUserObject. A pointer is used because the UserObject is not available during the
  // construction of the function
  _solution_object_ptr = &getUserObject<SolutionUserObject>("solution");

  // If 'from_variable' is supplied, use the value
  if (isParamValid("from_variable"))
    _var_name = getParam<std::string>("from_variable");

  // If not, get the value from the SolutionUserObject
  else
  {
    // Get all the variables from the SolutionUserObject
    std::vector<std::string> vars = _solution_object_ptr->getParam<std::vector<std::string> >("nodal_variables");
    std::vector<std::string> elem_vars = _solution_object_ptr->getParam<std::vector<std::string> >("elemental_variables");
    vars.insert(vars.end(), elem_vars.begin(), elem_vars.end());

    // If there are more than one, throw an error
    if (vars.size() > 1)
      mooseError("The SolutionUserObject contains multiple variables, the SolutionFunction must specifiy the desired variable in the input file with 'from_variable'");

    // Define the variable
    _var_name = vars[0];
  }
}

Real
SolutionFunction::value(Real t, const Point & p)
{
  return _scale_factor*(_solution_object_ptr->pointValue(t, p, _var_name)) + _add_factor;
}
