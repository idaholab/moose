#include "GravityPower.h"
 

template<>
InputParameters validParams<GravityPower>()
{
  InputParameters params;
  params.set<Real>("acceleration") = -9.80665;
  return params;
}

GravityPower::GravityPower(std::string name,
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
    _pv_var(coupled("pv")),
    _pv(coupledVal("pv")),
    _acceleration(parameters.get<Real>("acceleration"))
  {}

Real
GravityPower::computeQpResidual()
{
  return -_pv[_qp]*_acceleration*_phi[_i][_qp];
}

Real
GravityPower::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _pv_var)
  {
    return -_phi[_j][_qp]*_acceleration*_phi[_i][_qp];
  }

  return 0;
}
