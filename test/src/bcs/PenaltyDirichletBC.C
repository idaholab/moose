#include "PenaltyDirichletBC.h"
#include "Function.h"
#include "numeric_vector.h"

template<>
InputParameters validParams<PenaltyDirichletBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("penalty",1e5,"Penalty scalar");
  params.addParam<Real>("value", 0.0, "Boundary value of the variable");
  params.addRequiredParam<std::string>("function", "Forcing function");
  
  return params;
}

PenaltyDirichletBC::PenaltyDirichletBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _func(getFunction("function")),
    _p(getParam<Real>("penalty")),
    _v(getParam<Real>("value"))
{}

Real
PenaltyDirichletBC::computeQpResidual()
{
  return _p*_test[_i][_qp]*(-_v + _u[_qp]);
}

Real
PenaltyDirichletBC::computeQpJacobian()
{
  return _p*_phi[_j][_qp]*_test[_i][_qp];
}
