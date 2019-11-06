//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MMSForcing.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", MMSForcing);

InputParameters
MMSForcing::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

MMSForcing::MMSForcing(const InputParameters & parameters)
  : Kernel(parameters), _mesh_dimension(_mesh.dimension())
{
}

Real
MMSForcing::computeQpResidual()
{
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real t = _t;

  if (_mesh_dimension == 3)
  {
    Real z = _q_point[_qp](2);
    Real f =
        std::cos(a * x * y * z * t) *
            (a * x * y * z - a * y * z * t + 2.0 * a * x * z * t - 3.0 * a * x * y * t) +
        (std::sin(a * x * y * z * t) * std::sin(a * x * y * z * t) * std::sin(a * x * y * z * t)) *
            (a * a * y * y * z * z * t * t + a * a * x * x * z * z * t * t +
             a * a * x * x * y * y * t * t) +
        ((std::cos(a * x * y * z * t) * std::cos(a * x * y * z * t))) *
            std::sin(a * x * y * z * t) *
            (-2.0 * a * a * y * y * z * z * t * t - 2.0 * a * a * x * x * z * z * t * t -
             2.0 * a * a * x * x * y * y * t * t) +
        ((std::sin(a * x * y * z * t) * std::sin(a * x * y * z * t))) *
            (-2.0 * a * a * y * y * z * z * t * t - 2.0 * a * a * x * x * z * z * t * t -
             2.0 * a * a * x * x * y * y * t * t + 2.0) +
        ((std::cos(a * x * y * z * t) * cos(a * x * y * z * t))) *
            (2.0 * a * a * y * y * z * z * t * t + 2.0 * a * a * x * x * z * z * t * t +
             2.0 * a * a * x * x * y * y * t * t) +
        std::sin(a * x * y * z * t) *
            (2.0 * a * a * y * y * z * z * t * t + 2.0 * a * a * x * x * z * z * t * t +
             2.0 * a * a * x * x * y * y * t * t);
    return -(_test[_i][_qp] * f);
  }
  else
  {
    Real z = 1.0;
    Real f =
        std::cos(a * x * y * z * t) * (a * x * y * z - a * y * z * t + 2.0 * a * x * z * t) +
        (std::sin(a * x * y * z * t) * std::sin(a * x * y * z * t) * std::sin(a * x * y * z * t)) *
            (a * a * y * y * z * z * t * t + a * a * x * x * z * z * t * t) +
        ((std::cos(a * x * y * z * t) * std::cos(a * x * y * z * t))) *
            std::sin(a * x * y * z * t) *
            (-2.0 * a * a * y * y * z * z * t * t - 2.0 * a * a * x * x * z * z * t * t) +
        ((std::sin(a * x * y * z * t) * std::sin(a * x * y * z * t))) *
            (-2.0 * a * a * y * y * z * z * t * t - 2.0 * a * a * x * x * z * z * t * t + 2.0) +
        ((std::cos(a * x * y * z * t) * cos(a * x * y * z * t))) *
            (2.0 * a * a * y * y * z * z * t * t + 2.0 * a * a * x * x * z * z * t * t) +
        std::sin(a * x * y * z * t) *
            (2.0 * a * a * y * y * z * z * t * t + 2.0 * a * a * x * x * z * z * t * t);
    return -(_test[_i][_qp] * f);
  }
}

Real
MMSForcing::computeQpJacobian()
{
  return 0;
}
