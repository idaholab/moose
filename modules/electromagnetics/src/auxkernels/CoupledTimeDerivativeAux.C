//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledTimeDerivativeAux.h"

registerMooseObject("ElectromagneticsApp", CoupledTimeDerivativeAux);

InputParameters
CoupledTimeDerivativeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("AuxKernel to calculate the time derivative of a coupled variable.");
  params.addRequiredCoupledVar("coupled", "Coupled variable.");
  return params;
}

CoupledTimeDerivativeAux::CoupledTimeDerivativeAux(const InputParameters & parameters)
  : AuxKernel(parameters), _coupled_dt(coupledDot("coupled"))
{
}

Real
CoupledTimeDerivativeAux::computeValue()
{
  return _coupled_dt[_qp];
}
