#include "Moose.h" //for mooseAssert
#include "Functor.h"

Functor::Functor( std::string equation, std::vector<std::string> vars, std::vector<Real> vals ):
  _vars(vars.size() + 4)
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

  //prebuild map of variable names to pointers to put the variable value
  for (int i = 0; i < vars.size(); i++)
    _var_map[vars[i]] = &(_vars[4+i]); //add 4 to account for t,x,y,z

  //fill in the variables whose values we already know. The values in vals fill
  //in the first n variables in the variables list.
  mooseAssert( vars.size() <= vals.size(), "There can't be more values than variables!" );
  for (int i = 0; i < vals.size(); i++)
    _vars[i+4] = vals[i];

  //TODO compile with support for optimization (ask about alloca)
  _parser.Optimize();
}

Functor::~Functor()
{
}

void
Functor::defineConstants()
{
  _parser.AddConstant("pi", 3.1415926535897932);
  _parser.AddConstant("e", 2.71828183);
}

Real &
Functor::getVarAddr( std::string var )
{
  //TODO is this the best syntax?
  Real * v = _var_map[var];
  return *v;
}

Real
Functor::operator()(Real t, Real x, Real y, Real z)
{
  _vars[0] = t;
  _vars[1] = x;
  _vars[2] = y;
  _vars[3] = z;
  return _parser.Eval(&(_vars[0]));
}

/*
int main()
{
  std::vector<std::string> vars(1);
  vars[0] = "q";

  Functor f( std::string("y+q*x*t"), vars );

  Real & q = f.getVarAddr( std::string("q") );
  q = 1.5;

  std::cout << f(1, 2, 3) << "\n"; // 3 + 1.5 * 2 * 1 = 6

  q = 2.5;
  std::cout << f(1, 2, 3) << "\n"; // 3 + 2.5 * 2 * 1 = 8

  return 0;
}*/
