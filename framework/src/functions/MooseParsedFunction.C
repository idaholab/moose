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

#include "Moose.h" //for mooseError
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
    _function(getParam<std::string>("value"),
              isParamValid("vars") ? &getParam<std::vector<std::string> >("vars") : NULL,
              isParamValid("vals") ? &getParam<std::vector<Real> >("vals") : NULL)
{}

Real
MooseParsedFunction::value(Real t, const Point & pt)
{
  return _function(pt, t);
}
