#include "Functor.h"

Functor::Functor( std::string equation, std::vector<std::string> vars ):
  _vars(vars.size() + 4)
{
  std::string variables = "t,x,y,z";
  for( int i = 0; i < vars.size(); i++ )
    variables += "," + vars[i];

  _parser.AddConstant("pi", 3.1415926535897932);
  _parser.AddConstant("e", 2.71828183);
  int res = _parser.Parse( equation, variables );
  if (res != -1 )
  {
    //Show the user their equation with a ^ where the parsing error is
    std::string msg(res, ' ');
    msg = "ERROR: Can not parse function\n" + equation + "\n" + msg + "^\n";
    std::cout << msg;
    return; //TODO MooseError(msg);
  }

  //prebuild map of variable names to pointers to put the variable value
  for (int i = 0; i < vars.size(); i++)
    _var_map[vars[i]] = &(_vars[4+i]); //add 4 to account for t,x,y,z

  //TODO compile with support for optimization (ask about alloca)
  _parser.Optimize();
}

Functor::~Functor()
{
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
