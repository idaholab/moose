#include "FunctionDirichletBC.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionDirichletBC>()
{
  InputParameters params = validParams<NodalBC>();
//  params.set<bool>("_integrated") = false;
  params.addRequiredParam<std::string>("function", "The forcing function.");
  return params;
}

FunctionDirichletBC::FunctionDirichletBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),
    _func(getFunction("function"))
{
}

Real
FunctionDirichletBC::f()
{
  return _func.value(_t, (*_current_node)(0), (*_current_node)(1), (*_current_node)(2));
}

Real
FunctionDirichletBC::computeQpResidual()
{
  return _u[_qp]-f();
}
