//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearNonLinearIterationMaterial.h"

registerMooseObject("MooseTestApp", LinearNonLinearIterationMaterial);

InputParameters
LinearNonLinearIterationMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "The name of the property");
  params.addParam<Real>("prefactor", 1, "the prefactor to apply to the new material value");
  params.addClassDescription(
      "Material whose property is equal to (t_step+n_linear*n_nonlinear)*prefactor. Use "
      "this material only to test that other MOOSE systems can get in sync values. Only use RunApp "
      "type tests.");
  return params;
}

LinearNonLinearIterationMaterial::LinearNonLinearIterationMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _mat_prop(declareProperty<Real>(getParam<MaterialPropertyName>("prop_name"))),
    _prefactor(getParam<Real>("prefactor"))
{
}

void
LinearNonLinearIterationMaterial::computeQpProperties()
{
  _mat_prop[_qp] =
      (_t_step + _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()) * _prefactor;
}
