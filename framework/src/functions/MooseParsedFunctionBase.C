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
/*           See COPYRIGHT for full restrictions                */
/****************************************************************/

#include "MooseError.h"
#include "MooseParsedFunctionBase.h"
//#include "MooseParsedFunction.h"

template<>
InputParameters validParams<MooseParsedFunctionBase>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::string> >("vars", "The constant variables (excluding t,x,y,z) in the forcing function.");
  params.addParam<std::vector<std::string> >("vals", "The initial values of the variables (optional)");
  return params;
}

MooseParsedFunctionBase::MooseParsedFunctionBase(const std::string & /*name*/, InputParameters parameters) :
    _pfb_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
    _vars(verifyVars(parameters)),
    _vals(verifyVals(parameters))
{
}

MooseParsedFunctionBase::~MooseParsedFunctionBase()
{
}

const std::string
MooseParsedFunctionBase::verifyFunction(const std::string & function_str)
{
  // Throws an error if quotes are found
  if (function_str.find("\"") != std::string::npos)
    mooseError("The value in ParsedFunction contains quotes(\") which cannot be properly parsed");

  // Return the input equation (no error)
  return function_str;
}

const std::vector<std::string>
MooseParsedFunctionBase::verifyVars(const InputParameters & parameters)
{
  // Test the vars is defined
  if (!parameters.have_parameter<std::vector<std::string> >("vars"))
      mooseError("An input of std::vector<std::string> named 'vars' must exist.");

  // Get the 'vars' parameter
  const std::vector<std::string> vars = parameters.get<std::vector<std::string> >("vars");

  // Loop through the variables assigned by the user and give an error if x,y,z,t are used
  for (unsigned int i=0; i < vars.size(); ++i)
    if (vars[i].find_first_of("xyzt") != std::string::npos && vars[i].size() == 1)
      mooseError("The variables \"x, y, z, and t\" in the ParsedFunction are pre-declared for use and must not be declared in \"vars\"");

  // Return the variables (no error)
  return vars;
}

const std::vector<std::string>
MooseParsedFunctionBase::verifyVals(const InputParameters & parameters)
{
  // Test the vars is defined
  if (!parameters.have_parameter<std::vector<std::string> >("vals"))
      mooseError("An input of std::vector<std::string> named 'vals' must exist.");

  // Return the 'vals' parameter
  return parameters.get<std::vector<std::string> >("vals");
}
