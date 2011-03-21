#include "DirichletBCfuncXYZ1.h"

template<>
InputParameters validParams<DirichletBCfuncXYZ1>()
{
  InputParameters params = validParams<NodalBC>();

  params.set<Real>("A0")=0.;
  params.set<Real>("B0")=0.;
  params.set<Real>("C0")=0.;
  params.set<Real>("omega0")=1.;
  
  params.set<bool>("_integrated") = false;
  return params;
}

DirichletBCfuncXYZ1::DirichletBCfuncXYZ1(const std::string & name, InputParameters parameters)
  :NodalBC(name, parameters)
  {
     _A0     = getParam<Real>("A0");
     _B0     = getParam<Real>("B0");
     _C0     = getParam<Real>("C0");
     _omega0 = getParam<Real>("omega0");
  }

Real
DirichletBCfuncXYZ1::computeQpResidual()
  {
     return _u[_qp]-ManSol4ADR2(*_current_node,_A0,_B0,_C0,_omega0,_t);
  }

Real
DirichletBCfuncXYZ1::computeQpJacobian()
  {
  /// FIMXE: !!!
//    return _phi[_j][_qp];
  return 0;
  }
