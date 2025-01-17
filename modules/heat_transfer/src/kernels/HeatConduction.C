//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConduction.h"
#include "MooseMesh.h"

registerMooseObjectAliased("HeatTransferApp", HeatConductionKernel, "HeatConduction");

InputParameters
HeatConductionKernel::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addClassDescription("Diffusive heat conduction term $-\\nabla\\cdot(k\\nabla T)$ of the "
                             "thermal energy conservation equation");
  params.addParam<MaterialPropertyName>("diffusion_coefficient",
                                        "thermal_conductivity",
                                        "Property name of the diffusion coefficient");
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient_dT",
      "thermal_conductivity_dT",
      "Property name of the derivative of the diffusion coefficient with respect "
      "to the variable");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HeatConductionKernel::HeatConductionKernel(const InputParameters & parameters)
  : Diffusion(parameters),
    _diffusion_coefficient(getMaterialProperty<Real>("diffusion_coefficient")),
    _diffusion_coefficient_dT(hasMaterialProperty<Real>("diffusion_coefficient_dT")
                                  ? &getMaterialProperty<Real>("diffusion_coefficient_dT")
                                  : NULL)
{
}

Real
HeatConductionKernel::computeQpResidual()
{
  return _diffusion_coefficient[_qp] * Diffusion::computeQpResidual();
}

Real
HeatConductionKernel::computeQpJacobian()
{
  Real jac = _diffusion_coefficient[_qp] * Diffusion::computeQpJacobian();
  if (_diffusion_coefficient_dT)
    jac += (*_diffusion_coefficient_dT)[_qp] * _phi[_j][_qp] * Diffusion::computeQpResidual();
  return jac;
}
