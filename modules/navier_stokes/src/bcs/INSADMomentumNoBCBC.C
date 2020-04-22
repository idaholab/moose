//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumNoBCBC.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSADMomentumNoBCBC);

InputParameters
INSADMomentumNoBCBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();

  params.addClassDescription("This class implements the 'No BC' boundary condition based on the "
                             "'Laplace' form of the viscous stress tensor.");
  params.addRequiredCoupledVar("p", "pressure");
  params.addParam<bool>("integrate_p_by_parts",
                        true,
                        "Allows simulations to be run with pressure BC if set to false");

  // Optional parameters
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  return params;
}

INSADMomentumNoBCBC::INSADMomentumNoBCBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters),
    _p(adCoupledValue("p")),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts")),
    _mu(getADMaterialProperty<Real>("mu_name"))
{
}

ADReal
INSADMomentumNoBCBC::computeQpResidual()
{
  // The viscous term
  // -mu * (grad(u).n) * test
  ADReal residual = -_mu[_qp] * (_grad_u[_qp] * _normals[_qp]) * _test[_i][_qp];

  if (_integrate_p_by_parts)
    // pIn * test
    residual += _p[_qp] * _normals[_qp] * _test[_i][_qp];

  return residual;
}
