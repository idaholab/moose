#include "FunctionPenaltyDirichletBC.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionPenaltyDirichletBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("penalty",1e6,"Penalty scalar");
  params.addRequiredParam<FunctionName>("function", "Forcing function");

  return params;
}

FunctionPenaltyDirichletBC::FunctionPenaltyDirichletBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _func(getFunction("function")),
    _p(getParam<Real>("penalty"))
{}

Real
FunctionPenaltyDirichletBC::computeQpResidual()
{
  return _p*_test[_i][_qp]*(-_func.value(_t,_q_point[_qp]) + _u[_qp] );
}

Real
FunctionPenaltyDirichletBC::computeQpJacobian()
{
  return _p*_phi[_j][_qp]*_test[_i][_qp];
}
