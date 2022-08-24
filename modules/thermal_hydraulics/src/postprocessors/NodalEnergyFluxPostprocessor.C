//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalEnergyFluxPostprocessor.h"

registerMooseObject("ThermalHydraulicsApp", NodalEnergyFluxPostprocessor);

InputParameters
NodalEnergyFluxPostprocessor::validParams()
{
  InputParameters params = NodalPostprocessor::validParams();
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("H", "Specific total enthalpy");

  return params;
}

NodalEnergyFluxPostprocessor::NodalEnergyFluxPostprocessor(const InputParameters & parameters)
  : NodalPostprocessor(parameters), _arhouA(coupledValue("arhouA")), _H(coupledValue("H"))
{
}

void
NodalEnergyFluxPostprocessor::initialize()
{
  _value = 0;
}

void
NodalEnergyFluxPostprocessor::execute()
{
  _value = _arhouA[_qp] * _H[_qp];
}

PostprocessorValue
NodalEnergyFluxPostprocessor::getValue()
{
  return _value;
}

void
NodalEnergyFluxPostprocessor::finalize()
{
  gatherSum(_value);
}

void
NodalEnergyFluxPostprocessor::threadJoin(const UserObject & uo)
{
  const NodalEnergyFluxPostprocessor & niep =
      dynamic_cast<const NodalEnergyFluxPostprocessor &>(uo);
  _value += niep._value;
}
