//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADTemperatureAdvection.h"

registerMooseObject("NavierStokesApp", INSADTemperatureAdvection);

InputParameters
INSADTemperatureAdvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("This class computes the residual and Jacobian contributions for "
                             "temperature advection for a divergence free velocity field.");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  params.addRequiredCoupledVar("velocity", "The velocity variable");
  return params;
}

INSADTemperatureAdvection::INSADTemperatureAdvection(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _rho(getADMaterialProperty<Real>("rho_name")),
    _cp(getADMaterialProperty<Real>("cp_name")),
    _U(adCoupledVectorValue("velocity"))
{
}

ADReal
INSADTemperatureAdvection::precomputeQpResidual()
{
  return _rho[_qp] * _cp[_qp] * _U[_qp] * _grad_u[_qp];
}

registerMooseObject("NavierStokesApp", INSADTemperatureAdvectionSUPG);

InputParameters
INSADTemperatureAdvectionSUPG::validParams()
{
  InputParameters params = ADKernelSUPG::validParams();
  params.addClassDescription(
      "This class computes the residual and Jacobian contributions for "
      "SUPG stabilization of temperature advection for a divergence free velocity field.");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  params.addRequiredCoupledVar("velocity", "The velocity variable");
  return params;
}

INSADTemperatureAdvectionSUPG::INSADTemperatureAdvectionSUPG(const InputParameters & parameters)
  : ADKernelSUPG(parameters),
    _rho(getADMaterialProperty<Real>("rho_name")),
    _cp(getADMaterialProperty<Real>("cp_name")),
    _U(adCoupledVectorValue("velocity"))
{
}

ADReal
INSADTemperatureAdvectionSUPG::precomputeQpStrongResidual()
{
  return _rho[_qp] * _cp[_qp] * _U[_qp] * _grad_u[_qp];
}
