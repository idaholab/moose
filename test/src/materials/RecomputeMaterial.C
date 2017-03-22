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

// MOOSE includes
#include "RecomputeMaterial.h"

template <>
InputParameters
validParams<RecomputeMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("f_name",
                                       "The name of the property that holds the value to "
                                       "of the function for which the root is being "
                                       "computed");
  params.addRequiredParam<std::string>(
      "f_prime_name",
      "The name of the property that holds the value to of the derivative of the function");
  params.addRequiredParam<std::string>("p_name",
                                       "The name of the independant variable for the function");
  params.addParam<Real>("constant", 0, "The constant to add to the f equation.");
  return params;
}

RecomputeMaterial::RecomputeMaterial(const InputParameters & parameters)
  : Material(parameters),
    _f(declareProperty<Real>(getParam<std::string>("f_name"))),
    _f_prime(declareProperty<Real>(getParam<std::string>("f_prime_name"))),
    _p(getMaterialProperty<Real>(getParam<std::string>("p_name"))),
    _constant(getParam<Real>("constant"))
{
}

void
RecomputeMaterial::resetQpProperties()
{
  _f[_qp] = 84;
  _f_prime[_qp] = 42;
}

void
RecomputeMaterial::computeQpProperties()
{
  Real x = _p[_qp];
  _f[_qp] = x * x - _constant;
  _f_prime[_qp] = 2 * x;
}
