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
#include "SolutionAux.h"

template<>
InputParameters validParams<SolutionAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("solution", "The name of the SolutionUserObject");
  params.addParam<std::string>("from_variable", "The name of the variable to extract from the file");

  params.addParam<bool>("direct", false, "If true the meshes must be the same and then the values are simply copied over.");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor (a)  to be applied to the solution (x): ax+b, where b is the 'add_factor'");
  params.addParam<Real>("add_factor", 0.0, "Add this value (b) to the solution (x): ax+b, where a is the 'scale_factor'");
  return params;
}

SolutionAux::SolutionAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _solution_object(getUserObject<SolutionUserObject>("solution")),
    _direct(getParam<bool>("direct")),
    _scale_factor(getParam<Real>("scale_factor")),
    _add_factor(getParam<Real>("add_factor"))
{

  // If 'from_variable' is supplied, use the value
  if (isParamValid("from_variable"))
    _var_name = getParam<std::string>("from_variable");

  // If not, get the value from the SolutionUserObject
  else
  {
    // Get the variables from the SolutionUserObject
    std::vector<std::string> vars = _solution_object.getParam<std::vector<std::string> >("variables");

    // If there are more than one, throw an error
    if (vars.size() > 1)
      mooseError("The SolutionUserObject contains multiple variables, in this case the SolutionFunction must specifiy the desired variable in the input file with 'from_variable'");

    // Define the variable
    _var_name = vars[0];
  }
}

SolutionAux::~SolutionAux()
{
}

Real
SolutionAux::computeValue()
{
  // The value to output
  Real output;  // _direct=true, extract the values using the dof
  if(_direct)
  {
    if (isNodal())
      output = _solution_object.directValue(_current_node, _var_name);

    else
      output = _solution_object.directValue(_current_elem, _var_name);
  }

  // _direct=false, extract the values using time and point
  else
  {
    if (isNodal())
      output = _solution_object.pointValue(_t, *_current_node, _var_name);

    else
      output = _solution_object.pointValue(_t, _current_elem->centroid(), _var_name);
  }

  // Apply factors and return the value
  return _scale_factor*output + _add_factor;
}
