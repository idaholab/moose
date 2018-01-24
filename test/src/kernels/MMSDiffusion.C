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

#include "MMSDiffusion.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MMSDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

MMSDiffusion::MMSDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _mesh_dimension(_mesh.dimension())
{
}

Real
MMSDiffusion::computeQpResidual()
{
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real t = _t;
  if (_mesh_dimension == 3)
  {
    Real z = _q_point[_qp](2);
    Real u = std::sin(a * x * y * z * t);
    return _grad_test[_i][_qp] * (u * u - 2 * u + 2) * _grad_u[_qp];
    // We multiplied by our k(u).
  }
  else
  {
    Real z = 1.0;
    Real u = std::sin(a * x * y * z * t);
    return _grad_test[_i][_qp] * (u * u - 2 * u + 2) * _grad_u[_qp];
    // We multiplied by our k(u).
  }
}

Real
MMSDiffusion::computeQpJacobian()
{
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real t = _t;
  if (_mesh_dimension == 3)
  {
    Real z = _q_point[_qp](2);
    Real u = std::sin(a * x * y * z * t);
    return _grad_test[_i][_qp] * (u * u - 2 * u + 2) * _grad_phi[_j][_qp];
    // We multiplied by our k(u).
  }
  else
  {
    Real z = 1.0;
    Real u = std::sin(a * x * y * z * t);
    return _grad_test[_i][_qp] * (u * u - 2 * u + 2) * _grad_phi[_j][_qp];
    // We multiplied by our k(u).
  }
}
