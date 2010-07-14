#include "Moose.h" //for mooseError
#include "UserFunction.h"

template<>
InputParameters validParams<UserFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("function", "The user defined function.");
  params.addParam<std::vector<std::string> >("vars", std::vector<std::string>(0), "The constant variables (excluding t,x,y,z) in the forcing function.");
  params.addParam<std::vector<Real> >("vals", std::vector<Real>(0), "The values that correspond to the variables");
  return params;
}

UserFunction::UserFunction(std::string name, MooseSystem & moose_system, InputParameters parameters):
  Function(name, moose_system, parameters)
{
  initialize(parameters.get<std::string>("function"),
           parameters.get<std::vector<std::string> >("vars"),
           parameters.get<std::vector<Real> >("vals"));
}

UserFunction::~UserFunction()
{
}

void
UserFunction::initialize( std::string equation, std::vector<std::string> vars, std::vector<Real> vals )
{
  std::string variables = "t,x,y,z";
  for( int i = 0; i < vars.size(); i++ )
    variables += "," + vars[i];

  //add constants to the parser, like pi and e
  defineConstants();

  int res = _parser.Parse( equation, variables );
  if (res != -1 )
  {
    //Show the user their equation with a ^ where the parsing error is
    std::string msg(res, ' ');
    msg = "ERROR: Can not parse function\n" + equation + "\n" + msg + "^\n";
    mooseError(msg);
  }

  mooseAssert( vars.size() <= vals.size(), "There can't be more values than variables!" );
  _vars.resize(vars.size() + 4);

  //prebuild map of variable names to pointers to put the variable value
  for (int i = 0; i < vars.size(); i++)
    _var_map[vars[i]] = &(_vars[4+i]); //add 4 to account for t,x,y,z

  //fill in the variables whose values we already know. The values in vals fill
  //in the first n variables in the variables list.
  for (int i = 0; i < vals.size(); i++)
    _vars[i+4] = vals[i];

  //TODO compile with support for optimization (ask about alloca)
  _parser.Optimize();
}

void
UserFunction::defineConstants()
{
  _parser.AddConstant("pi", 3.14159265358979323846);
  _parser.AddConstant("e", 2.71828182845904523536);
}

Real &
UserFunction::getVarAddr( std::string var )
{
  //TODO is this the best syntax?
  Real * v = _var_map[var];
  return *v;
}

Real
UserFunction::operator()(Real t, Real x, Real y, Real z)
{
  _vars[0] = t;
  _vars[1] = x;
  _vars[2] = y;
  _vars[3] = z;
  return _parser.Eval(&(_vars[0]));
}
