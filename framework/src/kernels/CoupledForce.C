#include "CoupledForce.h"

CoupledForce::CoupledForce(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
    _v_var(coupled("v")),
    _v(coupledVal("v"))
  {}
Real
CoupledForce::computeQpResidual()
  {
    return -_v[_qp]*_phi[_i][_qp];
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
      return -_phi[_j][_qp]*_phi[_i][_qp];    
    return 0.0;
  }
