//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumFriction.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumFriction);

InputParameters
PINSFVMomentumFriction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Computes a friction force term on fluid in porous media in the "
                             "Navier Stokes i-th momentum equation.");
  params.addRequiredRangeCheckedParam<unsigned int>("component",
    "component < 3", "equation number (x = 0, y = 1, z = 2)");
  params.addRequiredParam<std::string>("Darcy_name", "Name of the Darcy coefficients material property.");
  params.addRequiredParam<std::string>("Forchheimer_name", "Name of the Forchheimer coefficients material property.");

  return params;
}

PINSFVMomentumFriction::PINSFVMomentumFriction(const InputParameters & params)
  : FVElementalKernel(params),
  _component(getParam<unsigned int>("component")),
  _cL(getADMaterialProperty<RealVectorValue>("Darcy_name")),
  _cQ(getADMaterialProperty<RealVectorValue>("Forchheimer_name"))
{
}

ADReal
PINSFVMomentumFriction::computeQpResidual()
{
  return - (_cL[_qp](_component) + _cQ[_qp](_component)) * _u[_qp];
}
