#include "CoupledForce.h"

template<>
InputParameters validParams<CoupledForce>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("v", "");
  
  return params;
}

CoupledForce::CoupledForce(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v"))
{
}

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
