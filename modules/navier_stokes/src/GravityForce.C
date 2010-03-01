#include "GravityForce.h"
 

template<>
InputParameters validParams<GravityForce>()
{
  InputParameters params;
  params.set<Real>("acceleration") = -9.80665;
  return params;
}

GravityForce::GravityForce(std::string name,
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
    _p_var(coupled("p")),
    _p(coupledVal("p")),
    _acceleration(parameters.get<Real>("acceleration"))
  {}

Real
GravityForce::computeQpResidual()
{
  return -_p[_qp]*_acceleration*_test[_i][_qp];
}

Real
GravityForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _p_var)
  {
    return -_phi[_j][_qp]*_acceleration*_test[_i][_qp];
  }

  return 0;
}
