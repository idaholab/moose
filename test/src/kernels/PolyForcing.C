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

#include "PolyForcing.h"

template<>
InputParameters validParams<PolyForcing>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

PolyForcing::PolyForcing(const std::string & name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :Kernel(name, sys, parameters)
{
  
}

Real
PolyForcing::computeQpResidual()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t =_t;
  Real f = a * std::pow(x, 0.3e1) * y - (0.2e1 * (a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1)) * (0.3e1 * a * x * x * y * t + e * y * std::pow(z, 0.4e1)) - 0.6e1 * a * x * x * y * t - 0.2e1 * e * y * std::pow(z, 0.4e1)) * (0.3e1 * a * x * x * y * t + e * y * std::pow(z, 0.4e1)) - 0.6e1 * (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1), 0.2e1) - 0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z - 0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) * a * x * y * t - (0.2e1 * (a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1)) * (a * std::pow(x, 0.3e1) * t + 0.2e1 * b * y * z + e * x * std::pow(z, 0.4e1)) - 0.2e1 * a * std::pow(x, 0.3e1) * t - 0.4e1 * b * y * z - 0.2e1 * e * x * std::pow(z, 0.4e1)) * (a * std::pow(x, 0.3e1) * t + 0.2e1 * b * y * z + e * x * std::pow(z, 0.4e1)) - 0.2e1 * (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1), 0.2e1) - 0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z - 0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) * b * z - (0.2e1 * (a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1)) * (b * y * y + 0.4e1 * e * x * y * std::pow(z, 0.3e1)) - 0.2e1 * b * y * y - 0.8e1 * e * x * y * std::pow(z, 0.3e1)) * (b * y * y + 0.4e1 * e * x * y * std::pow(z, 0.3e1)) - 0.12e2 * (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1), 0.2e1) - 0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z - 0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) * e * x * y * z * z - (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1), 0.2e1) - 0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z - 0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) * (0.3e1 * a * x * x * y * t + e * y * std::pow(z, 0.4e1)) + 0.2e1 * (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1), 0.2e1) - 0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z - 0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) * (a * std::pow(x, 0.3e1) * t + 0.2e1 * b * y * z + e * x * std::pow(z, 0.4e1)) - 0.3e1 * (std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1), 0.2e1) - 0.2e1 * a * std::pow(x, 0.3e1) * y * t - 0.2e1 * b * y * y * z - 0.2e1 * e * x * y * std::pow(z, 0.4e1) + 0.2e1) * (b * y * y + 0.4e1 * e * x * y * std::pow(z, 0.3e1)) + 0.2e1 * std::pow(a * std::pow(x, 0.3e1) * y * t + b * y * y * z + e * x * y * std::pow(z, 0.4e1), 0.2e1);
  
  
  return  -(_test[_i][_qp] * f);
}

Real
PolyForcing::computeQpJacobian()
{
  return 0;
}
