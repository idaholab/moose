/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MMSReaction.h"

template<>
InputParameters validParams<MMSReaction>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

MMSReaction::MMSReaction(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
  {}

Real
MMSReaction::computeQpResidual()
  {
    Real a = libMesh::pi;
    Real x = _q_point[_qp](0);
    Real y = _q_point[_qp](1);
    Real t = _t;
    if (_dim == 3)
    {
      Real z = _q_point[_qp](2);
      Real u = std::sin(a*x*y*z*t);
      return _test[_i][_qp]*2*u*u;
    }
    else
    {
      Real z = 1.0;
      Real u = std::sin(a*x*y*z*t);
      return _test[_i][_qp]*2*u*u;
    }
  }

Real
MMSReaction::computeQpJacobian()
  {
    return 0; //We have no grad u.
  }
