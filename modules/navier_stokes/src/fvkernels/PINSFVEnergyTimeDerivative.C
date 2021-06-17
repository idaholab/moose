//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyTimeDerivative.h"
#include "INSFVEnergyVariable.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyTimeDerivative);

InputParameters
PINSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation: "
      "for fluids: eps * rho * cp * dT/dt, for solids: (1 - eps) * rho * cp * dT/dt");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  params.addRequiredParam<MaterialPropertyName>("cp_name",
                                                "The name of the specific heat capacity");
  params.addRequiredCoupledVar("porosity", "Porosity variable");
  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature");
  return params;
}

PINSFVEnergyTimeDerivative::PINSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getParam<Real>("rho")),
    _cp(getADMaterialProperty<Real>("cp_name")),
    _eps(coupledValue("porosity")),
    _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
PINSFVEnergyTimeDerivative::computeQpResidual()
{
  // Note: This is neglecting time derivative of both _cp and _rho
  return (_is_solid ? 1 - _eps[_qp] : _eps[_qp]) * _rho * _cp[_qp] *
         FVTimeKernel::computeQpResidual();
}
