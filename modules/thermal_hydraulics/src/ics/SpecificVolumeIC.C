//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpecificVolumeIC.h"

registerMooseObject("ThermalHydraulicsApp", SpecificVolumeIC);

InputParameters
SpecificVolumeIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("rhoA",
                               "Density of the phase (conserved), \alpha \rho A for 2-phase model");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  params.addClassDescription("Sets an initial condition for the specific volume of a phase");
  return params;
}

SpecificVolumeIC::SpecificVolumeIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _rhoA(coupledValue("rhoA")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

Real
SpecificVolumeIC::value(const Point & /*p*/)
{
  mooseAssert(_rhoA[_qp] != 0, "Detected zero density.");
  return _alpha[_qp] * _area[_qp] / _rhoA[_qp];
}
