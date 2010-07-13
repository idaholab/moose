#include "MMSReaction.h"

template<>
InputParameters validParams<MMSReaction>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

MMSReaction::MMSReaction(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
  {}

Real
MMSReaction::computeQpResidual()
  {
    Real a = libMesh::pi;
    Real x = _q_point[_qp](0);
    Real y = _q_point[_qp](1);
    Real z = _q_point[_qp](2);
    Real t = _t;
    Real u = std::sin(a*x*y*z*t);
    return _test[_i][_qp]*2*u*u;
  }

Real
MMSReaction::computeQpJacobian()
  {
    return 0; //We have no grad u.
  }
