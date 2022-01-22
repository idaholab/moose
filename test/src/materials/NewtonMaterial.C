//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NewtonMaterial.h"
#include "Material.h"

registerMooseObject("MooseTestApp", NewtonMaterial);

InputParameters
NewtonMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("f_name",
                                       "The name of the property that holds the value of "
                                       "the function for which the root is being "
                                       "computed");
  params.addRequiredParam<std::string>(
      "f_prime_name",
      "The name of the property that holds the value to of the derivative of the function");
  params.addRequiredParam<std::string>("p_name",
                                       "The name of the independent variable for the function");
  params.addParam<Real>("tol", 1e-12, "Newton solution tolerance.");
  params.addParam<unsigned int>("max_iterations", 42, "The maximum number of Newton iterations.");
  params.addRequiredParam<MaterialName>("material", "The material object to recompute.");
  return params;
}

NewtonMaterial::NewtonMaterial(const InputParameters & parameters)
  : Material(parameters),
    _tol(getParam<Real>("tol")),
    _f(getMaterialProperty<Real>(getParam<std::string>("f_name"))),
    _f_prime(getMaterialProperty<Real>(getParam<std::string>("f_prime_name"))),
    _p(declareProperty<Real>(getParam<std::string>("p_name"))),
    _max_iterations(getParam<unsigned int>("max_iterations"))
{
}

void
NewtonMaterial::initialSetup()
{
  _discrete = &getMaterial("material");
}

// MOOSEDOCS_START
void
NewtonMaterial::computeQpProperties()
{
  _p[_qp] = 0.5; // initial guess

  // Newton iteration for find p
  for (unsigned int i = 0; i < _max_iterations; ++i)
  {
    _discrete->computePropertiesAtQp(_qp);
    _p[_qp] -= _f[_qp] / _f_prime[_qp];
    if (std::abs(_f[_qp]) < _tol)
      break;
  }
}
