#include "DirichletBCfuncXYZ0.h"

template<>
InputParameters validParams<DirichletBCfuncXYZ0>()
{
  InputParameters params = validParams<NodalBC>();

  params.set<Real>("A0")=0.;
  params.set<Real>("B0")=0.;
  params.set<Real>("C0")=0.;
  params.set<Real>("omega0")=1.;
  
  params.set<bool>("_integrated") = false;
  return params;
}

DirichletBCfuncXYZ0::DirichletBCfuncXYZ0(const std::string & name, InputParameters parameters)
  :NodalBC(name, parameters)
  {
     _A0     = getParam<Real>("A0");
     _B0     = getParam<Real>("B0");
     _C0     = getParam<Real>("C0");
     _omega0 = getParam<Real>("omega0");
  }

Real
DirichletBCfuncXYZ0::computeQpResidual()
  {
     return _u[_qp]-ManSol4ADR1(*_current_node,_A0,_B0,_C0,_omega0,_t,_is_transient);
  }

Real
DirichletBCfuncXYZ0::computeQpJacobian()
  {
    return 0;
    // FIXME: !!!
//    return _phi[_j][_qp];
  }
