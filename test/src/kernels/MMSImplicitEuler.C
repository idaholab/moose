#include "MMSImplicitEuler.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<MMSImplicitEuler>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

MMSImplicitEuler::MMSImplicitEuler(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
{}

Real
MMSImplicitEuler::computeQpResidual()
{
  return _test[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
}

Real
MMSImplicitEuler::computeQpJacobian()
{
  return _test[_i][_qp]*_phi[_j][_qp]/_dt;
}
