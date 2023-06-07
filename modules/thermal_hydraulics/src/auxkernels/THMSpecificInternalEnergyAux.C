//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSpecificInternalEnergyAux.h"

registerMooseObject("ThermalHydraulicsApp", THMSpecificInternalEnergyAux);

InputParameters
THMSpecificInternalEnergyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addClassDescription("Computed the specific internal energy.");
  return params;
}

THMSpecificInternalEnergyAux::THMSpecificInternalEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue("rhoA")),
    _rhou(coupledValue("rhouA")),
    _rhoE(coupledValue("rhoEA"))
{
}

Real
THMSpecificInternalEnergyAux::computeValue()
{
  return (_rhoE[_qp] - 0.5 * _rhou[_qp] * _rhou[_qp] / _rho[_qp]) / _rho[_qp];
}
