//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RecomputeMaterial.h"

registerMooseObject("MooseTestApp", RecomputeMaterial);

InputParameters
RecomputeMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("f_name",
                                       "The name of the property that holds the value to "
                                       "of the function for which the root is being "
                                       "computed");
  params.addRequiredParam<std::string>(
      "f_prime_name",
      "The name of the property that holds the value to of the derivative of the function");
  params.addRequiredParam<std::string>("p_name",
                                       "The name of the independent variable for the function");
  params.addParam<Real>("constant", 0, "The constant to add to the f equation.");
  params.set<bool>("compute") = false;
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

// MOOSEDOCS_START
void
RecomputeMaterial::computeQpProperties()
{
  Real x = _p[_qp];
  _f[_qp] = x * x - _constant;
  _f_prime[_qp] = 2 * x;
}
