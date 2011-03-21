#include "TimeDerivative.h"

template<>
InputParameters validParams<TimeDerivative>()
{
  InputParameters params = validParams<TimeKernel>();
  return params;
}

TimeDerivative::TimeDerivative(const std::string & name, InputParameters parameters) :
    TimeKernel(name, parameters)//,
//    _time_weight(_moose_system._time_weight)
{
}

Real
TimeDerivative::computeQpResidual()
{
//  std::cout<<"u_dot: "<<_u_dot[_qp] - ((_u[_qp] - _u_old[_qp])/_dt)<<std::endl;
//  std::cout<<"old  : "<<((_u[_qp] - _u_old[_qp])/_dt)<<std::endl;
  
  return ((_u[_qp] - _u_old[_qp])/_dt) * _test[_i][_qp];
  
//  return _test[_i][_qp]*_u_dot[_qp];
}

Real
TimeDerivative::computeQpJacobian()
{
  return ((_phi[_j][_qp])/_dt) * _test[_i][_qp];
//  return _test[_i][_qp]*_phi[_j][_qp]*_du_dot_du[_qp];
}
