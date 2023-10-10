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
  params += NestedSolve::validParams();
  params.addRequiredParam<std::string>("f_name",
                                       "The name of the property that holds the value of "
                                       "the function for which the root is being "
                                       "computed");
  params.addRequiredParam<std::string>(
      "f_prime_name",
      "The name of the property that holds the value to of the derivative of the function");
  params.set<Real>("relative_tolerance") = 1e-15;
  params.addRequiredParam<std::string>("p_name",
                                       "The name of the independent variable for the function");
  params.addRequiredParam<MaterialName>("material", "The material object to recompute.");
  return params;
}

NewtonMaterial::NewtonMaterial(const InputParameters & parameters)
  : Material(parameters),
    _f(getMaterialProperty<Real>(getParam<std::string>("f_name"))),
    _f_prime(getMaterialProperty<Real>(getParam<std::string>("f_prime_name"))),
    _p(declareProperty<Real>(getParam<std::string>("p_name"))),
    _nested_solve(NestedSolve(parameters))
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

  // Only attempt to solve if iterations are to be taken. This is usually not required, but needed
  // here to retain the old test behavior that would not trigger a discrete material evaluation. The
  // NestedSolve class will always evaluate the residual for the initial guess (and will return a
  // success state if the initial guess was exact).
  if (getParam<unsigned int>("max_iterations") > 0)
    _nested_solve.nonlinear(
        _p[_qp],
        // Lambda function to compute residual and jacobian. The initial guess is not
        // used here as it (_p) is directly coupled in the discrete material.
        [&](const Real & /*guess*/, Real & r, Real & j)
        {
          _discrete->computePropertiesAtQp(_qp);
          r = _f[_qp];
          j = _f_prime[_qp];
        });
}
