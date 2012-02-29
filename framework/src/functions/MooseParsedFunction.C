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
  params.addParam<std::vector<std::string> >("vars", std::vector<std::string>(0), "The constant variables (excluding t,x,y,z) in the forcing function.");
  params.addParam<std::vector<Real> >("vals", std::vector<Real>(0), "The values that correspond to the variables");
  return params;
}

MooseParsedFunction::MooseParsedFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _function(new ParsedFunction<Real>(getParam<std::string>("value"),
                                       &getParam<std::vector<std::string> >("vars"),
                                       &getParam<std::vector<Real> >("vals")))
{}

MooseParsedFunction::~MooseParsedFunction()
{
  delete _function;
}

/* TODO: Currently not implemented however 'pi' and 'e' are defined in the libMesh base class
void
MooseParsedFunction::defineConstants(FunctionParser & fp)
{
  fp.AddConstant("pi", 3.14159265358979323846);
  fp.AddConstant("e", 2.71828182845904523536);
}

Real &
MooseParsedFunction::getVarAddr(std::string var)
{
  //TODO is this the best syntax?
  Real * v = _var_map[var];
  return *v;
}
*/

Real
MooseParsedFunction::value(Real t, const Point & pt)
{
  return (*_function)(pt, t);
}
