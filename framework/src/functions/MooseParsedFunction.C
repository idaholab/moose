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
#include "MooseParsedFunction.h"

template<>
InputParameters validParams<MooseParsedFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("value", "The user defined function.");
  params.addParam<std::vector<std::string> >("vars", "The constant variables (excluding t,x,y,z) in the forcing function.");
  params.addParam<std::vector<Real> >("vals", "The initial_vals of the variables (optional)");

  return params;
}

MooseParsedFunction::MooseParsedFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _function(verifyInput(name, getParam<std::string>("value")) ? getParam<std::string>("value") : "",
              isParamValid("vars") ? verifyVars(&getParam<std::vector<std::string> >("vars")) : NULL,
              isParamValid("vals") ? &getParam<std::vector<Real> >("vals") : NULL)
{}

Real
MooseParsedFunction::value(Real t, const Point & pt)
{
  return _function(pt, t);
}

bool
MooseParsedFunction::verifyInput(const std::string & name, const std::string & value)
{
  // Error Checking
  if (value.find("\"") != std::string::npos)
    mooseError("The value in ParsedFunction \"" + name + "\" contains quotes(\") which cannot be properly parsed");
  return true;
}

const std::vector<std::string> *
MooseParsedFunction::verifyVars(const std::vector<std::string> * vars)
{
  for (unsigned int i=0; i < vars->size(); ++i)
    if ((*vars)[i].find_first_of("xyzt") != std::string::npos)
      mooseError("ParsedFunction: The variables \"x, y, z, and t\" are pre-declared for use and must not be declared in \"vars\"");

  return vars;
}
