//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledGradAux.h"

using namespace libMesh;

registerMooseObject("MooseTestApp", CoupledGradAux);

InputParameters
CoupledGradAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredCoupledVar("coupled", "Coupled gradient for calculation");

  params.addRequiredParam<RealGradient>("grad", "Gradient to dot it with");

  return params;
}

CoupledGradAux::CoupledGradAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad(getParam<RealGradient>("grad")),
    _coupled(coupled("coupled")),
    _coupled_grad(coupledGradient("coupled"))
{
}

CoupledGradAux::~CoupledGradAux() {}

Real
CoupledGradAux::computeValue()
{
  return _coupled_grad[_qp] * _grad;
}
