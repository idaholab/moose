//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxAverageAux.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", FluxAverageAux);

InputParameters
FluxAverageAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredCoupledVar("coupled", "Coupled variable for calculation of the flux");
  params.addRequiredParam<Real>("diffusivity", "Value to use as the 'diffusivity'");

  return params;
}

FluxAverageAux::FluxAverageAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _diffusivity(getParam<Real>("diffusivity")),
    _coupled_gradient(coupledGradient("coupled")),
    _coupled_var(dynamic_cast<MooseVariable &>(*getVar("coupled", 0))),
    _normals(_assembly.normals())
{
}

Real
FluxAverageAux::computeValue()
{
  return _diffusivity * _coupled_gradient[_qp] * _normals[_qp];
}
