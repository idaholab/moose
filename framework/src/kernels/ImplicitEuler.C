
#include "ImplicitEuler.h"

template<>
InputParameters validParams<ImplicitEuler>()
{
  InputParameters params;
  return params;
}


ImplicitEuler::ImplicitEuler(std::string name,
                InputParameters parameters,
                std::string var_name,
                std::vector<std::string> coupled_to,
                std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as)
{}

Real
ImplicitEuler::computeQpResidual()
{
  return _phi[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
}

Real
ImplicitEuler::computeQpJacobian()
{
  return _phi[_i][_qp]*_phi[_j][_qp]/_dt;
}
