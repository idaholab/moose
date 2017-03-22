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
#include "NewtonMaterial.h"
#include "Material.h"

template <>
InputParameters
validParams<NewtonMaterial>()
{
  InputParameters params = validParams<Material>();
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
    _max_iterations(getParam<unsigned int>("max_iterations")),
    _discrete(getMaterial("material"))
{
}

void
NewtonMaterial::computeQpProperties()
{
  _p[_qp] = 0.5; // initial guess

  // Newton iteration for find p
  for (unsigned int i = 0; i < _max_iterations; ++i)
  {
    _discrete.computePropertiesAtQp(_qp);
    _p[_qp] -= _f[_qp] / _f_prime[_qp];
    if (std::abs(_f[_qp]) < _tol)
      break;
  }
}
