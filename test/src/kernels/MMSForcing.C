#include "MMSForcing.h"

template<>
InputParameters validParams<MMSForcing>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

MMSForcing::MMSForcing(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :Kernel(name, sys, parameters)
{
  
}

Real
MMSForcing::computeQpResidual()
{
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t =_t;
  Real f;
  
  f= std::cos(a*x*y*z*t)*(a*x*y*z - a*y*z*t + 2.0*a*x*z*t - 3.0*a*x*y*z*t)+(std::sin(a*x*y*z*t)*std::sin(a*x*y*z*t)*std::sin(a*x*y*z*t))*(a*a*y*y*z*z*t*t + a*a*x*x*z*z*t*t + a*a*x*x*y*y*t*t) + ((std::cos(a*x*y*z*t)*std::cos(a*x*y*z*t)))*std::sin(a*x*y*z*t)*(-2.0*a*a*y*y*z*z*t*t - 2.0*a*a*x*x*z*z*t*t - 2.0*a*a*x*x*y*y*t*t) + ((std::sin(a*x*y*z*t)*std::sin(a*x*y*z*t)))*(-2.0*a*a*y*y*z*z*t*t - 2.0*a*a*x*x*z*z*t*t - 2.0*a*a*x*x*y*y*t*t + 2.0) + ((std::cos(a*x*y*z*t)*cos(a*x*y*z*t)))*(2.0*a*a*y*y*z*z*t*t + 2.0*a*a*x*x*z*z*t*t + 2.0*a*a*x*x*y*y*t*t) + std::sin(a*x*y*z*t)*(2.0*a*a*y*y*z*z*t*t + 2.0*a*a*x*x*z*z*t*t + 2.0*a*a*x*x*y*y*t*t);
  
  return  -(_test[_i][_qp] * f);
}

Real
MMSForcing::computeQpJacobian()
{
  return 0;
}
