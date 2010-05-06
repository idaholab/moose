#include "ImplicitEuler.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<ImplicitEuler>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

ImplicitEuler::ImplicitEuler(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
{}

Real
ImplicitEuler::computeQpResidual()
{
  return _test[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
}

Real
ImplicitEuler::computeQpJacobian()
{
  return _test[_i][_qp]*_phi[_j][_qp]/_dt;
}
