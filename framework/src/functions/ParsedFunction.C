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
#include "ParsedFunction.h"

template<>
InputParameters validParams<ParsedFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("value", "The user defined function.");
  params.addParam<std::vector<std::string> >("vars", std::vector<std::string>(0), "The constant variables (excluding t,x,y,z) in the forcing function.");
  params.addParam<std::vector<Real> >("vals", std::vector<Real>(0), "The values that correspond to the variables");
  return params;
}

ParsedFunction::ParsedFunction(const std::string & name, InputParameters parameters):
  Function(name, parameters)
{
  std::vector<std::string> vars = parameters.get<std::vector<std::string> >("vars");
  std::vector<Real>        vals = parameters.get<std::vector<Real> >("vals");

  //set up array to hold the variable values
  mooseAssert(vars.size() >= vals.size(), "There can't be more values than variables!");
  _vals.resize(vars.size() + 4);

  //prebuild map of variable names to pointers to put the variable value
  for (unsigned int i = 0; i < vars.size(); i++)
    _var_map[vars[i]] = &(_vals[4+i]); //add 4 to account for t,x,y,z

  //fill in the variables whose values we already know. The values in vals fill
  //in the first n variables in the variables list.
  for (unsigned int i = 0; i < vals.size(); i++)
    _vals[i+4] = vals[i];

  initializeParser(_parser, parameters.get<std::string>("value"), vars, vals);
}

ParsedFunction::~ParsedFunction()
{
}

void
ParsedFunction::initializeParser(FunctionParser & fp, std::string equation, std::vector<std::string> vars, std::vector<Real> /*vals*/)
{
  //add constants to the parser, like pi and e
  defineConstants(fp);

  std::string variables = "t,x,y,z";
  for (unsigned int i = 0; i < vars.size(); i++)
    variables += "," + vars[i];

  int res = fp.Parse( equation, variables );
  if (res != -1)
  {
    //Show the user their equation with a ^ where the parsing error is
    std::string msg(res+1, ' ');
    msg = "ERROR: Can not parse function\n'" + equation + "'\n" + msg + "^\n";
    mooseError(msg);
  }

  //TODO compile with support for optimization (edit fpconfig.h I think)
  fp.Optimize();
}

void
ParsedFunction::defineConstants(FunctionParser & fp)
{
  fp.AddConstant("pi", 3.14159265358979323846);
  fp.AddConstant("e", 2.71828182845904523536);
}

Real &
ParsedFunction::getVarAddr(std::string var)
{
  //TODO is this the best syntax?
  Real * v = _var_map[var];
  return *v;
}

Real
ParsedFunction::value(Real t, Real x, Real y, Real z)
{
  _vals[0] = t;
  _vals[1] = x;
  _vals[2] = y;
  _vals[3] = z;
  return _parser.Eval(&(_vals[0]));
}
