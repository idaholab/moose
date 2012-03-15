#include "MMSImplicitEuler.h"

template<>
InputParameters validParams<MMSImplicitEuler>()
{
  InputParameters params = validParams<TimeKernel>();
  return params;
}

MMSImplicitEuler::MMSImplicitEuler(const std::string & name, InputParameters parameters) :
    TimeKernel(name, parameters),
    _u_old(valueOld())
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
