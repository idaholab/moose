//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient", "thermal_conductivity", "Property name of the thermal conductivity");
  params.deprecateParam("diffusion_coefficient", "thermal_conductivity", "07/01/2027");
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient_dT",
      "thermal_conductivity_dT",
      "Property name of the derivative of the thermal conductivity with respect to the variable");
  params.deprecateParam("diffusion_coefficient_dT", "thermal_conductivity_dT", "07/01/2027");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HeatConductionKernel::HeatConductionKernel(const InputParameters & parameters)
  : Diffusion(parameters),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_dT(hasMaterialProperty<Real>("thermal_conductivity_dT")
                                 ? &getMaterialProperty<Real>("thermal_conductivity_dT")
                                 : NULL)
{
}

Real
HeatConductionKernel::computeQpResidual()
{
  return _thermal_conductivity[_qp] * Diffusion::computeQpResidual();
}

Real
HeatConductionKernel::computeQpJacobian()
{
  Real jac = _thermal_conductivity[_qp] * Diffusion::computeQpJacobian();
  if (_thermal_conductivity_dT)
    jac += (*_thermal_conductivity_dT)[_qp] * _phi[_j][_qp] * Diffusion::computeQpResidual();
  return jac;
}
