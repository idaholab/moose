//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DotCouplingAux.h"

registerMooseObject("MooseTestApp", DotCouplingAux);

InputParameters
DotCouplingAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("v", "Coupled variable");

  return params;
}

DotCouplingAux::DotCouplingAux(const InputParameters & parameters)
  : AuxKernel(parameters), _v_dot(coupledDot("v"))
{
}

DotCouplingAux::~DotCouplingAux() {}

Real
DotCouplingAux::computeValue()
{
  return _v_dot[_qp];
}
