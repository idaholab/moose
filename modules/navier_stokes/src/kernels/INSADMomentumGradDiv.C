//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumGradDiv.h"
#include "Assembly.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", INSADMomentumGradDiv);

InputParameters
INSADMomentumGradDiv::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription("Adds grad-div stabilization to the INS momentum equation");
  params.addRequiredParam<Real>("gamma", "The grad-div stabilization coefficient");
  return params;
}

INSADMomentumGradDiv::INSADMomentumGradDiv(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _gamma(getParam<Real>("gamma")),
    _coord_sys(_assembly.coordSystem()),
    _rz_radial_coord(_mesh.getAxisymmetricRadialCoord())
{
}

ADReal
INSADMomentumGradDiv::computeQpResidual()
{
  const auto test_div = NS::divergence(
      _grad_test[_i][_qp], _test[_i][_qp], _ad_q_point[_qp], _coord_sys, _rz_radial_coord);
  const auto u_div =
      NS::divergence(_grad_u[_qp], _u[_qp], _ad_q_point[_qp], _coord_sys, _rz_radial_coord);
  return _gamma * test_div * u_div;
}
