#include "CoupledForce.h"

template<>
InputParameters validParams<CoupledForce>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("v", "");
  
  return params;
}

CoupledForce::CoupledForce(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
    _v_var(coupled("v")),
    _v(coupledVal("v"))
  {}

Real
CoupledForce::computeQpResidual()
{
  return -_v[_qp]*_test[_i][_qp];
}

Real
CoupledForce::computeQpJacobian()
{
  return 0;
}

Real
CoupledForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _v_var)
    return -_phi[_j][_qp]*_test[_i][_qp];    
  return 0.0;
}
