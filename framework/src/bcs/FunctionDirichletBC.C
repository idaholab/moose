#include "FunctionDirichletBC.h"

template<>
InputParameters validParams<FunctionDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<bool>("_integrated") = false;
  params.addRequiredParam<std::string>("function", "The user defined forcing function.");
  params.addParam<std::vector<std::string> >("vars", std::vector<std::string>(0), "The variables (excluding t,x,y,z) in the function.");
  params.addParam<std::vector<Real> >("vals", std::vector<Real>(0), "The values that correspond to the variables");
  return params;
}

FunctionDirichletBC::FunctionDirichletBC(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :BoundaryCondition(name, sys, parameters),
  _functor(parameters.get<std::string>("function"),
           parameters.get<std::vector<std::string> >("vars"),
           parameters.get<std::vector<Real> >("vals"))
{
}

Real
FunctionDirichletBC::f()
{
  return _functor(_t, (*_current_node)(0), (*_current_node)(1), (*_current_node)(2));
}

Real
FunctionDirichletBC::computeQpResidual()
{
  return _u[_qp]-f();
}
