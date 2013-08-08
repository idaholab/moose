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
  params.addParam<std::vector<std::string> >("vals", "The initial values of the variables (optional)");

  return params;
}

MooseParsedFunction::MooseParsedFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _value(verifyInput(name, getParam<std::string>("value"))),
    _vars(verifyVars(name, getParam<std::vector<std::string> >("vars"))),
    _input_vals(getParam<std::vector<std::string> >("vals")),
    _initialized(false)
{
}

Real
MooseParsedFunction::value(Real t, const Point & pt)
{
  // This gurantees that the function is initilized
  mooseAssert(_initialized, "The 'initialSetup' method must be called before the 'value' method");

  // Make certain the things are up to date
  updatePostprocessorValues();

  // Evaluate and return the libMesh::ParsedFunction
  return (*_function)(pt, t);
}

MooseParsedFunction::~MooseParsedFunction()
{
  // Cleanup the libMesh::ParsedFunction object
  delete _function;
}

const std::string
MooseParsedFunction::verifyInput(const std::string & name, const std::string & value)
{
  // Throws an error if quotes are found
  if (value.find("\"") != std::string::npos)
    mooseError("The value in ParsedFunction \"" + name + "\" contains quotes(\") which cannot be properly parsed");

  // Return the input equation (no error)
  return value;
}

const std::vector<std::string>
MooseParsedFunction::verifyVars(const std::string & name, const std::vector<std::string> & vars)
{
  // Loop through the variables assigned by the user and give an error if x,y,z,t are used
  for (unsigned int i=0; i < vars.size(); ++i)
    if (vars[i].find_first_of("xyzt") != std::string::npos && vars[i].size() == 1)
      mooseError("The variables \"x, y, z, and t\" in the ParsedFunction \"" + name + "\" are pre-declared for use and must not be declared in \"vars\"");

  // Return the variables (no error)
  return vars;
}

void MooseParsedFunction::updatePostprocessorValues()
{
  // Loop through the variables that are Postprocessors and update the libMesh::ParsedFunction
  for (unsigned int i = 0; i < _pp_index.size(); ++i)
    (*_var_addr[i]) = (*_pp_vals[i]);
}

void
MooseParsedFunction::initialSetup()
{
  if(_initialized)
    return;

  // Loop through all the input values supplied by the users.
  for (unsigned int i=0; i < _input_vals.size(); ++i)
  {
    Real tmp; // desired type
    std::istringstream ss(_input_vals[i]); // istringstream object for performing conversion from std::string to Real

    // Case when a Postprocessor is found by the name given in the input values
    if (hasPostprocessor(_input_vals[i]))
    {
      // Store a pointer to the Postprocessor value
      _pp_vals.push_back(&getPostprocessorValueByName(_input_vals[i]));

      // Store the value for passing to the the libMesh::ParsedFunction
      _vals.push_back(getPostprocessorValueByName(_input_vals[i]));

      // Store the location of this variable
      _pp_index.push_back(i);
    }

    // Case when a Real is supplied, convert std::string to Real
    else
    {
      // Use istringstream to convert, if it fails produce an error, otherwise add the variable to the _vals variable
      if (!(ss >> tmp))
        mooseError("The input value '" << _input_vals[i] << "' was not understood, it must be a Real or a Postprocessor");
      else
        _vals.push_back(tmp);
    }
  }

  // Build the libMesh::ParsedFunction
  _function = new ParsedFunction<Real>(_value, &_vars, &_vals);

  // Loop through the Postprocessor variables and point the libMesh::ParsedFunction to the PostprocessorValue
  for (unsigned int i = 0; i < _pp_index.size(); ++i)
    _var_addr.push_back(&_function->getVarAddress(_vars[_pp_index[i]]));

  // Update postprocessor values
  updatePostprocessorValues();

  // The class is now initialized
   _initialized = true;
}
