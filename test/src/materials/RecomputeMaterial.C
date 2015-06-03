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
#include "RecomputeMaterial.h"

template<>
InputParameters validParams<RecomputeMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("f_name", "The name of the property that holds the value to of the function for which the root is being computed");
  params.addRequiredParam<std::string>("f_prime_name", "The name of the property that holds the value to of the derivative of the function");
  params.addRequiredParam<std::string>("p_name", "The name of the independant variable for the function");
  params.addRequiredCoupledVar("variable", "The coupled variable to use in the function evaluation.");
  return params;
}

RecomputeMaterial::RecomputeMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _var(coupledValue("variable")),
    _f(declareProperty<Real>(getParam<std::string>("f_name"))),
    _f_prime(declareProperty<Real>(getParam<std::string>("f_prime_name"))),
    _p(getMaterialProperty<Real>(getParam<std::string>("p_name"))),
    _dummy(declareProperty<Real>("dummy"))
{
}

void
RecomputeMaterial::computeQpProperties()
{
  Real x = _p[_qp];
  _f[_qp] = x * x - _var[_qp];
  _f_prime[_qp] = _var[_qp]*x;
  _dummy[_qp] = 42; // for testing of warning messages
}
