//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyForcing.h"

registerMooseObject("MooseTestApp", PolyForcing);

InputParameters
PolyForcing::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

PolyForcing::PolyForcing(const InputParameters & parameters) : Kernel(parameters) {}

Real
PolyForcing::computeQpResidual()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real f =
      a * std::pow(x, 0.3e1) * y -
      (0.2e1 * (a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1)) *
           (0.3e1 * a * x * x * y * t + e * y * std::pow(z, 0.4e1)) -
       0.6e1 * a * x * x * y * t - 0.2e1 * e * y * std::pow(z, 0.4e1)) *
          (0.3e1 * a * x * x * y * t + e * y * std::pow(z, 0.4e1)) -
      0.6e1 *
          (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1),
                    0.2e1) -
           0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z -
           0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) *
          a * x * y * t -
      (0.2e1 * (a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1)) *
           (a * std::pow(x, 0.3e1) * t + 0.2e1 * b * y * z + e * x * std::pow(z, 0.4e1)) -
       0.2e1 * a * std::pow(x, 0.3e1) * t - 0.4e1 * b * y * z -
       0.2e1 * e * x * std::pow(z, 0.4e1)) *
          (a * std::pow(x, 0.3e1) * t + 0.2e1 * b * y * z + e * x * std::pow(z, 0.4e1)) -
      0.2e1 *
          (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1),
                    0.2e1) -
           0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z -
           0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) *
          b * z -
      (0.2e1 * (a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1)) *
           (b * y * y + 0.4e1 * e * x * y * std::pow(z, 0.3e1)) -
       0.2e1 * b * y * y - 0.8e1 * e * x * y * std::pow(z, 0.3e1)) *
          (b * y * y + 0.4e1 * e * x * y * std::pow(z, 0.3e1)) -
      0.12e2 *
          (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1),
                    0.2e1) -
           0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z -
           0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) *
          e * x * y * z * z -
      (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1),
                0.2e1) -
       0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z -
       0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) *
          (0.3e1 * a * x * x * y * t + e * y * std::pow(z, 0.4e1)) +
      0.2e1 *
          (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1),
                    0.2e1) -
           0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z -
           0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) *
          (a * std::pow(x, 0.3e1) * t + 0.2e1 * b * y * z + e * x * std::pow(z, 0.4e1)) -
      0.3e1 *
          (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1),
                    0.2e1) -
           0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z -
           0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) *
          (b * y * y + 0.4e1 * e * x * y * std::pow(z, 0.3e1)) +
      0.2e1 *
          std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1),
                   0.2e1);

  return -(_test[_i][_qp] * f);
}

Real
PolyForcing::computeQpJacobian()
{
  return 0;
}
