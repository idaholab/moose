//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpecificTotalEnthalpyIC.h"

registerMooseObject("ThermalHydraulicsApp", SpecificTotalEnthalpyIC);

InputParameters
SpecificTotalEnthalpyIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  params.addClassDescription(
      "Sets the initial condition for the special total enthalpy of a phase");

  return params;
}

SpecificTotalEnthalpyIC::SpecificTotalEnthalpyIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _rhoA(coupledValue("rhoA")),
    _rhoEA(coupledValue("rhoEA")),
    _pressure(coupledValue("p")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

Real
SpecificTotalEnthalpyIC::value(const Point & /*p*/)
{
  return (_rhoEA[_qp] + _alpha[_qp] * _pressure[_qp] * _area[_qp]) / _rhoA[_qp];
}
