//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DriftDiffusionFluxAux.h"

registerMooseObject("MooseTestApp", DriftDiffusionFluxAux);

InputParameters
DriftDiffusionFluxAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("potential", "The potential responsible for charge advection");
  params.addParam<bool>("positive_charge",
                        true,
                        "Whether the potential is advecting positive "
                        "charges. Should be set to false if charges are "
                        "negative.");
  params.addRequiredCoupledVar("u", "The drift-diffusing species.");
  params.addParam<int>("component", 0, "The flux component you want to see.");
  return params;
}

DriftDiffusionFluxAux::DriftDiffusionFluxAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _sgn(getParam<bool>("positive_charge") ? 1 : -1),
    _grad_potential(coupledGradient("potential")),
    _u(coupledValue("u")),
    _grad_u(coupledGradient("u")),
    _component(getParam<int>("component"))
{
}

Real
DriftDiffusionFluxAux::computeValue()
{
  return _sgn * -_grad_potential[_qp](_component) * _u[_qp] - _grad_u[_qp](_component);
}
