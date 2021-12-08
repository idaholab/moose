//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConductivityLaplacian.h"
#include "MooseMesh.h"

registerMooseObject("ElectromagneticsApp", ConductivityLaplacian);

InputParameters
ConductivityLaplacian::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Computes the residual and Jacobian contribution for the weak form term "
      "associated with $\\nabla \\cdot (\\sigma \\nabla V)$, where "
      "$\\sigma$ is the electrical conductivity and $V$ is the electrostatic potential.");
  params.addParam<MaterialPropertyName>(
      "conductivity_coefficient",
      "conductivity",
      "Property name of the material conductivity (Default: conductivity).");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ConductivityLaplacian::ConductivityLaplacian(const InputParameters & parameters)
  : ADKernel(parameters), _conductivity(getADMaterialProperty<Real>("conductivity_coefficient"))
{
}

ADReal
ConductivityLaplacian::computeQpResidual()
{
  return _conductivity[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}
